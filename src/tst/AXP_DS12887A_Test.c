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
#include "AXP_Utility.h";
#include "AXP_DS12887A_TOYClock.h"

/*
 * A step contains a set of characters that contain the following format:
 *
 *	<Command>[<Address><Value>]
 *
 * where:
 *	<Command>:
 *		W = Write - The <Address> is the location to be written and <Value> is
 *			the value to be written to that location.
 *		R = Read - The <Address> is the location to be read from and <Value> is
 *			what is expected to be returned from that address.
 *		S = Reset - The <Address> and <Value> fields are optional/ignored
 *	<Address>:
 *		A 2-character hexidecimal value between 00 and 7f
 *	<Value>
 *		A 2-character hexidecimal value between 00 and ff
 */
typedef u8 TestStep[1+2+2];

/*
 * StepCount is the number of steps to be executed and tested.
 */
typedef struct
{
	int			stepCount;
	char		*testName;
	TestStep	*steps;
} TestSteps;

static bool		interruptsEnabled = false;

bool executeTest(int testNum, TestSteps *test)
{
	bool	retVal = true;
	int		ii;
	u8		address;
	u8		saveVal;
	u8		readVal;

	/*
	 * Loop until we either exhaust the tests in this set or detect a failure.
	 */
	for (ii = 0; ((ii < test->stepCount) && (retVal == true)); ii++)
	{

		/*
		 * If the command is not reset, then the address and value fields need
		 * to be converted from characters to a hexidecimal value.
		 */
		if (test->steps[ii][0] <> 'S')
		{

			/*
			 * Convert the address high nibble from characters to a hexidecimal
			 * value.
			 */
			if ((test->steps[ii][1] >= '0') && (test->steps[ii][1] <= '9')
				address = test->steps[ii][1] - '0';
			else
				address = (test->steps[ii][1] - 'a') + 10;

			/*
			 * Shift the address high nibble into its proper location, then
			 * convert the address low nibble from characters to a hexidecimal
			 * value.
			 */
			address <<= 4;
			if ((test->steps[ii][2] >= '0') && (test->steps[ii][2] <= '9')
				address += (test->steps[ii][2] - '0');
			else
				address += ((test->steps[ii][2] - 'a') + 10);

			/*
			 * Convert the Save Value high nibble from characters to a
			 * hexidecimal value.
			 */
			if ((test->steps[ii][3] >= '0') && (test->steps[ii][3] <= '9')
				saveVal = test->steps[ii][3] - '0';
			else
				saveVal = (test->steps[ii][3] - 'a') + 10;

			/*
			 * Shift the Save Value high nibble into its proper location, then
			 * convert the Save Value low nibble from characters to a
			 * hexidecimal value.
			 */
			saveVal <<= 4;
			if ((test->steps[ii][3] >= '0') && (test->steps[ii][3] <= '9')
				saveVal += (test->steps[ii][3] - '0');
			else
				saveVal += ((test->steps[ii][3] - 'a') + 10);
		}

		/*
		 * Based on the command, call the correct interface function.
		 */
		switch (test->steps[ii][0])
		{
			case 'W':
			
				/*
				 * If we are writting to Reegister B and are setting or
				 * clearing the SET bit, then we are also enabling or disabling
				 * the interrupt processing in the DS12887A chip.
				 */
				if (address == 0x0b)
				{
					if ((saveValue & 0x80) == 0x80)
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
				 * expected, then this test was successfull.
				 */
				retVal = readVal == saveVal;
				break;

			case 'S':
				AXP_DS12887A_Reset();

				/*
				 * The way write works, there is no way to know if it did
				 * what it was told.  So, there is nothing to check for
				 * success or failure.
				 */
				break;

		}
	}

	printf(
		"Test %d: %s %s.\n",
		testNum,
		test->testName,
		(retVal == true ? "Passed" : "Failed");

	/*
	 * Return the results back to the caller.
	 */
	return(retVal);
}

int main()
{
	bool		pass = true;
	int			ii;
	TestSteps	tests[] =
	{
		{
			.stepCount = 2,
			.testName = "Write/Read SET bit to 1 in Register B",
			.steps[] =
			{
				'W0b80',
				'R0b80'
			};
		},
		{
			.stepCount = 2,
			.testName = "Write/Read DM/DSE bits to 010/0110 in Register B",
			.steps[] =
			{
				'W0b86',	/* Leave SET bit to keep interrupt processing off */
				'R0b86'
			};
		},
		{
			.stepCount = 2,
			.testName = "Write/Read DV/RS bits in Register A",
			.steps[] =
			{
				'W0a26',	/* Period rate: 976.5625 microseconds */
				'R0a26'
			};
		},
		{
			.stepCount = 2,
			.testName = "Write/Read Alarm Seconds bits to don't care "
						" minutes and hours remain 0 to disable alarm "
						"interrupt",
			.steps[] =
			{
				'W01c0',
				'R01c0'
			};
		},
		{
			.stepCount = 6,
			.testName = "Write/Read Time to 9:00:00am",
			.steps[] =
			{
				'W0000',
				'W0200',
				'W0409',
				'R0200',
				'R0409',
				'R0000'
			};
		},
		{
			.stepCount = 6,
			.testName = "Write/Read Date to June 16, 1987",
			.steps[] =
			{
				'W0710',
				'W0806',
				'W0957',
				'R0957',
				'R0806',
				'R0710'
			};
		},
		{
			.stepCount = 6,
			.testName = "Write/Read to various RAM locations",
			.steps[] =
			{
				'W50F0',
				'W7086',
				'W307f',
				'R307f',
				'R7086',
				'R50f0'
			};
		},
		{
			.stepCount = 2,
			.testName = "Write/Read SET bit to 0 in Register B, "
						"leave DM/DSE as set above",
			.steps[] =
			{
				'W0b06',
				'R0b06'
			};
		},
		{
			.stepCount = 0,
		}
	};

	/*
	 * Let's turn on tracing first.
	 */
	if (AXP_TraceInit() == true)
	{
		for (ii = 0; ((tests[ii].stepCount != 0) && (pass == true)); ii++)
			pass = executeTest(ii+1, &tests[ii]);
	}

	/*
	 * TODO:	Look at the IRQ logic in the source code.  It should follow the
	 *			following rules:
	 *				1)	If none of PIE, AIE, and UIE are not set, then the IRQ
	 *					bit should not be set.
	 *				2)	If none of the PI, AI, and UI are not set then the IRQ
	 *					bit should not be set.
	 *				3)	Reading Register C clears all the above bits, so also
	 *					clears the IRQ bit.
	 * TODO:	Look at having the IRQ bit actually be a variable provided to
	 *			the interface (probably a variable, mutex, and condition) that
	 *			will be set and signalled when the IRQH bit is set in Register
	 *			C.
	 */

	/*
	 * Return back to the caller.
	 */
	return(0);
}
