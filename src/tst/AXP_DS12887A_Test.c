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
#include "AXP_Utility.h"
#include "AXP_DS12887A_TOYClock.h"

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
    char *testName;
    char *steps[10];
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
    u8 address;
    u8 saveVal;
    u8 readVal;

    /*
     * Loop until we either exhaust the tests in this set or detect a failure.
     */
    for (ii = 0; ((test->steps[ii][0] != 'D') && (retVal == true)); ii++)
    {

	/*
	 * If the command is not reset, then the address and value fields need
	 * to be converted from characters to a hexadecimal value.
	 */
	if (test->steps[ii][0] != 'S')
	{

	    /*
	     * Convert the address high nibble from characters to a hexadecimal
	     * value.
	     */
	    if ((test->steps[ii][1] >= '0') && (test->steps[ii][1] <= '9'))
	    {
		address = test->steps[ii][1] - '0';
	    }
	    else
	    {
		address = (test->steps[ii][1] - 'a') + 10;
	    }

	    /*
	     * Shift the address high nibble into its proper location, then
	     * convert the address low nibble from characters to a hexadecimal
	     * value.
	     */
	    address <<= 4;
	    if ((test->steps[ii][2] >= '0') && (test->steps[ii][2] <= '9'))
	    {
		address += (test->steps[ii][2] - '0');
	    }
	    else
	    {
		address += ((test->steps[ii][2] - 'a') + 10);
	    }

	    /*
	     * Convert the Save Value high nibble from characters to a
	     * hexadecimal value.
	     */
	    if ((test->steps[ii][3] >= '0') && (test->steps[ii][3] <= '9'))
	    {
		saveVal = test->steps[ii][3] - '0';
	    }
	    else
	    {
		saveVal = (test->steps[ii][3] - 'a') + 10;
	    }

	    /*
	     * Shift the Save Value high nibble into its proper location,
	     * then convert the Save Value low nibble from characters to a
	     * hexadecimal value.
	     */
	    saveVal <<= 4;
	    if ((test->steps[ii][4] >= '0') && (test->steps[ii][4] <= '9'))
	    {
		saveVal += (test->steps[ii][4] - '0');
	    }
	    else
	    {
		saveVal += ((test->steps[ii][4] - 'a') + 10);
	    }
	}

	printf("\tStep %d: Address: 0x%02x; Value: 0x%02x\n", ii, address, saveVal);

	/*
	 * Based on the command, call the correct interface function.
	 */
	switch (test->steps[ii][0])
	{
	    case 'W':

		/*
		 * If we are writing to Register B and are setting or clearing
		 * the SET bit, then we are also enabling or disabling the
		 * interrupt processing in the DS12887A chip.
		 */
		if (address == 0x0b)
		{
		    if ((saveVal & 0x80) == 0x80)
			interruptsEnabled = false;
		    else
			interruptsEnabled = true;
		}
		AXP_DS12887A_Write(address, saveVal);

		/*
		 * The way write works, there is no way to know if it did
		 * what it was told.  So, there is nothing to check for
		 * success or failure.
		 */
		break;

	    case 'R':
		AXP_DS12887A_Read(address, &readVal);

		/*
		 * If the value read from the interface is the same as we
		 * expected, then this test was successful.
		 */
		retVal = readVal == saveVal;
		if (retVal == false)
		    printf(
			"\tAddress: 0x%02x; Expected: 0x%02x; Got: 0x%02x\n",
			address,
			saveVal,
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
		sleep(saveVal);

		/*
		 * This is not actually testing the TOY clock interface, but is
		 * a way to allow the TOY clock to do some of its processing.
		 * So, there is nothing to check for success or failure.
		 */
		break;

	}
    }

    printf(
	"Test %d: %s %s.\n",
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
		"W0b80",
		"R0b80",
		"D0000"
	    }
        },
        {
	    "Write/Read DM/DSE bits to 010/0110 in Register B",
	    {
		"W0b86", /* Leave SET bit to keep interrupt processing off */
		"R0b86",
		"D0000"
	    }
        },
        {
	    "Write/Read DV/RS bits in Register A",
	    {
		"W0a26", /* Period rate: 976.5625 microseconds */
		"R0a26",
		"D0000"
	    }
        },
        {
	    "Write/Read Alarm Seconds bits to don't care minutes and hours remain "
	    "0 to disable alarm interrupt",
	    {
		"W01c0",
		"R01c0",
		"D0000"
	    }
        },
        {
	    "Write/Read Time to 9:00:00am",
	    {
		"W0000",
		"W0200",
		"W0409",
		"R0200",
		"R0409",
		"R0000",
		"D0000"
	    }
        },
        {
	    "Write/Read Date to June 16, 1987",
	    {
		"W0710",
		"W0806",
		"W0957",
		"R0957",
		"R0806",
		"R0710",
		"D0000"
	    }
        },
        {
	    "Write/Read to various RAM locations",
	    {
		"W50F0",
		"W7086",
		"W307f",
		"R307f",
		"R7086",
		"R50f0",
		"D0000"
	    }
        },
        {
	    "Write/Read SET bit to 0 in Register B, leave DM/DSE as set above",
	    {
		"W0b76",
		"R0b76",
		"D0000"
	    }
        },
	{
	    "Waiting for things to happen",
	    {
		"H000a",
		"D0000"
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
	AXP_DS12887A_Config(&irqCond, &irqMutex, &irqH, IRQMask);

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
