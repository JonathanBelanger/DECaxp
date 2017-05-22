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
 *	This source file contains the main function to test the branch prediction
 *	code.
 *
 * Revision History:
 *
 *	V01.000		22-May-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"

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