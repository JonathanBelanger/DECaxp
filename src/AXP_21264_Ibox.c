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
 *	This source file contains the functions needed to implement the
 *	functionality of the Ibox.
 *
 *	Revision History:
 *
 *	V01.000		10-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		14-May-2017	Jonathan D. Belanger
 *	Cleaned up some compiler errors.  Ran some tests, and corrected some coding
 *	mistakes.  The prediction code correctly predicts a branch instruction
 *	between 95.0% and 99.1% of the time.
 *
 *	V01.002		20-May-2017
 *	Did a bit of reorganization.  This module will contain all the
 *	functionality that is described as being part of the Ibox.  I move the
 *	prediction code here.
 */
#include "AXP_Blocks.h"
#include "AXP_Base_CPU.h"
#include "AXP_21264_Predictions.h"
#include "AXP_21264_CPU.h"
#include "AXP_Configure.h"

/*
 * The following module specific variable contains a list of instruction types
 * that are used to decode the Alpha AXP instructions.  The opcode is the index
 * into this array.
 */
static AXP_INS_TYPE instructionType[] =
{
/* Format	   Opcode	Mnemonic	Description 							*/
	Pcd,	/* 00		CALL_PAL	Trap to PALcode							*/
	Res,	/* 01					Reserved for Digital					*/
	Res,	/* 02					Reserved for Digital					*/
	Res,	/* 03					Reserved for Digital					*/
	Res,	/* 04					Reserved for Digital					*/
	Res,	/* 05					Reserved for Digital					*/
	Res,	/* 06					Reserved for Digital					*/
	Res,	/* 07					Reserved for Digital					*/
	Mem,	/* 08		LDA			Load address							*/
	Mem,	/* 09		LDAH		Load address high						*/
	Mem,	/* 0A		LDBU		Load zero-extended byte					*/
	Mem,	/* 0B		LDQ_U		Load unaligned quadword					*/
	Mem,	/* 0C		LDWU		Load zero-extended word					*/
	Mem,	/* 0D		STW			Store word								*/
	Mem,	/* 0E		STB			Store byte								*/
	Mem,	/* 0F		STQ_U		Store unaligned quadword				*/
	Opr,	/* 10		ADDL		Add longword							*/
	Opr,	/* 11		AND			Logical product							*/
	Opr,	/* 12		MSKBL		Mask byte low							*/
	Opr,	/* 13		MULL		Multiply longword						*/
	FP,		/* 14		ITOFS		Integer to floating move, S_floating	*/
	FP,		/* 15		ADDF		Add F_floating							*/
	FP,		/* 16		ADDS		Add S_floating							*/
	FP,		/* 17		CVTLQ		Convert longword to quadword			*/
	Mfc,	/* 18		TRAPB		Trap barrier							*/
	PAL,	/* 19		HW_MFPR		Reserved for PALcode					*/
	Mbr,	/* 1A		JMP			Jump									*/
	PAL,	/* 1B		HW_LD		Reserved for PALcode					*/
	Cond,	/* 1C		SEXTB		Sign extend byte						*/
	PAL,	/* 1D		HW_MTPR		Reserved for PALcode					*/
	PAL,	/* 1E		HW_REI		Reserved for PALcode					*/
	PAL,	/* 1F		HW_ST		Reserved for PALcode					*/
	Mem,	/* 20 		LDF			Load F_floating							*/
	Mem,	/* 21		LDG			Load G_floating							*/
	Mem,	/* 22		LDS			Load S_floating							*/
	Mem,	/* 23		LDT			Load T_floating							*/
	Mem,	/* 24		STF			Store F_floating						*/
	Mem,	/* 25		STG			Store G_floating						*/
	Mem,	/* 26		STS			Store S_floating						*/
	Mem,	/* 27		STT			Store T_floating						*/
	Mem,	/* 28		LDL			Load sign-extended longword				*/
	Mem,	/* 29		LDQ			Load quadword							*/
	Mem,	/* 2A		LDL_L		Load sign-extended longword locked		*/
	Mem,	/* 2B		LDQ_L		Load quadword locked					*/
	Mem,	/* 2C		STL			Store longword							*/
	Mem,	/* 2D		STQ			Store quadword							*/
	Mem,	/* 2E		STL_C		Store longword conditional				*/
	Mem,	/* 2F		STQ_C		Store quadword conditional				*/
	Bra,	/* 30		BR			Unconditional branch					*/
	Bra,	/* 31		FBEQ		Floating branch if = zero				*/
	Bra,	/* 32		FBLT		Floating branch if < zero				*/
	Bra,	/* 33		FBLE		Floating branch if <= zero				*/
	Mbr,	/* 34		BSR			Branch to subroutine					*/
	Bra,	/* 35		FBNE		Floating branch if != zero				*/
	Bra,	/* 36		FBGE		Floating branch if >=zero				*/
	Bra,	/* 37		FBGT		Floating branch if > zero				*/
	Bra,	/* 38		BLBC		Branch if low bit clear					*/
	Bra,	/* 39		BEQ			Branch if = zero						*/
	Bra,	/* 3A		BLT			Branch if < zero						*/
	Bra,	/* 3B		BLE			Branch if <= zero						*/
	Bra,	/* 3C		BLBS		Branch if low bit set					*/
	Bra,	/* 3D		BNE			Branch if != zero						*/
	Bra,	/* 3E		BGE			Branch if >= zero						*/
	Bra		/* 3F		BGT			Branch if > zero						*/
};

/*
 * AXP_Branch_Prediction
 *
 *	This function is called to determine if a branch should be taken or not.
 *	It uses past history, locally and globally, to determine this.
 *
 *	The Local History Table is indexed by bits 2-11 of the VPC.  This entry
 *	contains a 10-bit value (0-1023), which is generated by indicating when
 *	a branch is taken(1) versus not taken(0).  This value is used as an index
 *	into a Local Predictor Table.  This table contains a 3-bit saturation
 *	counter, which is incremented when a branch is actually taken and
 *	decremented when a branch is not taken.
 *
 *	The Global History Path, which is generated by the set of taken(1)/not
 *	taken(0) branches.  This is used as an index into a Global Predictor Table,
 *	which contains a 2-bit saturation counter.
 *
 *	The Global History Path is also used as an index into the Choice Predictor
 *	Table.  This table contains a 2-bit saturation counter that is incremented
 *	when the Global Predictor is correct, and decremented when the Local
 *	Predictor is correct.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	vpc:
 *		A 64-bit value of the Virtual Program Counter.
 *
 * Output Parameters:
 *	localTaken:
 *		A location to receive a value of true when the local predictor
 *		indicates that a branch should be taken.
 *	globalTaken:
 *		A location to receive a value of true when the global predictor
 *		indicates that a branch should be taken.
 *	choice:
 *		A location to receive a value of true when the global predictor should
 *		be selected, and false when the local predictor should be selected.
 *		This parameter is only used when the localPredictor and GlobalPredictor
 *		do not match.
 *
 * Return Value:
 *	None.
 */
bool AXP_Branch_Prediction(
				AXP_21264_CPU *cpu,
				AXP_PC vpc,
				bool *localTaken,
				bool *globalTaken,
				bool *choice)
{
	LPTIndex	lpt_index;
	int			lcl_history_idx;
	int			lcl_predictor_idx;
	bool		retVal;

	/*
	 * Need to extract the index into the Local History Table from the VPC, and
	 * use this to determine the index into the Local Predictor Table.
	 */
	lpt_index.vpc = vpc;
	lcl_history_idx = lpt_index.index.index;
	lcl_predictor_idx = cpu->localHistoryTable.lcl_history[lcl_history_idx];

	/*
	 * Return the take(true)/don't take(false) for each of the Predictor
	 * Tables.  The choice is determined and returned, but my not be used by
	 * the caller.
	 */
	*localTaken = AXP_3BIT_TAKE(cpu->localPredictor.lcl_pred[lcl_predictor_idx]);
	*globalTaken = AXP_2BIT_TAKE(cpu->globalPredictor.gbl_pred[cpu->globalPathHistory]);
	*choice = AXP_2BIT_TAKE(cpu->choicePredictor.choice_pred[cpu->globalPathHistory]);
	if (*localTaken != *globalTaken)
		retVal = (*choice == true) ? *globalTaken : *localTaken;
	else
		retVal = *localTaken;

	return(retVal);
}

/*
 * AXP_Branch_Direction
 *	This function is called when the branch instruction is retired to update the
 *	local, global, and choice prediction tables, and the local history table and
 *	global path history information.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	vpc:
 *		A 64-bit value of the Virtual Program Counter.
 *	localTaken:
 *		A value of what was predicted by the local predictor.
 *	globalTaken:
 *		A value of what was predicted by the global predictor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_Branch_Direction(
				AXP_21264_CPU *cpu,
				AXP_PC vpc,
				bool taken,
				bool localTaken,
				bool globalTaken)
{
	LPTIndex	lpt_index;
	int			lcl_history_idx;
	int			lcl_predictor_idx;

	/*
	 * Need to extract the index into the Local History Table from the VPC, and
	 * use this to determine the index into the Local Predictor Table.
	 */
	lpt_index.vpc = vpc;
	lcl_history_idx = lpt_index.index.index;
	lcl_predictor_idx = cpu->localHistoryTable.lcl_history[lcl_history_idx];

	/*
	 * If the choice to take or not take a branch agreed with the local
	 * predictor, then indicate this for the choice predictor, by decrementing
	 * the saturation counter
	 */
	if ((taken == localTaken) && (taken != globalTaken))
	{
		AXP_2BIT_DECR(cpu->choicePredictor.choice_pred[cpu->globalPathHistory]);
	}

	/*
	 * Otherwise, if the choice to take or not take a branch agreed with the
	 * global predictor, then indicate this for the choice predictor, by
	 * incrementing the saturation counter
	 *
	 * NOTE:	If the branch taken does not match both the local and global
	 *			predictions, then we don't update the choice at all (we had a
	 *			mis-prediction).
	 */
	else if ((taken != localTaken) && (taken == globalTaken))
	{
		AXP_2BIT_INCR(cpu->choicePredictor.choice_pred[cpu->globalPathHistory]);
	}

	/*
	 * If the branch was taken, then indicate this in the local and global
	 * prediction tables.  Additionally, indicate that the local and global
	 * paths were taken, when they agree.  Otherwise, decrement the appropriate
	 * predictions tables and indicate the local and global paths were not
	 * taken.
	 *
	 * NOTE:	If the local and global predictors indicated that the branch
	 *			should be taken, then both predictor are correct and should be
	 *			accounted for.
	 */
	if (taken == true)
	{
		AXP_3BIT_INCR(cpu->localPredictor.lcl_pred[lcl_predictor_idx]);
		AXP_2BIT_INCR(cpu->globalPredictor.gbl_pred[cpu->globalPathHistory]);
		AXP_LOCAL_PATH_TAKEN(cpu->localHistoryTable.lcl_history[lcl_history_idx]);
		AXP_GLOBAL_PATH_TAKEN(cpu->globalPathHistory);
	}
	else
	{
		AXP_3BIT_DECR(cpu->localPredictor.lcl_pred[lcl_predictor_idx]);
		AXP_2BIT_DECR(cpu->globalPredictor.gbl_pred[cpu->globalPathHistory]);
		AXP_LOCAL_PATH_NOT_TAKEN(cpu->localHistoryTable.lcl_history[lcl_history_idx]);
		AXP_GLOBAL_PATH_NOT_TAKEN(cpu->globalPathHistory);
	}
	return;
}

/*
 * Check
 */

#if _TEST_PREDICTION_

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
	FILE		*fp;
	char		*fileList[] =
				{
					"trace1.txt",
					"trace2.txt",
					"trace3.txt",
					"trace4.txt",
					"trace5.txt"
				};
	int			lineCnt[] =
				{
					2213673,
					1792835,
					1546797,
					895842,
					2422049
				};
	bool			taken;
	int				takenInt;
	int				ii, jj;
	AXP_21264_CPU	*cpu;
	AXP_PC			vpc;
	int				vpcInt;
	bool			localTaken;
	bool			globalTaken;
	bool			choice;
	bool			prediction;
	int				insCnt;
	int				predictedCnt;
	int				localCnt;
	int				globalCnt;
	int				choiceUsed;
	int				choiceCorrect;

	/*
	 * NOTE: 	The current simulation takes in one instruction at a time.  The
	 *			AXP simulator will process four instructions at a time and
	 *			potentially out of order.  When a branch instruction is retired,
	 *			only then is the
	 */
	printf("\nAXP 21264 Predictions Unit Tester\n");
	printf("%d trace files to be processed\n\n", (int) (sizeof(fileList)/sizeof(char *)));
	cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);
	for (ii = 0; ii < (sizeof(fileList)/sizeof(char *)); ii++)
	{
		fp = fopen(fileList[ii], "r");
		if (fp != NULL)
		{
			insCnt = 0;
			predictedCnt = 0;
			localCnt = 0;
			globalCnt = 0;
			choiceUsed = 0;
			choiceCorrect = 0;
			printf("\nProcessing trace file: %s (%d)...\n", fileList[ii], lineCnt[ii]);
			while (feof(fp) == 0)
			{
				fscanf(fp, "%d %d\n", &vpcInt, &takenInt);
				insCnt++;
				vpc.pc = vpcInt;
				taken = (takenInt == 1) ? true : false;

				/*
				 * Predict whether the branch should be taken or not.  We'll get
				 * results from the Local and Global Predictor, and the Choice
				 * selected (when the Local and Global do not agree).
			 	 */
				prediction = AXP_Branch_Prediction(cpu, vpc, &localTaken, &globalTaken, &choice);
				if (prediction == taken)
					predictedCnt++;

				/*
				 * Let's determine how the choice was determined.
				 */
				if (localTaken != globalTaken)
				{
					choiceUsed++;
					if (choice == true)
					{
						if (taken == globalTaken)
						{
							globalCnt++;
							choiceCorrect++;
						}
					}
					else
					{
						if (taken == localTaken)
						{
							localCnt++;
							choiceCorrect++;
						}
					}
				}
				else
				{
					if (taken == localTaken)
					{
						localCnt++;
						globalCnt++;
					}
				}

				/*
				 * Update the predictors based on whether the branch was actually
				 * taken or not and considering which of the predictors was
				 * correct.
				 *
				 * NOTE: 	Whether choice was used or not is irrelevant.  The
			 	 *			choice is determined by whether the local or global
			 	 *			were correct.  If both are correct or both are
			 	 *			incorrect, then the choice was not used and thus
			 	 *			would not have made a difference.
			 	 */
				AXP_Branch_Direction(cpu, vpc, taken, localTaken, globalTaken);
			}
			fclose(fp);

			/*
			 * Print out what we found.
			 */
			printf("---------------------------------------------\n");
			printf("Total Instructions:\t\t\t%d\n", insCnt);
			printf("Correct predictions:\t\t\t%d\n", predictedCnt);
			printf("Mispredictions:\t\t\t\t%d\n", (insCnt - predictedCnt));
			printf("Prediction accuracy:\t\t\t%1.6f\n\n", ((float) predictedCnt / (float) insCnt));
			printf("Times Local Correct:\t\t\t%d\n", localCnt);
			printf("Times Global Correct:\t\t\t%d\n", globalCnt);
			printf("Times Choice Used:\t\t\t%d\n", choiceUsed);
			printf("Times Choice Selected Correctly:\t%d\n", choiceCorrect);
			printf("Times Choice was wrong:\t\t\t%d\n", (choiceUsed - choiceCorrect));

			/*
			 * We need to clear out the prediction tables in the CPU record.
			 */
			cpu->globalPathHistory = 0;
			for (jj = 0; jj < ONE_K; jj++)
				cpu->localHistoryTable.lcl_history[jj] = 0;
			for (jj = 0; jj < ONE_K; jj++)
				cpu->localPredictor.lcl_pred[jj] = 0;
			for (jj = 0; jj < FOUR_K; jj++)
				cpu->globalPredictor.gbl_pred[jj] = 0;
			for (jj = 0; jj < FOUR_K; jj++)
				cpu->choicePredictor.choice_pred[jj] = 0;
		}
		else
		{
			printf("Unable to open trace file: %s\n", fileList[ii]);
		}
	}
	AXP_Deallocate_Block((AXP_BLOCK_DSC *) cpu);
	return(0);
}
#endif
