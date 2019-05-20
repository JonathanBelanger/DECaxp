/*
 * Copyright (C) Jonathan D. Belanger 2018-2019.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied only
 * in accordance with the terms of such license and with the inclusion of the
 * above copyright notice.  This software or any other copies thereof may not
 * be provided or otherwise made available to any other person.  No title to
 * and ownership of the software is hereby transferred.
 *
 * The information in this software is subject to change without notice and
 * should not be construed as a commitment by the author or co-authors.
 *
 * The author and any co-authors assume no responsibility for the use or
 * reliability of this software.
 *
 * Description:
 *
 *  The implementation of the Real Time Clock (RTC), also known as Time of Year
 *  (TOY) clock.  Other implementations of this functionality simply read the
 *  byte at the specified address, which is fine.  The issue is that the write
 *  actually stored the value in the byte, but then proceeded to get the system
 *  time and then update it with the current time, as defined in the system.
 *  What this does is mean that the emulator would have to have the same exact
 *  time as the host operating system.  This implementation will take into
 *  consideration that someone might like to have the host and guest operating
 *  systems have different system times.  To this end, the code will perform
 *  the following steps:
 *
 *
 *  For Writes:
 *      1)  If the SET - Update Transfer Inhibited bit is not set and we
 *          are not writing to one of the Control Registers or RAM
 *          locations, then swallow the write and just return back to the
 *          caller.
 *      2)  If the SET is being set, get the current time from the host
 *          operating system and store it into a module variable for use
 *          later (may want to save it in a way that will make future
 *          calculations easier and faster.  Then return back to the
 *          caller.
 *      3)  If the SET is set, then determine the difference between the
 *          saved host operating system time and the value being stored
 *          (remember the difference could be negative) and save that in
 *          the appropriate temporary register location and format.  Then
 *          return back to the caller.
 *      4)  If the SET is set and is being cleared, then move the temporary
 *          registers to the real locations, clear the UIP flag, and return
 *          back to the caller.
 *
 *  For Reads:
 *      1)  Reads can happen at any time.  There UIP and SET bits do not
 *          prevent reads from occurring.
 *
 * Revision History:
 *
 *  V01.000 25-May-2018 Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001 28-May-2018 Jonathan D. Belanger
 *  The time functions won't work as documented.  First off, the host
 *  current time is one that is affected by daylight savings time.  What
 *  this will cause is that if a time difference were calculated just
 *  before we loose an hour in the Fall and then get the time just after,
 *  the clock will go negative and the calculations will not work.  So, we
 *  are going to get GMT from the host and use that in our calculations.
 *  Secondly, the getting the difference when setting and adding that
 *  difference back in when reading does not take into account where any of
 *  the fields are greater than their maximum.  To this end, we will
 *  blindly add the number back in and then normalize the time information.
 *  Also, we need to take into consideration leap year.
 *
 *  V01.002 01-Jun-2018 Jonathan D. Belanger
 *  Added the ability to provide a mutex, condition variable, interrupt field,
 *  and interrupt mask, so that when an interrupt is triggered, the thread that
 *  needs to be notified, has been informed.
 *
 *  V01.003 05-Jun-2018 Jonathan D. Belanger
 *  The original code did not take into account Daylight Savings Time (DST).
 *  It did have the DSE bit in Control Register B, but since we are using GMT
 *  from the host operating system clock, there was no adjustment for DST.  One
 *  of the things that had concerned me was that the documentation indicated
 *  that the MSB for the seconds register was read-only.  I'm not sure why the
 *  actual chip had this, but I'm going to take advantage and use it to
 *  indicate when the time information in the registers contains a DST value.
 *  Using this flag, I'll be able to determine when to spring forward or fall
 *  backward.  Also, the actual chip did not account for a difference between
 *  DST for the US prior to and since 2007, or Europe prior to and since 1996.
 *  To account for this, there is going to be an additional configuration item
 *  that will set another reserved bit in Control Register D.  We also need to
 *  add code to determine if DST has occurred and we either need to spring
 *  forward of fall back.
 *
 *  V01.004 09-Jun-2018 Jonathan D. Belanger
 *  Moved the isDST flag from the Seconds register to Control Register D.
 *  Having it in the seconds register caused math problems that were easier to
 *  resolve moving this flag than keeping it where I had originally planned.
 *
 *  V01.005 19-May-2019 Jonathan D. Belanger
 *  GCC 7.4.0, and possibly earlier, turns on strict-aliasing rules by default.
 *  There are a number of issues in this module where the address of one
 *  variable is cast to extract a value in a different format.  In this module
 *  these all appear to be when trying to get the 64-bit value equivalent of
 *  the 64-bit long PC structure.  We will use shifts (in a macro) instead of
 *  the casts.
 */
#include "CommonUtilities/AXP_Utility.h"
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Trace.h"
#include "Devices/TOYClock/AXP_DS12887A_TOYClock.h"
#include <cygwin/signal.h>

/*
 * The RAM array contains 128 bytes to store date, time, and general
 * information.  The array is declared as signed bytes here, but the structure
 * definitions and input parameters are declared as unsigned bytes.  We do this
 * because the date and time entries will be the difference between the host
 * operating system time and the time being set by or returned to the caller.
 * This will simplify the math later.
 */
static i8 ram[AXP_DS12887A_RAM_SIZE];
AXP_DS12887A_ControlA *ctrlA = (AXP_DS12887A_ControlA *) &ram[AXP_ADDR_ControlA];
AXP_DS12887A_ControlB *ctrlB = (AXP_DS12887A_ControlB *) &ram[AXP_ADDR_ControlB];
AXP_DS12887A_ControlC *ctrlC = (AXP_DS12887A_ControlC *) &ram[AXP_ADDR_ControlC];
AXP_DS12887A_ControlD *ctrlD = (AXP_DS12887A_ControlD *) &ram[AXP_ADDR_ControlD];

/*
 * The first call to any of the interface calls will check this flag.  If it is
 * set to false, then the initialization function will be called and the flag
 * set to true.
 */
static bool initialized = false;

/*
 * This local module variable is set when the SET flag is set (man that sounds
 * odd) in Control Register B.  It is this time that will be used to calculate
 * any differences or return values for the time.
 */
static struct tm currentTime;

/*
 * We use a mutex to make sure that more than one thread does not go through
 * this code at any one time.  We are also going to serialize the code from
 * Update In Progress (UIP) being set until it is cleared.  The condition
 * variable will be signaled when the UIP is cleared.
 */
static pthread_cond_t rtcCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t rtcMutex = PTHREAD_MUTEX_INITIALIZER;

#define AXP_DS12887A_LOCK                                                   \
    pthread_mutex_lock(&rtcMutex);                                          \
    if (initialized == false)                                               \
    {                                                                       \
        AXP_DS12887A_Initialize();                                          \
    }
#define AXP_DS12887A_UNLOCK    pthread_mutex_unlock(&rtcMutex)

/*
 * We use timers to handle the update, alarm and periodic interrupts.
 *
 * The update interrupt generally occurs every second, unless the SET bit in
 * Register B or the UIP bit in Register A is set.  For the UIP bit, we'll wait
 * for it to clear.  For the SET bit, we'll just return and wait for the next
 * update.
 *
 * The alarm interrupt will be triggered when the time has passed.
 *
 * The periodic interrupt will be trigger every certain number of milliseconds.
 */
static timer_t periodicTimer;
static timer_t alarmTimer;
static timer_t updateTimer;
static bool timersArmed = false;

/*
 * Locations to store a mutex, condition variable, IRQ bit field and IRQ bit
 * mask, to be used when the IRQH bit has been set/cleared.
 */
static pthread_cond_t *irqCond = NULL;
static pthread_mutex_t *irqMutex = NULL;
static u64 *irqField = NULL;
static u64 irqMask = 0;

/*
 * Local Prototypes
 */
static int AXP_DS12887A_DST(u8, u8, u8, u8, u8, u8);
static void AXP_DS12887A_Normalize(struct tm *, bool);
void AXP_DS12887A_Notify(sigval_t);
static void AXP_DS12887A_StartTimers(bool);
static void AXP_DS12887A_StopTimers(void);
static void AXP_DS12887A_Initialize(void);
static void AXP_DS12887A_CheckIRQF(void);

/*
 * AXP_DS12887A_DST
 *  This function is called to determine if the hours register needs to be
 *  added to (Daylight Savings Time) or subtracted from (Standard Time).
 *
 * Input Parameters:
 *  y:
 *      An unsigned byte containing the 2-digit year.
 *  M:
 *      An unsigned byte containing the 2-digit month.
 *  d:
 *      An unsigned byte containing the 2-digit day.
 *  h:
 *      An unsigned byte containing the 2-digit hour.
 *  m:
 *      An unsigned byte containing the 2-digit minutes.
 *  s:
 *      An unsigned byte containing the 2-digit seconds.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  -1:     Transitioned from Daylight Savings Time to Standard Time.
 *   0:     No transition required.
 *  +1:     Transitioned from Standard Time to Daylight Savings Time.
 *
 *  Note:   This function is called under 2 conditions.  First, when the SET
 *          bit is being cleared by a write and 2 when any of the time
 *          registers are being read.  Under all other circumstances, DST state
 *          is allowed to float.
 */
static int AXP_DS12887A_DST(u8 y, u8 M, u8 d, u8 h, u8 m, u8 s)
{
    int retVal = 0;
    u16 year = y + ((y >= 70) && (y <= 99) ? 1900 : 2000);
    u16 mid;
    u32 date;
    u32 dstStart = 0, dstEnd = 0;

    date = (M << 22) + (d << 17) + (h << 12) + (m << 6) + s;

    /*
     * If Daylight Savings Time is enabled, then we need to determine  whether
     * we transitioned to or from Daylight Savings Time.  The  following rules
     * are applied to the time values submitted on this call.
     *
     *  For European DTS and years >= 1996:
     *      DST Starts:    Last Sunday in March
     *      DST Ends: Last Sunday in October
     *  For Non-European (US) DTS and years < 2007:
     *      DST Starts:    First Sunday in April
     *      DST Ends: Last Sunday in October
     *  For Non-European (US) DTS and years >= 2007:
     *      DST Starts:    Second Sunday in March
     *      DST Ends: First Sunday in November
     *
     * NOTE: This emulation supports years from 1970 to 2069.
     */
    if (ctrlB->dse == 1)
    {
        mid = (5 * year) / 4;        /* Middle part of the formula */

        /*
         * If this is not European DST (US DST), then determine the
         * start and end month and day for DST.
         */
        if (ctrlD->eu == 0)
        {

            /*
             * For years prior to 2007, US DST:
             *  Starts: First Sunday in April at 2:00am.
             *  Ends: Last Sunday in October at 2:00am.
             */
            if (year < 2007)
            {
                dstStart = (4 << 22) +                      /* April */
                           ((7-((4 + mid) % 7)) << 17) +    /* First Sunday */
                           (2 << 12);                       /* 2:00 am */
                dstEnd = (10 << 22) +                       /* October */
                         ((31-((1 + mid) % 7)) << 17) +     /* Last Sunday */
                         (2 << 12);                         /* 2:00 am */
            }

            /*
             * For years starting in 2007, US DST:
             *  Starts: Second Sunday in March at 2:00am.
             *  Ends: First Sunday in November at 2:00am.
             */
            else
            {
                dstStart = (3 << 22) +                      /* March */
                           ((14-((1 + mid) % 7)) << 17) +   /* Second Sunday */
                           (2 << 12);                       /* 2:00 am */
                dstEnd = (11 << 22) +                       /* November */
                         ((7 - ((1 + mid) % 7)) << 17) +    /* First Sunday */
                         (2 << 12);                         /* 2:00 am */
            }
        }

        /*
         * For years starting 1996 US DST:
         *  Starts: Last Sunday in March at 2:00am.
         *  Ends: Last Sunday in November at 2:00am.
         */
        else if (year >= 1996)
        {
            dstStart = (3 << 22) +                          /* March */
                       ((31 - ((4 + mid) % 7)) << 17) +     /* Last Sunday */
                       (2 << 12);                           /* 2:00 am */
            dstEnd = (11 << 22) +                           /* November */
                     ((31 - ((1 + mid) % 7)) << 17) +       /* First Sunday */
                     (2 << 12);                             /* 2:00 am */
        }

        /*
         * For everything else, DST is not supported, so clear the Daylight
         * Savings Time flag in the Control Register B, so that we can skip all
         * this processing.
         */
        else
        {
            ctrlB->dse = 0;
        }
    }

    /*
     * So, if we still have Daylight Savings enabled, then we now need to
     * determine if the date provided on the call needs to be adjusted +/- one
     * hour, to account for DST starting or ending.
     */
    if (ctrlB->dse == 1)
    {
        bool newIsDst = false;
        bool isDst = (ctrlD->isDst == 1);

        /*
         * If the date is between the DST start and end dates, and not because
         * we shifted
         */
        if ((dstStart < date) && (date < dstEnd) && (ctrlD->fellBack == 0))
        {
            newIsDst = true;
        }
        else if ((dstStart >= date) || (date >= dstEnd))
        {
            ctrlD->fellBack = 0;
        }

        /*
         * Now we are ready to figure out what we should be doing.  If the time
         * provided is in DST and the previous time we checked it was NOT in
         * DST, then we need to spring forward.
         */
        if ((newIsDst == true) && (isDst == false))
        {
            retVal = 1;
            ctrlD->isDst = 1;
        }

        /*
         * The only three other options are:
         *  1) The time provided is NOT in DST and the previous time was
         *     also NOT in DST.
         *  2) The time provided is NOT in DST and the previous time was in
         *     DST.
         *  3) The time provided is in DST and the previous time was also
         *     in DST.
         * Of these, only option 2 requires us to do something.  So, if the
         * time provided DST and the previous time DST do not match, then we
         * need to fall back.
         * Otherwise, nothing changed from the last time we checked.
         */
        else if (newIsDst != isDst)
        {
            retVal = -1;
            ctrlD->isDst = 0;
            ctrlD->fellBack = 1;
        }
    }

    /*
     * Return the results back to the caller.
     */
    return(retVal);
}

/*
 * AXP_DS12887A_Normalize
 *  This function is called to normalize a struct tm.
 *
 * Input Parameters:
 *  timeSpec:
 *      A pointer to a structure containing the time information to be
 *      normalized.
 *  justTime:
 *      A flag to indicate that just the time fields are to be normalized.
 *
 * Output Parameters:
 *  timeSpec:
 *      A pointer to a structure to receive the normalized time information.
 *
 * Return Values:
 *  None.
 */
static void AXP_DS12887A_Normalize(struct tm *timeSpec, bool justTime)
{
    int mDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int tmp;

    /*
     *  Steps for normalization is as follows:
     *      1)  Add the number of minutes in the seconds field to the minutes
     *          field.
     *      2)  Keep the seconds greater than 60 in the seconds field.
     *      3)  Add the number of hours in the minutes field to the hours
     *          field.
     *      4)  Keep the minutes greater than 60 in the minutes field.
     *      5)  Add the number of days in the hours field to the days field.
     *      6)  Keep the hours greater than 24 in the hours field.
     *      7)  Add the number of years in the month field to the years field.
     *      8)  Keep the months greater than 12 in the months field.
     *      9)  Use the year field to determine if we are in a leap year.  If
     *          so, add one day to the maximum number of days in February.
     *      10) Add the months greater than the maximum number of days for the
     *          currently calculated month to the months field.
     *      11) Keep the days greater than the maximum number of days for the
     *          month in the days field.
     *      12) Add the number of years in the month field to the years field.
     *      13) Keep the months greater than 12 in the months field.
     *      14) Get the correct day of the week.
     *
     * NOTE: We used 1970 as the cross over point for the century.  So, years
     *       from 70-99 are 1970-1999, and years from 00-69 are 2000-2069.
     */
    timeSpec->tm_min += (timeSpec->tm_sec / 60);
    timeSpec->tm_sec %= 60;
    timeSpec->tm_hour += (timeSpec->tm_min / 60);
    timeSpec->tm_min %= 60;
    if (justTime == false)
    {
        timeSpec->tm_mday += (timeSpec->tm_hour / 24);
        timeSpec->tm_hour %= 24;
        timeSpec->tm_year += (timeSpec->tm_mon / 12);
        timeSpec->tm_mon %= 12;
        tmp = (timeSpec->tm_year >= 70 ? 1900 : 2000) + timeSpec->tm_year;
        mDays[1] += (((tmp % 400) == 0) ?
                (((tmp % 100) == 0) ? 1 : 0) :
                (((tmp % 4) == 0) ? 1 : 0));
        tmp = timeSpec->tm_mon;
        timeSpec->tm_mon += (timeSpec->tm_mday / mDays[tmp]);
        timeSpec->tm_mday %= mDays[tmp];
        timeSpec->tm_year += (timeSpec->tm_mon / 12);
        timeSpec->tm_mon %= 12;
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DS12887A_CheckIRQF
 *  This function is called to either clear or set the IRQF bit.  If so, the
 *  irqMask will also be set/cleared.  If set, then irqCond will also be
 *  signaled.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
static void AXP_DS12887A_CheckIRQF(void)
{
    bool irqHSet = ctrlC->irqf == 1;

    /*
     * If any of the flags are set and the interrupt enabled, then set the IRQF
     * bit in Control Register C.
     */
    if (((ctrlB->pie == 1) && (ctrlC->pf == 1)) ||
        ((ctrlB->aie == 1) && (ctrlC->af == 1)) ||
        ((ctrlB->uie == 1) && (ctrlC->uf == 1)))
    {
        ctrlC->irqf = 1;
    }
    else
    {
        ctrlC->irqf = 0;
    }

    /*
     * If we have some one to notify, then do so now.
     */
    if ((irqMutex != NULL) && (irqField != NULL))
    {
        pthread_mutex_lock(irqMutex);
        if (ctrlC->irqf == 0)
        {
            *irqField &= ~irqMask;
        }
        else
        {
            *irqField |= irqMask;
            if ((ctrlC->irqf == 1) && (irqHSet == false) && (irqCond != NULL))
            pthread_cond_signal(irqCond);
        }
        pthread_mutex_unlock(irqMutex);
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DS12887A_Notify
 *  This function is called when one of the timers triggers.
 *
 * Input Parameters:
 *  sv:
 *      A value indicating the timer that was just triggered.
 *
 * Output Parameters:
 *  None.
 *
 * Return Results:
 *  None.
 */
void AXP_DS12887A_Notify(sigval_t sv)
{
    AXP_DS12887A_LOCK;

    if (timersArmed == true)
    {
        while (ctrlA->uip == 1)
        {
            pthread_cond_wait(&rtcCond, &rtcMutex);
        }

        /*
        * Set the appropriate interrupt flag.
        */
        switch (sv.sival_int)
        {
            case AXP_DS12887A_TIMER_PERIOD:
                ctrlC->pf = 1; /* always assume the period expired */
                break;

            case AXP_DS12887A_TIMER_ALARM:
                ctrlC->af = 1;
                AXP_DS12887A_StartTimers(false);
                break;

            case AXP_DS12887A_TIMER_UPDATE:

                /*
                 * If the Update-In-Progress bit and the SET bit are not set, then
                 * we can indicate that the interrupt has been triggered.  We don't
                 * care what the initial value of the flag was, we just set it.
                 */
                if ((ctrlA->uip == 0) && (ctrlB->set == 0))
                ctrlC->uf = 1;    /* always assume the update occurred */
                break;

            default:
                break;
        }

        /*
         * If any of the flags are set and the interrupt enabled, then set the IRQF
         * bit in Control Register C.
         */
        AXP_DS12887A_CheckIRQF();
    }
    AXP_DS12887A_UNLOCK;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DS12887A_StartTimers
 *  This function is called to determine what timers need to be started or
 *  restarted.  The update timer always runs and does so at 1 second.  The
 *  periodic timer runs if the Periodic Interrupt Timer (PIR) bits in Control
 *  Register A indicates something other than NONE.  The Alarm timer runs if
 *  the Alarm Time fields indicate something to be timed.
 *
 * Input Parameters:
 *  all:
 *      A boolean value indicating if all the timers should be started or just
 *      the alarm one.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 *
 *  NOTE:    We are called with the rtcMuex locked.
 */
static void AXP_DS12887A_StartTimers(bool all)
{
    struct itimerspec ts;
    int flag = 0;
    u64 periods[] =
    {
        0,
        3906250,
        7812500,
        122070,
        244141,
        488281,
        976562,
        1953125,
        3906250,
        7812500,
        15625000,
        31250000,
        62500000,
        125000000,
        250000000,
        500000000
    };

    if (AXP_SYS_OPT2)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("AXP_DS12887A_StartTimers has been called.");
        AXP_TRACE_END()
    }

    /*
     * Set the timer specification for the periodic interrupt timer.
     */
    if ((all == true) && (ctrlA->rs != AXP_PIR_NONE))
    {
        if (AXP_SYS_OPT2)
        {
            AXP_TRACE_BEGIN();
            AXP_TraceWrite("AXP_DS12887A_StartTimers Periodic Timer Started "
                           "at %d nanoseconds",
                           periods[ctrlA->rs]);
            AXP_TRACE_END();
        }
        ts.it_interval.tv_nsec = periods[ctrlA->rs];
        ts.it_interval.tv_sec = 0;
        ts.it_value.tv_nsec = periods[ctrlA->rs];
        ts.it_value.tv_sec = 0;
        timer_settime(periodicTimer, 0, &ts, NULL);
        timersArmed = true;
    }

    /*
     * The alarm timer is a bit more complex.  There are don't care values and
     * also settings for hours, minutes, and seconds.
     *
     * If the hours, minutes and seconds are greater than their maximum binary
     * values, then it's a don't care condition and a timer gets triggered each
     * second.
     *
     * The three alarm bytes can be used in two ways. First, when the alarm
     * time is written in the appropriate hours, minutes, and seconds alarm
     * locations, the alarm interrupt is initiated at the specified time each
     * day, if the alarm-enable bit is high. In this mode, the ?0? bits in the
     * alarm registers and the corresponding time registers must always be
     * written to 0 (Table 2A and 2B). Writing the 0 bits in the alarm and/or
     * time registers to 1 can result in undefined operation.  The second use
     * condition is to insert a ?don?t care? state in one or more of the three
     * alarm bytes. The don?t care code is any hexadecimal value from C0 to FF.
     * The two most significant bits of each byte set the don?t-care condition
     * when at logic 1. An alarm is generated each hour when the don?t-care
     * bits are set in the hours byte.  Similarly, an alarm is generated every
     * minute with don?t-care codes in the hours and minute alarm bytes.  The
     * don?t-care codes in all three alarm bytes create an interrupt every
     * second.
     *
     * For this implementation a don't care value is a value greater than the
     * field can store (60 for seconds and minutes, and 24 for hours).
     *
     * So based on the above description, if all three alarm registers are
     * don't care values, then we trigger the timer every second.
     */
    ts.it_interval.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_value.tv_sec = 0;
    if ((ram[AXP_ADDR_SecondsAlarm] > 59) &&
        (ram[AXP_ADDR_MinutesAlarm] > 59) &&
        (ram[AXP_ADDR_HoursAlarm] > 23))
    {
        ts.it_interval.tv_sec = 1;  /* every 1 second */
        ts.it_value.tv_sec = 1;     /* in 1 second */
        if (AXP_SYS_OPT2)
        {
            AXP_TRACE_BEGIN();
            AXP_TraceWrite("AXP_DS12887A_StartTimers Alarm Timer Started "
                           "at 1 second and every 1 second");
            AXP_TRACE_END();
        }
    }

    /*
     * If the alarm minutes is a don't care then we trigger the timer every
     * minute.
     */
    else if (ram[AXP_ADDR_MinutesAlarm] > 59)
    {
        ts.it_interval.tv_sec = 60; /* every 1 minute */
        ts.it_value.tv_sec = 60; /* in 1 minute */
        if (AXP_SYS_OPT2)
        {
            AXP_TRACE_BEGIN();
            AXP_TraceWrite("AXP_DS12887A_StartTimers Alarm Timer Started "
                           "at 1 minute and every 1 minute");
            AXP_TRACE_END();
        }
    }

    /*
     * If the alarm hours is a don't care then we trigger the timer every
     * hour.
     */
    else if (ram[AXP_ADDR_HoursAlarm] > 23)
    {
        ts.it_interval.tv_sec = 3600; /* every 1 hour */
        ts.it_value.tv_sec = 3600; /* in 1 hour */
        if (AXP_SYS_OPT2)
        {
            AXP_TRACE_BEGIN();
            AXP_TraceWrite("AXP_DS12887A_StartTimers Alarm Timer Started "
                           "at 1 hour and every 1 hour");
            AXP_TRACE_END();
        }
    }

    /*
     * If the seconds are not a don't care, then we need to determine the next
     * time of the alarm.
     */
    else if (ram[AXP_ADDR_SecondsAlarm] <= 59)
    {
        struct tm gmt, nextA;
        time_t now;

        now = time(NULL);
        gmtime_r(&now, &gmt);
        nextA.tm_sec = gmt.tm_sec - ram[AXP_ADDR_SecondsAlarm];
        nextA.tm_min = gmt.tm_min - ram[AXP_ADDR_MinutesAlarm];
        nextA.tm_hour = gmt.tm_hour - ram[AXP_ADDR_HoursAlarm];
        nextA.tm_min += (nextA.tm_sec / 60);
        nextA.tm_sec %= 60;
        nextA.tm_hour += (nextA.tm_min / 60);
        nextA.tm_min %= 60;
        nextA.tm_hour %= 24;
        nextA.tm_mday = gmt.tm_mday;
        nextA.tm_mon = gmt.tm_mon;
        nextA.tm_year = gmt.tm_year;

        /*
        * If the alarm time is in the future, then schedule it for today.
        * Otherwise, schedule it for tomorrow.
        */
        if (gmt.tm_hour > nextA.tm_hour)
        {
            nextA.tm_mday++;
        }
        else if ((gmt.tm_hour == nextA.tm_hour) && (gmt.tm_min > nextA.tm_min))
        {
            nextA.tm_mday++;
        }
        else if ((gmt.tm_hour == nextA.tm_hour) &&
                 (gmt.tm_min == nextA.tm_min) &&
                 (gmt.tm_sec > nextA.tm_sec))
        {
            nextA.tm_mday++;
        }

        /*
        * Normalize the time, then get the absolute time for the next trigger.
        */
        AXP_DS12887A_Normalize(&nextA, false);
        ts.it_value.tv_sec = mktime(&nextA);
        flag = TIMER_ABSTIME;
        if (AXP_SYS_OPT2)
        {
            AXP_TRACE_BEGIN();
            AXP_TraceWrite("AXP_DS12887A_StartTimers Alarm Timer Started "
                           "at %d seconds",
                           ts.it_value.tv_sec);
            AXP_TRACE_END();
        }
    }
    if (ts.it_value.tv_sec != 0)
    {
        timer_settime(alarmTimer, flag, &ts, NULL);
        timersArmed = true;
    }

    /*
     * Set the timer specification to 1 second for the update timer.
     */
    if ((all == true) && (ctrlB->set == 0))
    {
        if (AXP_SYS_OPT2)
        {
            AXP_TRACE_BEGIN();
            AXP_TraceWrite("AXP_DS12887A_StartTimers Update Timer Started at "
                           "1 seconds.");
            AXP_TRACE_END();
        }
        ts.it_interval.tv_nsec = 0;
        ts.it_interval.tv_sec = 1;
        ts.it_value.tv_nsec = 0;
        ts.it_value.tv_sec = 1;
        timer_settime(updateTimer, 0, &ts, NULL);
        timersArmed = true;
    }

    if (AXP_SYS_OPT2)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("AXP_DS12887A_StartTimers returning.");
        AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;

}

/*
 * AXP_DS12887A_StopTimers
 *  This function is called to stop the timers because the reset was called,
 *  we are in the process of setting the the date and time fields, or the
 *  settings turned off the interrupt processing.  The update timer always
 *  runs, except during setting.  So, this function always just shuts the
 *  timers off.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 *
 *  NOTE:    We are called with the rtcMuex locked.
 */
static void AXP_DS12887A_StopTimers(void)
{
    struct itimerspec ts;

    if (timersArmed == true)
    {
        if (AXP_SYS_OPT2)
        {
            AXP_TRACE_BEGIN();
            AXP_TraceWrite("AXP_DS12887A_StopTimers has been called.");
            AXP_TRACE_END();
        }

        /*
        * Set the timer specification to all zeros, which will disarm the timer.
        */
        ts.it_interval.tv_nsec = 0;
        ts.it_interval.tv_sec = 0;
        ts.it_value.tv_nsec = 0;
        ts.it_value.tv_sec = 0;

        /*
        * Disarm all the timers.
        */
        timer_settime(periodicTimer, 0, &ts, NULL);
        timer_settime(alarmTimer, 0, &ts, NULL);
        timer_settime(updateTimer, 0, &ts, NULL);
        timersArmed = false;

        if (AXP_SYS_OPT2)
        {
            AXP_TRACE_BEGIN();
            AXP_TraceWrite("AXP_DS12887A_StopTimers returning.");
            AXP_TRACE_END();
        }
    }

    /*
     * Return back to the caller.
     */
    return;

}

/*
 * AXP_DS12887A_Initialize
 *  This function is called to initialize the RAM to all zeros.  It is assumed
 *  that the host system time will be the guest system time.  This assumption
 *  may be changed by making calls to the AXP_DS12887A_Write function and
 *  specifying the address of the date and time registers.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 *
 *  NOTE:    We are called with the rtcMuex locked.
 */
static void AXP_DS12887A_Initialize(void)
{
    sigevent_t se;
    int ii;

    /*
     * If tracing is turned on, then trace this call entry.
     */
    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("Dallas Semiconductor RTC (DS12887A) is initializing");
        AXP_TRACE_END()
    }

    /*
     * Set everything to zero.
     */
    for (ii = 0; ii < AXP_DS12887A_RAM_SIZE; ii++)
    {
        ram[ii] = 0;
    }

    /*
     * Now initialize a number of the control fields.
     */
    ctrlA->dv = AXP_DV_ON_CCE;
    ctrlA->rs = AXP_PIR_9765625;
    ctrlB->dm = 1; /* Binary Format */
    ctrlB->twentyFour = 1; /* 24 hour clock */
    ctrlB->dse = 1; /* Daylight Savings Enabled */
    ctrlD->vrt = 1; /* RAM and Time Valid */

    /*
     * Let's get all the timers created.
     *
     * Set the sigevent structure to cause the signal to be delivered by
     * creating a new thread.  The sigev_value will be used to indicate which
     * timer was triggered.
     */
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_notify_function = &AXP_DS12887A_Notify;
    se.sigev_notify_attributes = NULL;

    se.sigev_value.sival_int = AXP_DS12887A_TIMER_PERIOD;
    timer_create(CLOCK_REALTIME, &se, &periodicTimer);

    se.sigev_value.sival_int = AXP_DS12887A_TIMER_ALARM;
    timer_create(CLOCK_REALTIME, &se, &alarmTimer);

    se.sigev_value.sival_int = AXP_DS12887A_TIMER_UPDATE;
    timer_create(CLOCK_REALTIME, &se, &updateTimer);

    /*
     * Indicate that we should not be called again.
     */
    initialized = true;

    /*
     * If tracing is turned on, then trace this call entry.
     */
    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("DS12887A initialization complete.");
        AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DS12887A_Config
 *  This function is called to configure this code to be able to update another
 *  thread when the IRQF bit has been set/cleared.
 *
 * Input Parameters:
 *  cond:
 *      A pointer to a condition variable to be triggered when the IRQH bit has
 *      been set.
 *  mutex:
 *      A pointer to a mutex to be used to control the setting of the IRQ bit
 *      in the next parameter.
 *  irq_field:
 *      A pointer to an unsigned 64-bit value to have a bit set or cleared when
 *      the IRQF bit has been set/cleared.
 *  irq_mask:
 *      A value indicating the bit with in the field to be set/cleared when the
 *      IRQF bit has been set/cleared.
 *  dstIsEuropean:
 *      A boolean value to indicate that Daylight Savings Time (DST)
 *      calculations will be based on the European calendar.  If the DSE bit is
 *      not set, this flag has no effect.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_DS12887A_Config(
    pthread_cond_t *cond,
    pthread_mutex_t *mutex,
    u64 *irq_field,
    u64 irq_mask,
    bool dstIsEuropean)
{
    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("DS12887A Configure has been called.");
        AXP_TRACE_END();
    }

    irqCond = cond;
    irqMutex = mutex;
    irqField = irq_field;
    irqMask = irq_mask;
    ctrlD->eu = dstIsEuropean ? 1 : 0;

    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("DS12887A Configure returning.");
        AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DS12887A_Reset
 *  This function is called when a RESET occurs on the Real-Time Clock (RTC).
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_DS12887A_Reset(void)
{
    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("DS12887A Reset has been called.");
        AXP_TRACE_END();
    }

    /*
     * Go reset those things that need to be reset.
     */
    AXP_DS12887A_LOCK;
    ctrlB->pie = 0;
    ctrlB->aie = 0;
    ctrlB->uie = 0;
    ctrlB->sqwe = 0;
    ctrlC->irqf = 0;
    ctrlC->pf = 0;
    ctrlC->af = 0;
    ctrlC->uf = 0;
    AXP_DS12887A_StopTimers();
    AXP_DS12887A_CheckIRQF();
    AXP_DS12887A_UNLOCK;

    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("DS12887A Reset returning.");
        AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DS12887A_Write
 *  This function is called to write one of the ram address values.  If the SET
 *  bit is being set in Register B, then the UIP register is cleared and the
 *  current time is set.  Depending upon the address being written, additional
 *  functionality may be kicked off or performed.  If one of the date/time
 *  values is being written, then the value to be written is determined by
 *  subtracting the current time value from the one being set.  The resulting
 *  value will be used to determine what is returned when a read is performed.
 *
 * Input Parameters:
 *  addr:
 *      An unsigned byte value indicating the address to be written.  If it is
 *      one of the general use RAM areas, then we just write to it and get out.
 *      If it is one of the control areas, then the functionality of the other
 *      register locations may be changed.  Additionally, periodic interrupts
 *       may get triggered.
 *  value:
 *      An unsigned byte value to be written to the address location.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_DS12887A_Write(u8 addr, u8 value)
{
    bool startIRQF = false;

    AXP_DS12887A_LOCK;

    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("AXP_DS12887A_Write has been called.");
        AXP_TraceWrite("\taddr: 0x%02x(%d); value: 0x%02x(%d)",
                       addr,
                       addr,
                       value,
                       value);
        AXP_TRACE_END();
    }

    while (ctrlA->uip == 1)
    {
        pthread_cond_wait(&rtcCond, &rtcMutex);
    }
    switch (addr)
    {
        case AXP_ADDR_Seconds:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_Seconds updSec =
                {
                     .value = value
                };
                u8 sec;

                if (ctrlB->dm == 1)
                {
                    sec = updSec.bin.sec;
                }
                else
                {
                    sec = updSec.BCD.tenSec * 10 + updSec.BCD.sec;
                }
                ram[AXP_ADDR_Seconds] = currentTime.tm_sec - (i8) sec;
            }
            break;

        case AXP_ADDR_SecondsAlarm:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_SecondsAlarm updSec =
                {
                     .value = value
                };
                u8 sec;

                if (AXP_CHECK_DONT_CARE(value))
                {
                    ram[AXP_ADDR_SecondsAlarm] = 60;
                }
                else
                {
                    if (ctrlB->dm == 1)
                    {
                        sec = updSec.bin.sec;
                    }
                    else
                    {
                        sec = updSec.BCD.tenSec * 10 + updSec.BCD.sec;
                    }
                    ram[AXP_ADDR_SecondsAlarm] = sec + ram[AXP_ADDR_Seconds];
                }
            }
            break;

        case AXP_ADDR_Minutes:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_Minutes updMin =
                {
                     .value = value
                };
                u8 min;

                if (ctrlB->dm == 1)
                {
                    min = updMin.bin.min;
                }
                else
                {
                    min = updMin.BCD.tenMin * 10 + updMin.BCD.min;
                }
                ram[AXP_ADDR_Minutes] = currentTime.tm_min - (i8) min;
            }
            break;

        case AXP_ADDR_MinutesAlarm:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_MinutesAlarm updMin =
                {
                     .value = value
                };
                u8 min;

                if (AXP_CHECK_DONT_CARE(value))
                {
                    ram[AXP_ADDR_MinutesAlarm] = 60;
                }
                else
                {
                    if (ctrlB->dm == 1)
                    {
                        min = updMin.bin.min;
                    }
                    else
                    {
                        min = updMin.BCD.tenMin * 10 + updMin.BCD.min;
                    }
                    ram[AXP_ADDR_MinutesAlarm] = min + ram[AXP_ADDR_Minutes];
                }
            }
            break;

        case AXP_ADDR_Hours:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_Hours updHrs =
                {
                     .value = value
                };
                u8 hrs;

                if (ctrlB->dm == 1)
                {
                    hrs = updHrs.bin.hrs;
                }
                else
                {
                    hrs = updHrs.BCD.tenHrs * 10 + updHrs.BCD.hrs;
                }
                if (ctrlB->twentyFour == 0)
                {
                    if (updHrs.bin.amPm == 1)
                    {
                        hrs += 12;
                    }
                    hrs--;
                }
                ram[AXP_ADDR_Hours] = currentTime.tm_hour - (i8) hrs;
            }
            break;

        case AXP_ADDR_HoursAlarm:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_HoursAlarm updHrs =
                {
                     .value = value
                };
                u8 hrs;

                if (AXP_CHECK_DONT_CARE(value))
                {
                    ram[AXP_ADDR_HoursAlarm] = 24;
                }
                else
                {
                    if (ctrlB->dm == 1)
                    {
                        hrs = updHrs.bin.hrs;
                    }
                    else
                    {
                        hrs = updHrs.BCD.tenHrs * 10 + updHrs.BCD.hrs;
                    }
                    if (ctrlB->twentyFour == 0)
                    {
                        if (updHrs.bin.amPm == 1)
                        {
                            hrs += 12;
                        }
                        hrs--;
                    }
                    ram[AXP_ADDR_HoursAlarm] = hrs + ram[AXP_ADDR_Hours];
                }
            }
            break;

        case AXP_ADDR_Date:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_Date updDate =
                {
                     .value = value
                };
                u8 date;

                if (ctrlB->dm == 1)
                {
                    date = updDate.bin.date;
                }
                else
                {
                    date = updDate.BCD.tenDate * 10 + updDate.BCD.date;
                }
                ram[AXP_ADDR_Date] = currentTime.tm_mday - (i8) date;
            }
            break;

        case AXP_ADDR_Month:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_Month updMonth =
                {
                     .value = value
                };
                u8 month;

                if (ctrlB->dm == 1)
                {
                    month = updMonth.bin.month;
                }
                else
                {
                    month = updMonth.BCD.tenMonth * 10 + updMonth.BCD.month;
                }
                ram[AXP_ADDR_Month] = currentTime.tm_mon - (i8) (month - 1);
            }
            break;

        case AXP_ADDR_Year:
            if (ctrlB->set == 1)
            {
                AXP_DS12887A_Year updYear =
                {
                     .value = value
                };
                u8 year;
                u8 curYear;

                curYear = currentTime.tm_year -
                          (currentTime.tm_year >= 100 ? 100 : 0);
                if (ctrlB->dm == 1)
                {
                    year = updYear.bin.year;
                }
                else
                {
                    year = updYear.BCD.tenYear * 10 + updYear.BCD.year;
                }
                ram[AXP_ADDR_Year] = curYear - (i8) year;
            }
            break;

        case AXP_ADDR_ControlA:
            ctrlA->value = value & AXP_MASK_ControlA;
            break;

        case AXP_ADDR_ControlB:
            if (ctrlB->set == 0)
            {
                AXP_DS12887A_ControlB updVal =
                {
                     .value = value
                };
                time_t now;

                if (updVal.set == 1)
                {
                    AXP_DS12887A_StopTimers();
                    now = time(NULL);
                    gmtime_r(&now, &currentTime);
                }
            }
            else
            {
                AXP_DS12887A_ControlB updVal =
                {
                     .value = value
                };

                startIRQF = updVal.set == 0;
            }
            ctrlB->value = value;

            /*
            * If we need to start the timers, we also need to make sure the
            * DST information is set properly.
            */
            if (startIRQF == true)
            {
                struct tm    binTime;

                binTime.tm_sec = currentTime.tm_sec - ram[AXP_ADDR_Seconds];
                binTime.tm_min = currentTime.tm_min - ram[AXP_ADDR_Minutes];
                binTime.tm_hour = currentTime.tm_hour - ram[AXP_ADDR_Hours];
                binTime.tm_mday = currentTime.tm_mday - ram[AXP_ADDR_Date];
                binTime.tm_mon = currentTime.tm_mon - ram[AXP_ADDR_Month] + 1;
                binTime.tm_year = (currentTime.tm_year -
                                   (currentTime.tm_year >= 100 ? 100 : 0)) -
                                  ram[AXP_ADDR_Year];
                AXP_DS12887A_Normalize(&binTime, false);

                /*
                * This call will set the DST bit, we can ignore the rest of
                * the information.
                */
                (void) AXP_DS12887A_DST(binTime.tm_year,
                                        binTime.tm_mon,
                                        binTime.tm_mday,
                                        binTime.tm_hour,
                                        binTime.tm_min,
                                        binTime.tm_sec);
                AXP_DS12887A_StartTimers(true);
            }
            break;

        /*
        * These 2 locations are read-only, so just ignore any write that is
        * attempted.
        */
        case AXP_ADDR_ControlC:
        case AXP_ADDR_ControlD:
            break;

        /*
        * This location is calculated when day information is being read.
        */
        case AXP_ADDR_Day:
            break;

        /*
        * It is a RAM location.  Just write the data and be done with it.
        */
        default:
            ram[addr] = value;
            break;
    }

    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("AXP_DS12887A_Write returning.");
        AXP_TRACE_END();
    }

    AXP_DS12887A_CheckIRQF();
    AXP_DS12887A_UNLOCK;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DS12887A_Read
 *  This function is called to read from one of the RAM address values.  If the
 *  UIP bit is set and we are reading on of the Date/Time or Control Registers,
 *  then we wait for the bit to be cleared and then perform the read.  If it we
 *  are reading on of the general purpose locations, then the read can be
 *  performed immediately.
 *
 * Input Parameters:
 *  addr:
 *      An unsigned byte value indicating the address to be read.  If it is
 *      one of the general use RAM areas, then we just read from it and get
 *      out.  If it is the Control Register C, then after being read the
 *      contents of Register C will be cleared.  If it is one of the other
 *      Control Registers, then it is just read and returned back to the
 *      caller.
 *
 * Output Parameters:
 *  value:
 *      A pointer to an unsigned byte to have the contents of the RAM location
 *      saved and returned back to the caller.
 *
 * Return Values:
 *  None.
 */
void AXP_DS12887A_Read(u8 addr, u8 *value)
{
    AXP_DS12887A_Seconds *retSec = (AXP_DS12887A_Seconds *) value;
    AXP_DS12887A_SecondsAlarm *retSecA = (AXP_DS12887A_SecondsAlarm *) value;
    AXP_DS12887A_Minutes *retMin = (AXP_DS12887A_Minutes *) value;
    AXP_DS12887A_MinutesAlarm *retMinA = (AXP_DS12887A_MinutesAlarm *) value;
    AXP_DS12887A_Hours *retHrs = (AXP_DS12887A_Hours *) value;
    AXP_DS12887A_HoursAlarm *retHrsA = (AXP_DS12887A_HoursAlarm *) value;
    AXP_DS12887A_Day *retDay = (AXP_DS12887A_Day *) value;
    AXP_DS12887A_Date *retDate = (AXP_DS12887A_Date *) value;
    AXP_DS12887A_Month *retMonth = (AXP_DS12887A_Month *) value;
    AXP_DS12887A_Year *retYear = (AXP_DS12887A_Year *) value;
    struct tm binTime;
    struct tm binTimeA;
    time_t now;

    AXP_DS12887A_LOCK;

    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("AXP_DS12887A_Read has been called.");
        AXP_TraceWrite("\taddr: 0x%02x(%d)", addr, addr);
        AXP_TRACE_END();
    }

    while (ctrlA->uip == 1)
    {
        pthread_cond_wait(&rtcCond, &rtcMutex);
    }

    /*
     * We need to get the current time and then calculate the binary value to
     * be returned to the caller.  We calculate the binary time using the
     * following steps:
     *
     *  1) Get the current time.
     *  2) Calculate the binary seconds, minutes, hour, date, month and year
     *     values.  NOTE: Remove the century part of the current year.
     *  3) Normalize the binary values.
     *  4) Determine if DST has occurred and adjust accordingly.
     *  4) Calculate the alarm binary seconds, minutes, and hours.
     *  5) Normalize the alarm binary values.
     *
     * Step 1:
     */
    now = time(NULL);
    gmtime_r(&now, &currentTime);

    /*
     * Step 2:
     */
    binTime.tm_sec = currentTime.tm_sec - (i8) ram[AXP_ADDR_Seconds];
    binTime.tm_min = currentTime.tm_min - ram[AXP_ADDR_Minutes];
    binTime.tm_hour = currentTime.tm_hour - ram[AXP_ADDR_Hours];
    binTime.tm_mday = currentTime.tm_mday - ram[AXP_ADDR_Date];
    binTime.tm_mon = currentTime.tm_mon - ram[AXP_ADDR_Month] + 1;
    binTime.tm_year = (currentTime.tm_year -
                       (currentTime.tm_year >= 100 ? 100 : 0)) -
                      ram[AXP_ADDR_Year];

    /*
     * Step 3:
     */
    AXP_DS12887A_Normalize(&binTime, false);

    /*
     * Step 4:
     */
    if (ctrlB->dse == 1)
    {
        int    hourAdjust;

        hourAdjust = AXP_DS12887A_DST(binTime.tm_year,
                                      binTime.tm_mon,
                                      binTime.tm_mday,
                                      binTime.tm_hour,
                                      binTime.tm_min,
                                      binTime.tm_sec);
        binTime.tm_hour += hourAdjust;
        ram[AXP_ADDR_Hours] -= hourAdjust;
    }

    /*
     * Step 5:
     */
    if ((ram[AXP_ADDR_SecondsAlarm] < 60) &&
        (ram[AXP_ADDR_MinutesAlarm] < 60) &&
        (ram[AXP_ADDR_HoursAlarm] < 24))
    {
        binTimeA.tm_sec = currentTime.tm_sec- ram[AXP_ADDR_SecondsAlarm];
        binTimeA.tm_min = currentTime.tm_min - ram[AXP_ADDR_MinutesAlarm];
        binTimeA.tm_hour = currentTime.tm_hour - ram[AXP_ADDR_HoursAlarm];

        /*
         * Step 6:
         */
        AXP_DS12887A_Normalize(&binTimeA, true);
    }

    switch (addr)
    {
        case AXP_ADDR_Seconds:
            if (ctrlB->dm == 1)
            {
                retSec->bin.sec = binTime.tm_sec;
                retSec->bin.res = 0;
            }
            else
            {
                retSec->BCD.tenSec = binTime.tm_sec / 10;
                retSec->BCD.sec = binTime.tm_sec % 10;
                retSec->BCD.res = 0;
            }
            break;

        case AXP_ADDR_SecondsAlarm:
            if (ram[AXP_ADDR_SecondsAlarm] > 59)
            {
                retSecA->value = AXP_ALARM_DONT_CARE;
            }
            else
            {
                if (ctrlB->dm == 1)
                {
                    retSecA->bin.sec = binTimeA.tm_sec;
                    retSecA->bin.res = 0;
                }
                else
                {
                    retSecA->BCD.tenSec = binTimeA.tm_sec / 10;
                    retSecA->BCD.sec = binTimeA.tm_sec % 10;
                }
            }
            break;

        case AXP_ADDR_Minutes:
            if (ctrlB->dm == 1)
            {
                retMin->bin.min = binTime.tm_min;
                retMin->bin.res = 0;
            }
            else
            {
                retMin->BCD.tenMin = binTime.tm_min / 10;
                retMin->BCD.min = binTime.tm_min % 10;
                retMin->BCD.res = 0;
            }
            break;

        case AXP_ADDR_MinutesAlarm:
            if (ram[AXP_ADDR_MinutesAlarm] > 59)
            {
                retMinA->value = AXP_ALARM_DONT_CARE;
            }
            else
            {
                if (ctrlB->dm == 1)
                {
                    retMinA->bin.min = binTimeA.tm_min;
                    retMinA->bin.res = 0;
                }
                else
                {
                    retMinA->BCD.tenMin = binTimeA.tm_min / 10;
                    retMinA->BCD.min = binTimeA.tm_min % 10;
                }
            }
            break;

        case AXP_ADDR_Hours:
            retHrs->bin.amPm = 0;
            if ((ctrlB->twentyFour == 0) && (binTime.tm_hour >= 12))
            {
                binTime.tm_hour -= 11;
                retHrs->bin.amPm = 1;
            }
            if (ctrlB->dm == 1)
            {
                retHrs->bin.hrs = binTime.tm_hour;
                retHrs->bin.res = 0;
            }
            else
            {
                retHrs->BCD.tenHrs = binTime.tm_hour / 10;
                retHrs->BCD.hrs = binTime.tm_hour % 10;
                retHrs->BCD.res = 0;
            }
            break;

        case AXP_ADDR_HoursAlarm:
            if (ram[AXP_ADDR_HoursAlarm] > 23)
            {
                retHrsA->value = AXP_ALARM_DONT_CARE;
            }
            else
            {
                retHrs->bin.amPm = 0;
                if ((ctrlB->twentyFour == 0) && (binTimeA.tm_hour >= 12))
                {
                    binTimeA.tm_hour -= 11;
                    retHrsA->bin.amPm = 1;
                }
                if (ctrlB->dm == 1)
                {
                    retHrsA->bin.hrs = binTimeA.tm_hour;
                    retHrsA->bin.res = 0;
                }
                else
                {
                    retHrsA->BCD.tenHrs = binTimeA.tm_hour / 10;
                    retHrsA->BCD.hrs = binTimeA.tm_hour % 10;
                    retHrsA->BCD.res = 0;
                }
            }
            break;

        case AXP_ADDR_Day:
            binTime.tm_year += AXP_DOW_YEAR(binTime.tm_year);
            retDay->bin.day =
            AXP_DOW(binTime.tm_year, binTime.tm_mon, binTime.tm_mday) + 1;
            retDay->bin.res = 0;
            break;

        case AXP_ADDR_Date:
            if (ctrlB->dm == 1)
            {
                retDate->bin.date = binTime.tm_mday;
                retDate->bin.res = 0;
            }
            else
            {
                retDate->BCD.tenDate = binTime.tm_mday / 10;
                retDate->BCD.date = binTime.tm_mday % 10;
                retDate->BCD.res = 0;
            }
            break;

        case AXP_ADDR_Month:
            if (ctrlB->dm == 1)
            {
                retMonth->bin.month = binTime.tm_mon;
                retMonth->bin.res = 0;
            }
            else
            {
                retMonth->BCD.tenMonth = binTime.tm_mon / 10;
                retMonth->BCD.month = binTime.tm_mon % 10;
                retMonth->BCD.res = 0;
            }
            break;

        case AXP_ADDR_Year:
            if (ctrlB->dm == 1)
            {
                retYear->bin.year = binTime.tm_year;
                retYear->bin.res = 0;
            }
            else
            {
                retYear->BCD.tenYear = binTime.tm_year / 10;
                retYear->BCD.year = binTime.tm_year % 10;
            }
            break;

        case AXP_ADDR_ControlA:
            *value = ctrlA->value;
            break;

        case AXP_ADDR_ControlB:
            *value = ctrlB->value;
            break;

        case AXP_ADDR_ControlC:
            *value = ctrlC->value & AXP_MASK_ControlC;
            ctrlC->value = 0; /* This register is cleared upon reading */
            break;

        case AXP_ADDR_ControlD:
            *value = ctrlD->value & AXP_MASK_ControlD;
            break;

        /*
        * It is a RAM location.  Just read the data and be done with it.
        */
        default:
            *value = ram[addr];
            break;
    }

    if (AXP_SYS_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("AXP_DS12887A_Read returning.");
        AXP_TraceWrite("\taddr: 0x%02x(%d); value: 0x%02x(%d)",
                       addr,
                       addr,
                       *value,
                       *value);
        AXP_TRACE_END();
    }

    AXP_DS12887A_CheckIRQF();
    AXP_DS12887A_UNLOCK;

    /*
     * Return the value back to the caller.
     */
    return;
}
