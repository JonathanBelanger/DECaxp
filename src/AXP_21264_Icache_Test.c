/*
 * Copyright (C) Jonathan D. Belanger 2017.
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
 *	This source file contains the main function to test the instruction cache
 *	(Icache) code.
 *
 * Revision History:
 *
 *	V01.000		31-May-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_Ibox.h"

static AXP_INS_FMT memory[] =
{
	0x4be0173f,		/* Address Offset: 0x0000000000000000 */
	0x43ff0401,
	0x43ff0521,
	0x47ff0001,
	0x47ff0401,
	0x47ff0801,
	0x47ff0501,
	0x43ff09a1,
	0x4be03721,
	0x4be05681,
	0x4be07781,
	0x43ff0401,
	0x43ff0521,
	0x47ff0001,
	0x47ff0401,
	0x47ff0801,
	0x47ff0501,		/* Address Offset: 0x0000000000000040 */
	0x43ff09a1,
	0x4be03721,
	0x4be05681,
	0x4be07781,
	0x43e01401,
	0xa03f0004,
	0x303f0004,
	0x303f0004,
	0x283f0004,
	0x283f0004,
	0x203f0000,
	0x47e01001,
	0x47e01401,
	0x47e01501,
	0x43e019a1,
	0x47e83408,		/* Address Offset: 0x0000000000000080 */
	0x49021728,
	0x45083408,
	0xb11f0000,
	0xa13f0000,
	0x47e5d408,
	0x351f0000,
	0xa13f0000,
	0x45083408,
	0x391f0000,
	0xa13f0000,
	0x47e45408,
	0x49021728,
	0x45045408,
	0xb11f0000,
	0xa13f0000,
	0x313f0004,		/* Address Offset: 0x00000000000000c0 */
	0x313f0004,
	0x293f0004,
	0x293f0004,
	0x43e15404,
	0xc3e00066,		/* Branch: always taken */
	0x43e15410,
	0xa2100000,
	0x4a039731,
	0x43e03417,
	0x43ff0401,
	0x42e3b9b6,
	0x42df05b6,
	0xf6c00004,		/* Branch: 28 not taken, 1 taken */
	0x4a203792,
	0x4a203691,
	0x47f10511,		/* Address Offset: 0x0000000000000100 */
	0x42e01417,
	0xc3fffff7,		/* Branch: always taken */
	0x203f0001,
	0x4823f721,
	0x43e01404,
	0x43e05401,
	0x43e33402,
	0x43e03403,
	0x40230403,
	0x43e6f408,
	0x48205727,
	0xb1070000,
	0x41280409,
	0x40230401,
	0x47ff041f,
	0x404105a2,		/* Address Offset: 0x0000000000000140 */
	0x47ff041f,
	0xf4400001,		/* Branch: 252 not taken, 1 taken */
	0x47ff041f,
	0xc3fffff6,		/* Branch: always taken */
	0x203f0001,
	0x4823f721,
	0x43e01404,
	0x43e05401,
	0x43e33402,
	0x43e03403,
	0x40230401,
	0x48205727,
	0x40e60407,
	0xa1070000,
	0x41280409,
	0x40230401,		/* Address Offset: 0x0000000000000180 */
	0x404105a2,
	0xf4400000,		/* Branch: 252 not taken, 1 taken */
	0x47ff041f,
	0xc3fffff7,		/* Branch: always taken */
	0x49209689,
	0x43e2340b,
	0x412b052a,
	0xa18a0000,
	0x43e01404,
	0x43e05401,
	0x43e67402,
	0x43e03403,
	0x40230401,
	0x48203727,
	0x40e60407,
	0x31070000,		/* Address Offset: 0x00000000000001c0 */
	0x41280409,
	0x40230401,
	0x47ff041f,
	0x404105a2,
	0x47ff041f,
	0xf4400001,		/* Branch: 508 not taken, 1 taken */
	0x47ff041f,
	0xc3fffff5,		/* Branch: always taken */
	0x49209689,
	0x43e9d40b,
	0x412b052a,
	0xa18a0000,
	0x43e01404,
	0x43e05401,
	0x43e15402,
	0x43e03403,		/* Address Offset: 0x0000000000000200 */
	0x40230401,
	0x48201727,
	0x40e60407,
	0x29070000,
	0x41280409,
	0x40230401,
	0x47ff041f,
	0x404105a2,
	0x47ff041f,
	0xf4400001,		/* Branch: 1020 not taken, 1 taken */
	0x47ff041f,
	0xc3fffff5,		/* Branch: always taken */
	0x49209689,
	0x43e1740b,
	0x412b052a,
	0xa18a001a,		/* Address Offset: 0x0000000000000240 */
	0x47ff041f,
	0x47ff041f,
	0x47ff041f,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x43ff0401,
	0x43bf141d,
	0xb01d0004,
	0xb09d0000,
	0x408039a2,		/* Address Offset: 0x0000000000000280 */
	0xe4400005,		/* Branch: 10 taken, 1 not tkane */
	0xa0840000,
	0xa01d0004,
	0x43a1141d,
	0x47e4040e,
	0x6be00000,
	0x47e4040e,
	0x47e40510,
	0x409ff404,
	0x47e40411,
	0x42110412,
	0x42110533,
	0x46110014,
	0x46310415,
	0x46110516,
	0x46110017,		/* Address Offset: 0x00000000000002c0 */
	0x404105a2,
	0xc3ffffe9,		/* Branch: always taken */
	0xa09d0000,		/* This line and the remaining never get referenced */
	0xa01d0000,
	0x43a1141d,
	0x47e4040e,
	0x47ff041f		/* Address Offset: 0x00000000000002dc */
};

#define AXP_NUMBER_OR_BRANCHES 13

/*
 * main
 *	This function is compiled in when unit testing.  It exercises the branch
 *	prediction code, and should be somewhat extensive.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
int main()
{
	bool done = false;
	int hitCnt, cacheMissCnt, ITBMissCnt, cycleCnt, instrCnt;
	int jj;
	u64 ii;
	AXP_MEMORY_PROTECTION prot;
	struct
	{
		u64 address;
		i32 taken;
		u64 destination;
		bool defaultTake;
	} brCntArr[AXP_NUMBER_OR_BRANCHES] =
		{
			{0x00000000000000d4UL,    0, 0x0000000000000270UL, false},
			{0x00000000000000f4UL,   28, 0x0000000000000108UL, false},
			{0x0000000000000108UL,    0, 0x00000000000000e8UL, false},
			{0x0000000000000148UL,  252, 0x0000000000000150UL, false},
			{0x0000000000000150UL,    0, 0x000000000000012cUL, false},
			{0x0000000000000188UL,  252, 0x0000000000000190UL, false},
			{0x0000000000000190UL,    0, 0x0000000000000170UL, false},
			{0x00000000000001d8UL,  508, 0x00000000000001e0UL, false},
			{0x00000000000001e0UL,    0, 0x00000000000001b8UL, false},
			{0x0000000000000228UL, 1020, 0x0000000000000230UL, false},
			{0x0000000000000230UL,    0, 0x0000000000000208UL, false},
			{0x0000000000000284UL,   10, 0x000000000000029cUL, true},
			{0x00000000000002c8UL,    0, 0x0000000000000270UL, false}
		};
	union
	{
		u64 	pc;
		AXP_PC	vpc;
	} pc = { .pc = 0x0000000000000004UL };
	u64 fetchedPC;
	AXP_21264_CPU *cpu;
	AXP_CACHE_FETCH retStatus;
	AXP_IBOX_INS_LINE nextLine;

	printf("\nAXP 21264 I-Cache Unit Tester\n");
	cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);
	hitCnt = 0;
	cacheMissCnt = 0;
	ITBMissCnt = 0;
	cycleCnt = 0;
	instrCnt = 0;
	prot.kre = 1;
	prot.ere = 1;
	prot.sre = 1;
	prot.ure = 1;
	while (!done)
	{
		fetchedPC = pc.pc;

		/*
		 * Attempt to get the next set of instructions for the Icache.
		 */
		retStatus = AXP_ICacheFetch(cpu, pc.vpc, &nextLine);
		switch (retStatus)
		{

			/*
			 * The instruction line we were looking for was found in the
			 * Icache.  Increment the hit counter, then "simulate" the CPU
			 * processing.
			 */
			case Hit:
				hitCnt++;
				for (ii = 0; ((ii < AXP_IBOX_INS_FETCHED) && !done); ii++)
				{

					/*
					 * If the next instruction does not have an opcode of 0,
					 * then we have something to process.  Otherwise, we are
					 * done processing.
					 */
					if (nextLine.instructions[ii].pal.opcode != 0x00)
					{
						instrCnt++;
						pc.vpc.pc++;	/* increment the pc */

						/*
						 * If the next instruction to process is a branch, then
						 * we may need to jump to a different destination.
						 */
						if ((nextLine.instrType[ii] == Bra) ||
							(nextLine.instrType[ii] == Mbr))
						{

							/*
							 * Look through the list of branch instructions in
							 * the array of branch addresses and determine if
							 * we are to branch to a new address or not.
							 */
							for (jj = 0; jj < AXP_NUMBER_OR_BRANCHES; jj++)
								if (brCntArr[jj].address == (pc.vpc.pc + ii - 1))
								{

									/*
									 * If the taken cound is zero, then if the
									 * default action is to not have taken the
									 * branch, then do so now.
									 */
									if (brCntArr[jj].taken == 0)
									{
										if (brCntArr[jj].defaultTake == false)
											pc.pc = brCntArr[jj].destination;
									}
									
									/*
									 * Otherwise, decrement the taken counter
									 * and if the default is to take the
									 * branch, then do so now.
									 */
									else
									{
										brCntArr[jj].taken--;
										if (brCntArr[jj].defaultTake == true)
											pc.pc = brCntArr[jj].destination;
									}
									break;		/* We're done here. */
								}
						}
					}
					else
					{
						done = true;
						continue;
					}

					/*
					 * If the next instruction to "process" is not contained
					 * within the PC addresses of the fetched set of
					 * instructions, then we need to get out of the current
					 * for loop.
					 */
					if ((pc.pc < fetchedPC) &&
						(pc.pc >= (fetchedPC + AXP_IBOX_INS_FETCHED)))
						break;
				}
				break;

			/*
			 * The instructions were not in the cache, so we need to move the
			 * next set of instructions into the iCache and then replay the
			 * instruction.
			 */
			case Miss:
				cacheMissCnt++;
				AXP_ICacheAdd(cpu, pc.vpc, &memory[pc.pc], prot); // TODO: physical --> virtual
				break;

			/*
			 * The instructions were not in the Instruction Translation Buffer,
			 * so we have to add an ITB entry and then replay the instruction/
			 */
			case WayMiss:
				ITBMissCnt++;
				//status = AXP_ITBAdd(cpu, pc.vpc);	// Do we want to put the instructions in the cache as well?
				break;
		}
		cycleCnt++;
	}

	/*
	 * Print out the results.
	 */
	printf("\nNumber of cycles:                %d\n", cycleCnt);
	printf("Number of instructions executed: %d\n", instrCnt);
	printf("Number of cache look-ups:        %d\n", (hitCnt+cacheMissCnt+ITBMissCnt));
	printf("    Number of cache hits:        %d\n", hitCnt);
	printf("    Number of cache misses:      %d\n", cacheMissCnt);
	printf("    Number of ITB misses:        %d\n\n", ITBMissCnt);
	printf("    Hit percentage:              %1.2f\n", ((float)hitCnt/(float)(hitCnt+cacheMissCnt+ITBMissCnt)));
	printf("    Miss percentage:             %1.2f\n", ((float)(ITBMissCnt+cacheMissCnt)/(float)(hitCnt+cacheMissCnt+ITBMissCnt)));
	printf("    Way Miss percentage:         %1.2f\n\n", ((float)ITBMissCnt/(float)(hitCnt+cacheMissCnt+ITBMissCnt)));
	printf("Cache look-ups per cycle:        %5.2f\n", ((float)(hitCnt+cacheMissCnt+ITBMissCnt)/(float)cycleCnt));
	printf("Instructions per cycle:          %5.2f\n\n", ((float)instrCnt/(float)cycleCnt));
	return(0);
}
