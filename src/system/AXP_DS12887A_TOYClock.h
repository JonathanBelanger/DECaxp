/*
 * Copyright (C) Jonathan D. Belanger 2018.
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
 *	For Writes:
 *	    1)	If the SET - Update Transfer Inhibited bit is not set and we
 *		are not writing to one of the Control Registers or RAM
 *		locations, then swallow the write and just return back to the caller.
 *	    2)	If the SET is being set, get the current time from the host
 *		operating system and store it into a module variable for use
 *		later (may want to save it in a way that will make future
 *		calculations easier and faster.  Then return back to the
 *		caller.
 *	    3)	If the SET is set, then determine the difference between the
 *		saved host operating system time and the value being stored
 *		(remember the difference could be negative) and save that in
 *		the appropriate temporary register location and format.  Then
 *		return back to the caller.
 *	    4)	If the SET is set and is being cleared, then move the temporary
 *		registers to the real locations, clear the UIP flag, and return
 *		back to the caller.
 *
 *      For Reads:
 *	    1)	If the SET - Update Transfer Inhibited bit is not set and we
 *		are not reading from one of the Control Registers or RAM
 *		locations, then swallow the read and just return back to the
 *		caller.
 *	    2)	If the SET bit is set, then get the current time from the host
 *		operating system and store it in a current time location (for
 *		use until the SET bit is cleared).
 *	    2)	Using the current time plus the saved difference, calculate
 *		the value for the register being read, and return back to the
 *		caller.
 *
 * Revision History:
 *
 *	V01.000		24-May-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_DS12887A_TOYCLOCK_DEFS_
#define _AXP_DS12887A_TOYCLOCK_DEFS_

/*
 * Available RAM, in bytes.  For both RTC and RAM locations there is a total of
 * 128 bytes available.  The RTC functionality uses 14 of these locations,
 * leaving a total of 114 bytes of available general purpose RAM.
 *
 *	NOTE:	The century functionality is only available with the DS12C887
 *		and DS12C887A of the chips.  This implementation is for the
 *		DS12887A, which does not have this functionality.
 */
#define AXP_DS12887A_RAM_SIZE	128

/*
 * The following definition will be used for the TOY clock when reading from
 * and writing to it.
 *
 * Control Register A at address 0x0a
 *
 *  This register contains the rate and oscillator on/off settings and the
 *  flag to indicate that the date and time information has an update in
 *  progress.
 *
 *	Table 3: Periodic Interrupt Rate and Square-Wave Output Frequency
 *	------------------------------------------------------------------
 *	RATE SELECTOR		tPI PERIODIC		SQW OUTPUT
 *	BITS (RS)		INTERRUPT RATE		FREQUENCY
 *	------------------------------------------------------------------
 *	0000			None			None
 *	0001			3.90625ms		256Hz
 *	0010			7.8125ms		128Hz
 *	0011			122.070us		8.192kHz
 *	0100			244.141us		4.096kHz
 *	0101			488.281us		2.048kHz
 *	0110			976.5625us		1.024kHz
 *	0111			1.953125ms		512Hz
 *	1000			3.90625ms		256Hz
 *	1001			7.8125ms		128Hz
 *	1010			15.625ms		64Hz
 *	1011			31.25ms			32Hz
 *	1100			62.5ms			16Hz
 *	1101			125ms			8Hz
 *	1110			250ms			4Hz
 *	1111			500ms			2Hz
 *	------------------------------------------------------------------
 *
 *	Table: Oscillator Control Bits
 *	------------------------------------------------------------------
 *	CONTROL
 *	BITS (DV)	Description
 *	------------------------------------------------------------------
 *	000		Oscillator off.
 *	001		Oscillator off.
 *	010		Oscillator on and countdown chain enabled.
 *	011		Oscillator off.
 *	100		Oscillator off.
 *	101		Oscillator off.
 *	110		Oscillator on and countdown chain disabled.
 *	111		Oscillator on and countdown chain disabled.
 *	------------------------------------------------------------------
 */
typedef union
{
    u8 value;
    struct
    {
	u8 rs :4;		/* Rate Selector */
	u8 dv :3;		/* Oscillator on/off */
	u8 uip :1;		/* Update In Progress */
    };
} AXP_DS12887A_ControlA;
#define AXP_ADDR_ControlA	10
#define AXP_MASK_ControlA	0x7f

/*
 * Periodic Interrupt Rate (RS bits)
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
 * Square-Wave Output Frequency (RS bits)
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

/*
 * Oscillator Control
 */
#define AXP_DV_OFF0	0x0	/* Oscillator off */
#define AXP_DV_OFF1	0x1	/* Oscillator off */
#define AXP_DV_ON_CCE	0x2	/* Oscillator on, Countdown Change enabled */
#define AXP_DV_OFF03	0x3	/* Oscillator off */
#define AXP_DV_OFF04	0x4	/* Oscillator off */
#define AXP_DV_OFF05	0x5	/* Oscillator off */
#define AXP_DV_ON_CCD6	0x6	/* Oscillator on, Countdown Chain disabled */
#define AXP_DV_ON_CCD7	0x7	/* Oscillator on, Countdown Chain disabled */

/*
 * Control Register B at address 0x0b
 *
 *  This register controls the contents of the date and time registers.  There
 *  are flags to inhibit updates, as well as flags to determine what format the
 *  data and time data is stored within the registers.  There are also flags to
 *  handle interrupt processing, both alarm and periodic.
 *
 *	Bit 7:	SET. When the SET bit is 0, the update transfer functions
 *		normally by advancing the counts once per second. When the
 *		SET bit is written to 1, any update transfer is inhibited, and
 *		the program can initialize the time and calendar bytes without
 *		an update occurring in the midst of initializing. Read cycles
 *		can be executed in a similar manner. SET is a read/write bit
 *		and is not affected by RESET or internal functions of the
 *		device.
 *	Bit 6:	Periodic Interrupt Enable (PIE). The PIE bit is a read/write
 *		bit that allows the periodic interrupt flag (PF) bit in
 *		Register C to drive the IRQ pin low. When the PIE bit is set to
 *		1, periodic interrupts are generated by driving the IRQ pin low
 *		at a rate specified by the RS3–RS0 bits of Register A. A 0 in
 *		the PIE bit blocks the IRQ output from being driven by a
 *		periodic interrupt, but the PF bit is still set at the periodic
 *		rate. PIE is not modified by any internal device functions, but
 *		is cleared to 0 on RESET.
 *	Bit 5:	Alarm Interrupt Enable (AIE). This bit is a read/write bit
 *		that, when set to 1, permits the alarm flag (AF) bit in
 *		Register C to assert IRQ. An alarm interrupt occurs for each
 *		second that the three time bytes equal the three alarm bytes,
 *		including a don’t-care alarm code of binary 11XXXXXX. The AF
 *		bit does not initiate the IRQ signal when the AIE bit is set to
 *		0. The internal functions of the device do not affect the AIE
 *		bit, but is cleared to 0 on RESET.
 *	Bit 4:	Update-Ended Interrupt Enable (UIE). This bit is a read/write
 *		bit that enables the update-end flag (UF) bit in Register C to
 *		assert IRQ. The RESET pin going low or the SET bit going high
 *		clears the UIE bit.  The internal functions of the device do
 *		not affect the UIE bit, but is cleared to 0 on RESET.
 *	Bit 3:	Square-Wave Enable (SQWE). When this bit is set to 1, a
 *		square-wave signal at the frequency set by the rate-selection
 *		bits RS3–RS0 is driven out on the SQW pin. When the SQWE bit is
 *		set to 0, the SQW pin is held low. SQWE is a read/write bit and
 *		is cleared by RESET.  SQWE is low if disabled, and is high
 *		impedance when VCC is below VPF. SQWE is cleared to 0 on RESET.
 *	Bit 2:	Data Mode (DM). This bit indicates whether time and calendar
 *		information is in binary or BCD format.  The DM bit is set by
 *		the program to the appropriate format and can be read as
 *		required. This bit is not modified by internal functions or
 *		RESET.  A 1 in DM signifies binary data, while a 0 in DM
 *		specifies BCD data.
 *	Bit 1:	24/12. The 24/12 control bit establishes the format of the
 *		hours byte. A 1 indicates the 24-hour mode and a 0 indicates
 *		the 12-hour mode. This bit is read/write and is not affected by
 *		internal functions or RESET.
 *	Bit 0:	Daylight Saving Enable (DSE). This bit is a read/write bit that
 *		enables two daylight saving adjustments when DSE is set to 1.
 *		On the first Sunday in April, the time increments from
 *		1:59:59am to 3:00:00am.  On the last Sunday in October when the
 *		time first reaches 1:59:59am, it changes to 1:00:00am.  When
 *		DSE is enabled, the internal logic test for the first/last
 *		Sunday condition at midnight. If the DSE bit is not set when
 *		the test occurs, the daylight saving function does not operate
 *		correctly. These adjustments do not occur when the DSE bit is
 *		0. This bit is not affected by internal functions or RESET.
 */
typedef union
{
    u8 value;
    struct
    {
	u8 dse :1; 		/* Daylight Savings Enable */
	u8 twentyFour :1;	/* 24-12 Hour Mode */
	u8 dm :1;		/* Data Mode */
	u8 sqwe :1;		/* Square-Wave Enable */
	u8 uie :1;		/* Update-Ended Interrupt Enable */
	u8 aie :1;		/* Alarm Interrupt Enable */
	u8 pie :1;		/* Periodic Interrupt Enable */
	u8 set :1;		/* Update Transfer Function */
    };
} AXP_DS12887A_ControlB;
#define AXP_ADDR_ControlB	11

/*
 * Control Register C at address 0x0c
 *
 *  This register handles the interrupt processing  Paired with their enable
 *  counterpart pits, will drive the IRQ bit.
 *
 *	Bit 7:	Interrupt Request Flag (IRQF). This bit is set to 1 when any of
 *		the following are true:
 *			PF = PIE = 1
 *			AF = AIE = 1
 *			UF = UIE = 1
 *		Any time the IRQF bit is 1, the IRQ pin is driven low.  This
 *		bit can be cleared by reading Register C or with a RESET.
 *	Bit 6:	Periodic Interrupt Flag (PF). This bit is read-only and is set
 *		to 1 when an edge is detected on the selected tap of the
 *		divider chain. The RS3 through RS0 bits establish the periodic
 *		rate.  PF is set to 1 independent of the state of the PIE bit.
 *		When both PF and PIE are 1s, the IRQ signal is active and sets
 *		the IRQF bit.  This bit can be cleared by reading Register C or
 *		with a RESET.
 *	Bit 5:	Alarm Interrupt Flag (AF). A 1 in the AF bit indicates that the
 *		current time has matched the alarm time.  If the AIE bit is
 *		also 1, the IRQ pin goes low and a 1 appears in the IRQF bit.
 *		This bit can be cleared by reading Register C or with a RESET.
 *	Bit 4:	Update-Ended Interrupt Flag (UF). This bit is set after each
 *		update cycle. When the UIE bit is set to 1, the 1 in UF causes
 *		the IRQF bit to be a 1, which asserts the IRQ pin. This bit can
 *		be cleared by reading Register C or with a RESET.
 *	Bits 3 to 0: Unused. These bits are unused in Register C. These bits
 *		always read 0 and cannot be written.
 *
 *  This is a read-only register, but reading it causes the bits to be cleared.
 */
typedef union
{
    u8 value;
    struct
    {
	u8 res :4;
	u8 uf :1;		/* Update-Ended Interrupt Flag */
	u8 af :1;		/* Alarm Interrupt Flag */
	u8 pf :1;		/* Periodic Interrupt Flag */
	u8 irqf :1;		/* PF=PIE=1 or AF-AIE=1 or UF=UIE=1 */
    };
} AXP_DS12887A_ControlC;
#define AXP_ADDR_ControlC	12
#define AXP_MASK_ControlC	0xf0

/*
 * Control Register D at address 0x0d.
 *
 *  This register contains the bit to indicate that that the RAM and Time
 *  contents are valid.
 *
 *	Bit 7:	Valid RAM and Time (VRT). This bit indicates the condition of
 *		the battery connected to the VBAT pin.  This bit is not
 *		writeable and should always be 1 when read. If a 0 is ever
 *		present, an exhausted internal lithium energy source is
 *		indicated and both the contents of the RTC data and RAM data
 *		are questionable. This bit is unaffected by RESET.
 *	Bits 6 to 0: Unused. The remaining bits of Register D
 *		are not usable. They cannot be written and they always read 0.
 *
 *  This is a read-only register.
 */
typedef union
{
    u8 value;
    struct
    {
	u8 res :7;
	u8 vrt :1;		/* Valid RAM and Time */
    };
} AXP_DS12887A_ControlD;
#define AXP_ADDR_ControlD	13
#define AXP_MASK_ControlD	0x80

/*
 * Seconds Register at address 0x00.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the seconds.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|0-5|9 */
    {
	u8 sec :4;
	u8 tenSec :3;
	u8 res :1;
    } BCD;
    struct			/* Range: 0x00-0x3b */
    {
	u8 sec :6;
	u8 res :2;
    } bin;
} AXP_DS12887A_Seconds;
#define AXP_ADDR_Seconds	0
#define AXP_BCD_SecondsMask	0x7f
#define AXP_BIN_SecondsMask	0x3f

/*
 * Seconds Alarm Register at address 0x01.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the seconds for the alarm functionality.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|0-5|9 */
    {
	u8 sec :4;
	u8 tenSec :3;
	u8 res :1;
    } BCD;
    struct			/* Range: 0x00-0x3b */
    {
	u8 sec :6;
	u8 res :2;
    } bin;
} AXP_DS12887A_SecondsAlarm;
#define AXP_ADDR_SecondsAlarm	1

/*
 * Minutes Register at address 0x02.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the minutes.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|0-5|9 */
    {
	u8 min :4;
	u8 tenMin :3;
	u8 res :1;
    } BCD;
    struct			/* Range: 0x00-0x3b */
    {
	u8 min :6;
	u8 res :2;
    } bin;
} AXP_DS12887A_Minutes;
#define AXP_ADDR_Minutes	2
#define AXP_BCD_MinutesMask	0x7f
#define AXP_BIN_MinutesMask	0x3f

/*
 * Minutes Alarm Register at address 0x03.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the minutes for the alarm functionality.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|0-5|9 */
    {
	u8 min :4;
	u8 tenMin :3;
	u8 res :1;
    } BCD;
    struct			/* Range: 0x00-0x3b */
    {
	u8 min :6;
	u8 res :2;
    } bin;
} AXP_DS12887A_MinutesAlarm;
#define AXP_ADDR_MinutesAlarm	3

/*
 * Hours Register at address 0x04.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the hours.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|1-1|2 AM/PM or 0|0-2|3 */
    {
	u8 hrs :4;
	u8 tenHrs :2;
	u8 res :1;
	u8 amPm :1;
    } BCD;
    struct			/* Range: 0x01-0x0c AM/PM or 0x00-0x17 */
    {
	u8 hrs :5;
	u8 res :2;
	u8 amPm :1;
    } bin;
} AXP_DS12887A_Hours;
#define AXP_ADDR_Hours		4
#define AXP_BCD_12HoursMask	0x9f
#define AXP_BCD_24HoursMask	0x3f
#define AXP_BIN_12HoursMask	0x8f
#define AXP_BIN_24HoursMask	0x1f

/*
 * Hours Alarm Register at address 0x05.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the hours for the alarm functionality.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|1-1|2 AM/PM or 0|0-2|3 */
    {
	u8 hrs :4;
	u8 tenHrs :2;
	u8 res :1;
	u8 amPm :1;
    } BCD;
    struct			/* Range: 0x01-0x0c AM/PM or 0x00-0x17 */
    {
	u8 hrs :5;
	u8 res :2;
	u8 amPm :1;
    } bin;
} AXP_DS12887A_HoursAlarm;
#define AXP_ADDR_HoursAlarm	5

/*
 * Day (of the week) Register at address 0x06.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the day of the week.
 */
typedef union
{
    u8 value;
    struct			/* Range: 1-7 */
    {
	u8 day :3;
	u8 res :5;
    } BCD;
    struct			/* Range: 0x1-0x7 */
    {
	u8 day :3;
	u8 res :5;
    } bin;
} AXP_DS12887A_Day;
#define AXP_ADDR_Day		6
#define AXP_BCD_DayMask		0x07
#define AXP_BIN_DayMask		0x07

/*
 * Date Register at address 0x07.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the day of the month.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|1-3|1 */
    {
	u8 date :4;
	u8 tenDate :2;
	u8 res :4;
    } BCD;
    struct			/* Range: 0x01-0x1f */
    {
	u8 date :5;
	u8 res :3;
    } bin;
} AXP_DS12887A_Date;
#define AXP_ADDR_Date		7
#define AXP_BCD_DateMask	0x3f
#define AXP_BIN_DateMask	0x1f

/*
 * Month Register at address 0x08.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the month.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|1-1|2 */
    {
	u8 month :4;
	u8 tenMonth :1;
	u8 res :3;
    } BCD;
    struct			/* Range: 0x01-0x0c */
    {
	u8 month :4;
	u8 res :4;
    } bin;
} AXP_DS12887A_Month;
#define AXP_ADDR_Month		8
#define AXP_BCD_MonthMask	0x1f
#define AXP_BIN_MonthMask	0x0f

/*
 * Year Register at address 0x09.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the year.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|0-9|9 */
    {
	u8 year :4;
	u8 tenYear :4;
    } BCD;
    struct			/* Range: 0x00-0x63 */
    {
	u8 year :7;
	u8 res :1;
    } bin;
} AXP_DS12887A_Year;
#define AXP_ADDR_Year		9
#define AXP_BCD_YearMask	0xff
#define AXP_BIN_YearMask	0x7f

/*
 * Century Register at address 0x32.
 *
 * This structure is used to be able to properly decode a write and encode a
 * read to the century portion of the year.
 *
 *	NOTE:	I added the binary format.  It is not in the Dallas
 *		Semiconductor documentation.
 */
typedef union
{
    u8 value;
    struct			/* Range: 0|0-9|9 */
    {
	u8 century :4;
	u8 tenCentury :4;
    } BCD;
    struct			/* Range: 0x00-0x63 */
    {
	u8 century :7;
	u8 res :1;
    } bin;
} AXP_DS12887A_Century;
#define AXP_ADDR_Century	50
#define AXP_BCD_CenturyMask	0xff
#define AXP_BIN_CenturyMask	0x7f

/*
 * Alarm "Don't Care" code is a value of 11xxxxxx (0xc0-0xff)
 */
#define AXP_ALARM_DONT_CARE 0xc0
#define AXP_CHECK_DONT_CARE(value)  \
    (((value) & AXP_ALARM_DONT_CARE) == AXP_ALARM_DONT_CARE)

/*
 * Interval Timer IDs.
 */
#define AXP_DS12887A_TIMER_PERIOD	1
#define AXP_DS12887A_TIMER_ALARM	2
#define AXP_DS12887A_TIMER_UPDATE	3

/*
 * External prototypes.
 */
void AXP_DS12887A_Reset(void);
void AXP_DS12887A_Write(u8, u8);
void AXP_DS12887A_Read(u8, u8 *);

#endif	/* _AXP_DS12887A_TOYCLOCK_DEFS_ */
