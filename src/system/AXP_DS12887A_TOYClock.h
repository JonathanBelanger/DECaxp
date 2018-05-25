/*
 * Copywrite comment
 *
 * Description:
 *  The implementation of the Real Time Clock (RTC), also known as Time of Year
 *  (TOY) clock.  Other implementations of this functionalty simply read the
 *  byte at the specified address, which is fine.  The issue is that the write
 *  actually stored the value in the byte, but then proceeded to get the system
 *  time and then update it with the current time, as defined in the system.
 *  What this does is mean that the emulator would have to have the same exact
 *  time as the host operating system.  This implementation will take into
 *  consideration that someone might like to have the host and guest operating
 *  systems have different system times.  To this end, the code will perform
 *  the following steps:
 *
 *      For Writes:
 *          1)  If the UIP - Update In Progress bit is not set and we are not
 *              writing to one of the Control Registers or RAM locations, then
 *              swallow the write and just return back to the caller.
 *          2)  If the UIP is being set, get the current time from the host
 *              operating system and store it into a module variable for use
 *              later (may want to save it in a way that will make future
 *              calculations easier and faster.  Then return back to the
 *              caller.
 *          3)  If the UIP is set, then determine the difference between the
 *              saved host operating system time and the value being stored
 *              (remember the difference could be negative) and save that in
 *              the appropriate temporary register location and format.  Then
 *              return back to the caller.
 *          4)  If the UIP is set and is being cleared, then move the temporary
 *              registers to the real locations, clear the UIP flag, and return
 *              back to the caller.
 *
 *      For Reads:
 *          1)  Get the current time from the host operating system.
 *          2)  If the previous time from the host operating system has not
 *              been set, then do so now.
 *          3)  If the previous time is set, then determine if the previous
 *              time is more than 2 seconds less than the current time.  Take
 *              into consideration that the current time could be only 2
 *              seconds after the previous time, but be in an entirely
 *              different year.  For example: Sunday 31-DEC-2017 23:59:58 plus
 *              2 seconds will be Monday 01-JAN-2018 00:00:00.
 *          4)  Using the previous time plus the saved difference, calculate
 *              the value for the register being read, and return back to the
 *              caller.
 *
 *  NOTE:   The code can be really simplified if all that is stored is the
 *          difference and the registers are calculated at the time of being
 *          read.
 */
#ifndef _TOY_
#define _TOY_

/*
 * The following definition will be used for the TOY clock when reading and
 * writing to it.
 */
typedef struct
{
    union
    {
        u8      value;
        struct                  /* Range: 00-59 */
        {
            u8  sec : 4;
            u8  tenSec : 3;
            u8  res : 1;
        } BCD;
        struct                  /* Range: 00-3b */
        {
            u8  sec : 6;
            u8  res : 2;
        } bin;
    } seconds;                  /* Address: 00h */
    union
    {
        u8      value;
        struct                  /* Range: 00-59 */
        {
            u8  sec : 4;
            u8  tenSec : 3;
            u8  res : 1;
        } BCD;
        struct                  /* Range: 00-3b */
        {
            u8  sec : 6;
            u8  res : 2;
        } bin;
    } secondsAlarm;             /* Address: 01h */
    union
    {
        u8      value;
        struct                  /* Range: 00-59 */
        {
            u8  min : 4;
            u8  tenmin : 3;
            u8  res : 1;
        } BCD;
        struct                  /* Range: 00-3b */
        {
            u8  min : 6;
            u8  res : 2;
        } bin;
    } minutes;                  /* Address: 02h */
    union
    {
        u8      value;
        struct                  /* Range: 00-59 */
        {
            u8  min : 4;
            u8  tenMin : 3;
            u8  res : 1;
        } BCD;
        struct                  /* Range: 00-3b */
        {
            u8  min : 6;
            u8  res : 2;
        } bin;
    } minutesAlarm;             /* Address: 03h */
    union
    {
        u8      value;
        struct                  /* Range: 01-12 AM/PM or 00-23 */
        {
            u8  hrs : 4;
            u8  tenHrs : 2;
            u8  res : 1;
            u8  amPm : 1;
        } BCD;
        struct                  /* Range: 01-0c AM/PM or 00-17 */
        {
            u8  hrs : 5;
            u8  res : 2;
            u8  amPm : 1;
        } bin;
    } hours;                    /* AddressL 04h */
    union
    {
        u8      value;
        struct                  /* Range: 01-12 AM/PM or 00-23 */
        {
            u8  hrs : 4
            u8  tenHrs : 2;
            u8  res : 1;
            u8  amPm : 1;
        } BCD;
        struct                  /* Range: 01-0c AM/PM or 00-17 */
        {
            u8  hrs : 5;
            u8  res : 2;
            u8  amPm : 1;
        } bin;
    } hoursAlarm;               /* Address: 05h */
    union
    {
        u8      value;
        struct                  /* Range: 01-07 */
        {
            u8  day : 2;
            u8  res : 6;
        } BCD;
        struct                  /* Range: 01-07 */
        {
            u8  day : 2;
            u8  res : 6;
        } bin;
    } day;                      /* Address: 06h */
    union
    {
        u8      value;
        struct                  /* Range: 01-31 */
        {
            u8  date : 4;
            u8  tenDate : 2;
            u8  res : 4;
        } BCD;
        struct                  /* Range: 01-1f */
        {
            u8  date : 5;
            u8  res : 3;
        } bin;
    } date;                     /* Address: 07h */
    union
    {
        u8      value;
        struct                  /* Range: 01-12 */
        {
            u8  month : 4;
            u8  tenMonth : 1;
            u8  res : 3;
        } BCD;
        struct                  /* Range: 01-0c */
        {
            u8  month : 4;
            u8  res : 4;
        } bin;
    } month;                    /* Address: 08h */
    union
    {
        u8      value;
        struct                  /* Range: 00-99 */
        {
            u8  year : 4;
            u8  tenYear : 4;
        } BCD;
        struct                  /* Range: 00-63 */
        {
            u8  year : 7;
            u8  res : 1;
        } bin;
    } year;                     /* Address: 09h */
    union
    {
        u8      value;
        struct
        {
            u8  rs0 : 4;        /* Rate Selector */
            u8  dv : 3;         /* Occilator on/off */
            u8  uip : 1;        /* Update In Progress */
        };
    } controlA;                 /* Address: 0ah */
    union
    {
        u8      value;
        struct
        {
            u8  dse : 1;        /* Daylight Savings Enable */
            u8  twentyFour : 1; /* 24-12 Hour Mode */
            u8  dm : 1;         /* Data Mode */
            u8  sqwe : 1;       /* Square-Wave Enable */
            u8  uie : 1;        /* Update-Ended Interrupt Enable */
            u8  aie : 1;        /* Alarm Interrupt Enable */
            u8  pie : 1;        /* Periodic Interrupt Enable */
            u8  set : 1;        /* Update Transfer Function */
        };
    } controlB;                 /* Address: 0bh */
    union
    {
        u8      value;
        struct
        {
            u8  res : 4;
            u8  uf : 1;         /* Update-Ended Interrupt Flag */
            u8  af : 1;         /* Alarm Interrupt Flag */
            u8  pf : 1;         /* Periodic Interrupt Flag */
            u8  irqf : 1;       /* PF=PIE=1 or AF-AIE=1 or UF=UIE=1 */
        };
    } controlC;                 /* Address: 0ch */
    union
    {
        u8      value;
        struct
        {
            u8  res : 7;
            u8  vrt : 1;        /* Valid RAM and Time */
        };
    } controlD;                 /* Address: 0dh */
    u8  ramLow[36]              /* Addresses: 0eh-31h */
    union
    {
        u8      value;          /* Range: 00-99 */
        struct
        {
            u8  century : 4;
            u8  tenCentury : 4;
        } BCD;
    } century;
    u8  ramHigh[77]             /* Addresses: 33h-7fh */
} AXP_DS12887A_RTC;

/*
 * Alarm "Don't Care" code is a value of 11xxxxxx (0xc0-0xff)
 */
#define AXP_ALARM_DONT_CARE 0xc0
#define AXP_CHECK_DONT_CARE(value)  \
    (((value) & AXP_ALARM_DONT_CARE) == AXP_ALARM_DONT_CARE)

/*
 * Periodic Interrupt Rate
 */
#define AXP_PIR_NONE    0x0     /* None */
#define AXP_PIR_390625  0x1     /* 3.90625ms */
#define AXP_PIR_78125   0x2     /* 7.8125 ms */
#define AXP_PIR_122070  0x3     /* 122.070us */
#define AXP_PIR_244141  0x4     /* 244.141us */
#define AXP_PIR_488281  0x5     /* 488.281us */
#define AXP_PIR_9765625 0x6     /* 976.5625us */
#define AXP_PIR_1953125 0x7     /* 1.953125ms */
#define AXP_PIR_390625  0x8     /* 3.90625ms */
#define AXP_PIR_78125   0x9     /* 7.8125ms */
#define AXP_PIR_15625   0xa     /* 15.625ms */
#define AXP_PIR_3125    0xb     /* 31.25ms */
#define AXP_PIR_625     0xc     /* 62.5ms */
#define AXP_PIR_125     0xd     /* 125ms */
#define AXP_PIR_250     0xe     /* 250ms */
#define AXP_PIR_500     0xf     /* 500ms */

/*
 * Square-Wave Output Frequency
 */
#define AXP_SQW_NONE    0x0     /* None */
#define AXP_SQW_256     0x1     /* 256Hz */
#define AXP_SQW_128     0x2     /* 128Hz */
#define AXP_SQW_8192    0x3     /* 8.192kHz */
#define AXP_SQW_4096    0x4     /* 4.096kHz */
#define AXP_SQW_2048    0x5     /* 2.048kHz */
#define AXP_SQW_1024    0x6     /* 1.024kHz */
#define AXP_SQW_512     0x7     /* 512Hz */
#define AXP_SQW_256     0x8     /* 256Hz */
#define AXP_SQW_128     0x9     /* 128Hz */
#define AXP_SQW_64      0xa     /* 64Hz */
#define AXP_SQW_32      0xb     /* 32Hz */
#define AXP_SQW_16      0xc     /* 16Hz */
#define AXP_SQW_8       0xd     /* 8Hz */
#define AXP_SQW_4       0xe     /* 4Hz */
#define AXP_SQW_2       0xf     /* 2Hz */

#endif	/* _TOY_ */