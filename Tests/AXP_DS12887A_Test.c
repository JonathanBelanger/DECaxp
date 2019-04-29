/*
 * Copyright (C) Jonathan D. Belanger 2018.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied only
 * in accordance with the terms of such license and with the inclusion of the
 * above copyright notice.	This software or any other copies thereof may not
 * be provided or otherwise made available to any other person.	 No title to
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
 *	This module contains the code to test the DS12887A Real Time Clock
 *	code.
 *
 * Revision History:
 *
 *	V01.000		28-May-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "CommonUtilities/AXP_Utility.h"
#include "Devices/TOYClock/AXP_DS12887A_TOYClock.h"

/*
 * A step contains a set of characters that contain the following format:
 *
 *  <Command>[<Address><Value>]
 *
 * where:
 *  <Command>:
 *	W = Write - The <Address> is the location to be written and <Value> is
 *	    	    the value to be written to that location.
 *	R = Read  - The <Address> is the location to be read from and <Value> is
 *	    	    what is expected to be returned from that address.
 *	S = Reset - The <Address> and <Value> fields are optional/ignored
 *	H = Wait  - The <Address> field is ignored and the <Value> field
 *		    represents the number of seconds that the code should wait
 *		    before continuing on.  This can be used to wait while we
 *		    are hoping to see the interrupt bit getting set.
 *	D = Done  - This is the last command and indicates there are no more
 *		    tests steps for this test case.
 *  <Address>:
 *	A 2-character hexadecimal value between 00 and 7f
 *  <Value>
 *	A 2-character hexadecimal value between 00 and ff
 */

/*
 * StepCount is the number of steps to be executed and tested.
 */
typedef struct
{
    char cmd;
    u8 address;
    u8 saveVal;
} TestStep;
typedef struct
{
    char *testName;
    TestStep steps[30];
} TestSteps;

static bool interruptsEnabled = false;

pthread_t	threadID;
bool		threadStarted = false;
pthread_cond_t	irqCond;
pthread_mutex_t	irqMutex;
u64		irqH = 0;
u64		periodicInterrupt = 0;
u64		updateInterrupt = 0;
u64		alarmInterrupt = 0;
#define IRQMask 0x0000001000000000ll

static char *regNames[] =
{
    "Seconds",
    "Alarm Seconds",
    "Minutes",
    "Alarm Minutes",
    "Hours",
    "Alarm Hours",
    "Day of Week",
    "Day of Month",
    "Month",
    "Year",
    "Control Register A",
    "Control Register B",
    "Control Register C",
    "Control Register D",
    "RAM"
};
#define AXP_ADDR_RAM		0x0e
#define AXP_REG_NAME(addr)	\
    (((addr) < AXP_ADDR_RAM) ? regNames[(addr)] : regNames[AXP_ADDR_RAM])

void *irqHMonitoring(void *arg)
{
    AXP_DS12887A_ControlC	value;

    printf("...IRQ monitoring starting\n");
    pthread_mutex_lock(&irqMutex);
    threadStarted = true;
    pthread_cond_signal(&irqCond);	/* the main waits for this signal */

    while (true)
    {
	while (irqH == 0)
	    pthread_cond_wait(&irqCond, &irqMutex);
	if (irqH != 0)
	{

	    /*
	     * This call will reset/clear the bits in Control Register C.
	     * Because the Read can set/clear the IRQ bit, and as a result
	     * lock the mutex for it, we need to unlock the mutex before the
	     * read.
	     */
	    pthread_mutex_unlock(&irqMutex);
	    AXP_DS12887A_Read(0x0c, &value.value);
	    pthread_mutex_lock(&irqMutex);

	    /*
	     * Let's see what interrupt flag had been set.
	     */
	    if (value.pf == 1)
		periodicInterrupt++;
	    if (value.af == 1)
		alarmInterrupt++;
	    if (value.uf == 1)
		updateInterrupt++;
	}
    }

    printf("...IRQ monitoring done.\n");
    pthread_mutex_unlock(&irqMutex);
    return(NULL);
}

static bool executeTest(int testNum, TestSteps *test)
{
    bool retVal = true;
    int ii;
    u8 readVal;

    printf(
	"Test %d: %s Starting...\n",
	testNum,
	test->testName);

    /*
     * Loop until we either exhaust the tests in this set or detect a failure.
     */
    for (ii = 0; ((test->steps[ii].cmd != 'D') && (retVal == true)); ii++)
    {
	printf(
	    "\tStep %d (%s): Address: 0x%02x (%s); Value: 0x%02x\n",
	    ii,
	    (test->steps[ii].cmd == 'W' ?
		"Write" :
		(test->steps[ii].cmd == 'R' ?
		    "Read" :
		    (test->steps[ii].cmd == 'S' ? "Reset" : "Wait"))),
	    test->steps[ii].address,
	    (test->steps[ii].cmd == 'H' ?
		"N/A" :
		AXP_REG_NAME(test->steps[ii].address)),
	    test->steps[ii].saveVal);

	/*
	 * Based on the command, call the correct interface function.
	 */
	switch (test->steps[ii].cmd)
	{
	    case 'W':

		/*
		 * If we are writing to Register B and are setting or clearing
		 * the SET bit, then we are also enabling or disabling the
		 * interrupt processing in the DS12887A chip.
		 */
		if (test->steps[ii].address == 0x0b)
		{
		    if ((test->steps[ii].saveVal & 0x80) == 0x80)
			interruptsEnabled = false;
		    else
			interruptsEnabled = true;
		}
		AXP_DS12887A_Write(
			test->steps[ii].address,
			test->steps[ii].saveVal);

		/*
		 * The way write works, there is no way to know if it did
		 * what it was told.  So, there is nothing to check for
		 * success or failure.
		 */
		break;

	    case 'R':
		AXP_DS12887A_Read(test->steps[ii].address, &readVal);

		/*
		 * If the value read from the interface is the same as we
		 * expected, then this test was successful.
		 */
		retVal = (readVal == test->steps[ii].saveVal) ||
			(test->steps[ii].saveVal == 0xff);
		if ((retVal == false) || (test->steps[ii].saveVal == 0xff))
		    printf(
			"\tAddress: 0x%02x (%s); Expected: 0x%02x; Got: 0x%02x\n",
			test->steps[ii].address,
			AXP_REG_NAME(test->steps[ii].address),
			test->steps[ii].saveVal,
			readVal);
		break;

	    case 'S':
		AXP_DS12887A_Reset();

		/*
		 * The way write works, there is no way to know if it did what
		 * it was told.  So, there is nothing to check for
		 * success or failure.
		 */
		break;

	    case 'H':
		sleep(test->steps[ii].saveVal);

		/*
		 * This is not actually testing the TOY clock interface, but is
		 * a way to allow the TOY clock to do some of its processing.
		 * So, there is nothing to check for success or failure.
		 */
		break;

	}
    }

    printf(
	"Test %d: ...%s %s.\n",
	testNum,
	test->testName,
	(retVal == true ? "Passed" : "Failed"));

    /*
     * Return the results back to the caller.
     */
    return (retVal);
}

int main()
{
    bool pass = true;
    int ii;
    TestSteps tests[] =
    {
        {
            "Write/Read SET bit to 1 in Register B",
	    {
		{'W', AXP_ADDR_ControlB, 0x80},
		{'R', AXP_ADDR_ControlB, 0x80},
		{'D', 0x00, 0x00}
	    }
        },
        {
	    "Write/Read DM/DSE bits to 010/0110 in Register B",
	    {
		{'W', AXP_ADDR_ControlB, 0x86}, /* Leave SET bit, disabling interrupt processing */
		{'R', AXP_ADDR_ControlB, 0x86},
		{'D', 0x00, 0x00}
	    }
        },
        {
	    "Write/Read DV/RS bits in Register A",
	    {
		{'W', AXP_ADDR_ControlA, 0x26},  /* Period rate: 976.5625 microseconds */
		{'R', AXP_ADDR_ControlA, 0x26},
		{'D', 0x00, 0x00}
	    }
        },
        {
	    "Write/Read Alarm Seconds bits to don't care minutes and hours remain "
	    "0 to disable alarm interrupt",
	    {
		{'W', AXP_ADDR_SecondsAlarm, 0xc0},
		{'R', AXP_ADDR_SecondsAlarm, 0xc0},
		{'D', 0x00, 0x00}
	    }
        },
        {
	    "Write/Read Time to 9:00:00am",
	    {
		{'W', AXP_ADDR_Seconds, 0x00},
		{'W', AXP_ADDR_Minutes, 0x00},
		{'W', AXP_ADDR_Hours, 0x09},
		{'R', AXP_ADDR_Minutes, 0x00},
		{'R', AXP_ADDR_Hours, 0x09},
		{'R', AXP_ADDR_Seconds, 0xff},
		{'D', 0x00, 0x00}
	    }
        },
        {
	    "Write/Read Date to June 16, 1987",
	    {
		{'W', AXP_ADDR_Date, 0x10},
		{'W', AXP_ADDR_Month, 0x06},
		{'W', AXP_ADDR_Year, 0x57},
		{'R', AXP_ADDR_Year, 0x57},
		{'R', AXP_ADDR_Month, 0x06},
		{'R', AXP_ADDR_Date, 0x10},
		{'D', 0x00, 0x00}
	    }
        },
        {
	    "Write/Read to various RAM locations",
	    {
		{'W', 0x50, 0xf0},
		{'W', 0x70, 0x86},
		{'W', 0x30, 0x7f},
		{'R', 0x50, 0xf0},
		{'R', 0x70, 0x86},
		{'R', 0x30, 0x7f},
		{'D', 0x00, 0x00}
	    }
        },
        {
	    "Write/Read SET bit to 0 in Register B, leave DM/DSE as set above",
	    {
		{'W', AXP_ADDR_ControlB, 0x76}, /* Clear SET bit, disabling interrupt processing */
		{'R', AXP_ADDR_ControlB, 0x76},
		{'D', 0x00, 0x00}
	    }
        },
	{
	    "Waiting for things to happen",
	    {
		{'H', 0x00, 10},	/* in seconds */
		{'D', 0x00, 0x00}
	    }
	},
	{
	    "Let's try something with Daylight Savings Time (Spring)",
	    {
		{'W', AXP_ADDR_ControlB, 0x87}, /* Set SET and DSE bits first */
		/* Set time Sunday, March 11, 2018 01:59:59 */
		{'W', AXP_ADDR_Month, 3},
		{'W', AXP_ADDR_Date, 11},
		{'W', AXP_ADDR_Year, 18},
		{'W', AXP_ADDR_Hours, 1},
		{'W', AXP_ADDR_Minutes, 59},
		{'W', AXP_ADDR_Seconds, 59},
		{'W', AXP_ADDR_ControlB, 0x07}, /* Clear SET bit */
		{'H', 0x00, 10},	/* in seconds */
		/* Time should be returned as March 11, 2018 03:00 */
		{'R', AXP_ADDR_Day, 1},
		{'R', AXP_ADDR_Month, 3},
		{'R', AXP_ADDR_Date, 11},
		{'R', AXP_ADDR_Year, 18},
		{'R', AXP_ADDR_Seconds, 0xff},
		{'R', AXP_ADDR_Minutes, 00},
		{'R', AXP_ADDR_Hours, 3},
		{'D', 0x00, 0x00}
	    }
	},
	{
	    "Let's try something with Daylight Savings Time (Fall)",
	    {
		{'W', AXP_ADDR_ControlB, 0x87}, /* Set SET and DSE bits first */
		/* Set time Sunday, November 4, 2018 01:59:59 */
		{'W', AXP_ADDR_Month, 11},
		{'W', AXP_ADDR_Date, 4},
		{'W', AXP_ADDR_Year, 18},
		{'W', AXP_ADDR_Hours, 1},
		{'W', AXP_ADDR_Minutes, 59},
		{'W', AXP_ADDR_Seconds, 59},
		{'W', AXP_ADDR_ControlB, 0x07}, /* Clear SET bit */
		{'H', 0x00, 10},	/* in seconds */
		/* Time should be returned as November 4, 2018 01:00 */
		{'R', AXP_ADDR_Day, 1},
		{'R', AXP_ADDR_Month, 11},
		{'R', AXP_ADDR_Date, 4},
		{'R', AXP_ADDR_Year, 18},
		{'R', AXP_ADDR_Seconds, 0xff},
		{'R', AXP_ADDR_Minutes, 00},
		{'R', AXP_ADDR_Hours, 1},
		{'R', AXP_ADDR_Day, 1},
		{'R', AXP_ADDR_Month, 11},
		{'R', AXP_ADDR_Date, 4},
		{'R', AXP_ADDR_Year, 18},
		{'R', AXP_ADDR_Seconds, 0xff},
		{'R', AXP_ADDR_Minutes, 00},
		{'R', AXP_ADDR_Hours, 1},
		{'D', 0x00, 0x00}
	    }
	},
        {
            NULL
        }
    };

    /*
     * Let's turn on tracing first.
     */
    if (AXP_TraceInit() == true)
    {
	pthread_cond_init(&irqCond, NULL);
	pthread_mutex_init(&irqMutex, NULL);
	pthread_mutex_lock(&irqMutex);
	pthread_create(&threadID, NULL, &irqHMonitoring, NULL);
	while (threadStarted == false)
	    pthread_cond_wait(&irqCond, &irqMutex);
	pthread_mutex_unlock(&irqMutex);
	AXP_DS12887A_Config(&irqCond, &irqMutex, &irqH, IRQMask, false);

	for (ii = 0; ((tests[ii].testName != NULL) && (pass == true)); ii++)
	    pass = executeTest(ii + 1, &tests[ii]);

	/*
	 * Check that the timer interrupts worked as expected.
	 */
	printf(
	    "Test %d: Interrupt Processing (p: %llu, a: %llu, u: %llu) ",
	    ii,
	    periodicInterrupt,
	    alarmInterrupt,
	    updateInterrupt);
	if ((periodicInterrupt > 0) && (updateInterrupt > 0))
	    printf("Passed.\n");
	else
	{
	    printf("Failed (");
	    if (periodicInterrupt == 0)
	    {
		printf("Periodic");
		if (updateInterrupt == 0)
		    printf(", ");
	    }
	    if (updateInterrupt == 0)
		printf("Update");
	    printf(").\n");
	}

    }

    /*
     * Return back to the caller.
     */
    return (0);
}
