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
 *	This source file contains the main function to test the integer load code.
 *
 * Revision History:
 *
 *	V01.000		26-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Blocks.h"
#include "AXP_21264_Instructions.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_Ebox_LoadStore.h"
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
	FILE			*fp;
	char			*fileName = "LDA_LDAH_TestData.csv";
	char			*inputFmt = "0x%016xll,0x%x016xll,0x%04x,0x016xll\n";
	bool			pass = true;
	u64				ldaAddress, ldahAddress;
	AXP_21264_CPU	*cpu;
	AXP_INSTRUCTION	instr;
	AXP_EXCEPTIONS	retVal;
	int				testCnt;

	/*
	 * NOTE: 	The current simulation takes in one instruction at a time.  The
	 *			AXP simulator will process four instructions at a time and
	 *			potentially out of order.
	 */
	printf("\nAXP 21264 Integer Load/Store Tester\n");
	cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);
	instr.uniqueID = 0;		// This will be incremented for each test (and should wrap).
	instr.aSrc1 = 5;		// Architectural Register (R05).
	instr.src1 = 40;		// Physical Register.
	instr.aSrc2 = 31;		// Architectural Register (R31).
	instr.src2 = 72;		// Physical Register.
	instr.aDest = 29;		// Architectural Register (R29).
	instr.dest = 31;		// Physical Register.
	instr.type_hint_index = 0;
	instr.scbdMask = 0;
	instr.len_stall = 0;
	instr.lockFlagPending = false;
	instr.clearLockPending = false;
	instr.useLiteral = false;
	instr.branchPredict = false;
	instr.literal = 0;
	instr.src2v.r.uq = 0;	// Only 2 registers (Ra = destination, Rb = source)
	instr.lockPhysAddrPending = 0;
	instr.lockVirtAddrPending = 0;
	instr.format = Mem;		// Memory formatted instruction.
	instr.type = Load;		// Load operation.
	instr.pc.pc = 0x000000007ffe000ll;
	instr.pc.pal = 0;		// not PALmode.
	instr.branchPC.pc = 0;
	instr.branchPC.pal = 0;
	instr.state = Retired;	// All instructions are initially Retired.

	fp = fopen(fileName, "r");
	if(fp != NULL)
	{
		printf("\n Processing Test Data File: %s\n", fileName);
		fscanf(fp, inputFmt, &ldahAddress, &ldaAddress, &instr.displacement, &instr.src1v.r.uq);
		while ((feof(fp) == 0) && (pass == true))
		{

			/*
			 * Sign extend the displacement.
			 */
			instr.displacement |=
				(instr->displacement & 0x0000000000008000ll ? 0xffffffffffff0000 : 0);
			instr.state = Executing;
			testCnt++;
			retVal = AXP_LDA(cpu, &instr);		// Call the LDA - Load Address instruction.
			if (retVal == NoException)
			{
				if (instr.state == WaitingRetirement)
				{
					if (instr.destv.r.uq != ldaAddress)
					{
						printf("LDA failed: Rbv = 0x%016xll, disp = 0x%04x, Rav = 0x%016xll (expected: 0x%016xll)\n",
							instr.src1v.r.uq, instr.displacement, instr.destv.r.uq, ldaAddress);
						pass = false;
					}
					else
					{
						instr.pc.pc++;
						instr.uniqueID++;
					}
				}
				else
				{
					printf("LDA failed in instruction state!\n");
					pass = false;
				}
			}
			else
			{
				printf("LDA failed in instruction return value!\n");
				pass = false;
			}
			if (pass == true)
			{
				instr.destv.r.uq = 0;
				instr.state = Executing;
				testCnt++;
				retVal = AXP_LDAH(cpu, &instr);		// Call the LDAH - Load Address High instruction.
				if (retVal == NoException)
				{
					if (instr.state == WaitingRetirement)
					{
						if (instr.destv.r.uq != ldaAddress)
						{
							printf("LDAH failed: Rbv = 0x%016llx, disp = 0x%04llx, Rav = 0x%016llx (expected: 0x%016llx)\n",
								instr.src1v.r.uq, instr.displacement, instr.destv.r.uq, ldaAddress);
							pass = false;
						}
					else
					{
						instr.pc.pc++;
						instr.uniqueID++;
					}
					}
					else
					{
						printf("LDAH failed in instruction state!\n");
						pass = false;
					}
				}
				else
				{
					printf("LDAH failed in instruction return value!\n");
					pass = false;
				}
			}
			fscanf(fp, inputFmt, &ldahAddress, &ldaAddress, &instr.displacement, &instr.src1v.r.uq);
		}	// while (!EOF && passing)
		fclose(fp);
	}
	else
	{
		printf("Unable to open test data file: %s\n", fileName);
		pass = false;
	}

	/*
	 * Display the results of the test.
	 */
	 if (pass == true)
		 printf("Test passed.  %d test cases executed.\n", testCnt);
	 else
		 printf("\nTest failed (%d passed before failing).  See preceding message.\n", testCnt);
	
	/*
	 * We are done.
	 */
	return(0);
}
