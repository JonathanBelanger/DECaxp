/*
 * Copyright (C) Jonathan D. Belanger 2017-2019.
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
 *  This source file contains the main function to test the branch prediction
 *  code.
 *
 * Revision History:
 *
 *  V01.000 22-May-2017 Jonathan D. Belanger
 *  Initially written.
 */
#include "CommonUtilities/AXP_Blocks.h"
#include "CPU/AXP_21264_CPU.h"
#include "CPU/Ibox/AXP_21264_Ibox.h"

#ifndef AXP_TEST_DATA_FILES
#define AXP_TEST_DATA_FILES "."
#endif
#define AXP_MAX_FILENAME_LEN 256

typedef union
{
	struct
	{
		u8 b : 1;
		u8 a : 1;
		u8 res : 6;
	};
	u8 cnt;
} AXP_2BIT_SAT_CNT;
#define AXP_2BIT_SAT_MAX 0x03
#define _AXP_2BIT_INCR(cntr)                                                \
    {                                                                       \
        AXP_2BIT_SAT_CNT tmp = cntr;                                        \
        cntr.a = tmp.a | tmp.b;                                             \
        cntr.b = tmp.a | (!tmp.b);                                          \
    }
#define _AXP_2BIT_DECR(cntr)                                                \
    {                                                                       \
        AXP_2BIT_SAT_CNT tmp = cntr;                                        \
        cntr.a = tmp.a & tmp.b;                                             \
        cntr.b = tmp.a & (!tmp.b);                                          \
    }
typedef union
{
	struct
	{
		u8 c : 1;
		u8 b : 1;
		u8 a : 1;
		u8 res : 5;
	};
	u8 cnt;
} AXP_3BIT_SAT_CNT;
#define AXP_3BIT_SAT_MAX 0x07
#define _AXP_3BIT_INCR(cntr)                                                \
    {                                                                       \
        AXP_3BIT_SAT_CNT tmp = cntr;                                        \
        cntr.a = tmp.a | (tmp.b & tmp.c);                                   \
        cntr.b = (tmp.a & tmp.b & tmp.c) | (tmp.b ^ tmp.c);                 \
        cntr.c = (tmp.a & tmp.b) | (!tmp.c);                                \
    }
#define _AXP_3BIT_DECR(cntr)                                                \
    {                                                                       \
        AXP_3BIT_SAT_CNT tmp = cntr;                                        \
        cntr.a = tmp.a & (tmp.b | tmp.c);                                   \
        cntr.b = (tmp.a & (!(tmp.b ^ tmp.c))) | (tmp.b & tmp.c);            \
        cntr.c = (!tmp.c) & (tmp.a | tmp.b);                                \
    }

/*
 * TestSaturatingCounters
 * 	This function is called to test the performance of using bitwise and
 * 	conditional	calculations for 2 and 3 bit saturating counters.  To get some
 * 	real numbers we will generate a set of random numbers of 0 or 1.  These
 * 	values will be used to iterate through these counters 10,000,000 times.
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
#define AXP_COUNTER_TESTS 15625
void TestSaturatingCounters(void)
{
    u64 origIteratorBits[AXP_COUNTER_TESTS];
    u64 iteratorBits[AXP_COUNTER_TESTS];
    AXP_2BIT_SAT_CNT twoBitSaturatingCounter;
    AXP_3BIT_SAT_CNT threeBitSaturatingCounter;
    int ii, jj;
    bool increment;
    u32 pass = 0;
    u8 pre;
    u8 post;

    printf("\nGenerating %u bits to be used with saturating counters...",
           AXP_COUNTER_TESTS*64);
    for(ii = 0; ii < AXP_COUNTER_TESTS; ii++)
    {
        for (jj = 0; jj < 64; jj++)
        {
            origIteratorBits[ii] <<= 1;
            if (rand() > (RAND_MAX / 2))
            {
                origIteratorBits[ii] |= 1;
            }
        }
    }
    printf("Done!\n");
    printf("Running 2 bit bitwise saturating counter tests...");
    twoBitSaturatingCounter.cnt = 0;
    for(ii = 0; ii < AXP_COUNTER_TESTS; ii++)
    {
        for(jj = 0; jj < 64; jj++)
        {
            increment = (iteratorBits[ii] & 1) != 0;
            pre = twoBitSaturatingCounter.cnt;
            iteratorBits[ii] >>= 1;
            if (increment == true)
            {
                _AXP_2BIT_INCR(twoBitSaturatingCounter);
            }
            else
            {
                _AXP_2BIT_DECR(twoBitSaturatingCounter);
            }
            post = twoBitSaturatingCounter.cnt;
            if (increment == true)
            {
                switch(pre)
                {
                    case 0:
                        if (post != 1)
                        {
                            pass = 0x01;
                        }
                        break;

                    case 1:
                        if (post != 2)
                        {
                            pass = 0x01;
                        }
                        break;

                    case 2:
                        if (post != 3)
                        {
                            pass = 0x01;
                        }
                        break;

                    case 3:
                        if (post != 3)
                        {
                            pass = 0x01;
                        }
                        break;

                    default:
                        pass = 0x01;
                        break;
                }
            }
            else
            {
                switch(pre)
                {
                    case 0:
                        if (post != 0)
                        {
                            pass |= 0x02;
                        }
                        break;

                    case 1:
                        if (post != 0)
                        {
                            pass |= 0x02;
                        }
                        break;

                    case 2:
                        if (post != 1)
                        {
                            pass |= 0x02;
                        }
                        break;

                    case 3:
                        if (post != 2)
                        {
                            pass |= 0x02;
                        }
                        break;

                    default:
                        pass |= 0x02;
                        break;
                }
            }
        }
    }
    if ((pass && 0x03) != 0)
    {
        printf("Failed");
    }
    else
    {
        printf("Passed");
    }
    printf("!\n");
    for(ii = 0; ii < AXP_COUNTER_TESTS; ii++)
    {
        iteratorBits[ii] = origIteratorBits[ii];
    }
    printf("Running 2 bit conditional saturating counter tests...");
    twoBitSaturatingCounter.cnt = 0;
    for(ii = 0; ii < AXP_COUNTER_TESTS; ii++)
    {
        for(jj = 0; jj < 64; jj++)
        {
            increment = (iteratorBits[ii] & 1) != 0;
            pre = twoBitSaturatingCounter.cnt;
            iteratorBits[ii] >>= 1;
            if (increment == true)
            {
                AXP_2BIT_INCR(twoBitSaturatingCounter.cnt);
            }
            else
            {
                AXP_2BIT_DECR(twoBitSaturatingCounter.cnt);
            }
            post = twoBitSaturatingCounter.cnt;
            if (increment == true)
            {
                switch(pre)
                {
                    case 0:
                        if (post != 1)
                        {
                            pass |= 0x04;
                        }
                        break;

                    case 1:
                        if (post != 2)
                        {
                            pass |= 0x04;
                        }
                        break;

                    case 2:
                        if (post != 3)
                        {
                            pass |= 0x04;
                        }
                        break;

                    case 3:
                        if (post != 3)
                        {
                            pass |= 0x04;
                        }
                        break;

                    default:
                        pass |= 0x04;
                        break;
                }
            }
            else
            {
                switch(pre)
                {
                    case 0:
                        if (post != 0)
                        {
                            pass |= 0x08;
                        }
                        break;

                    case 1:
                        if (post != 0)
                        {
                            pass |= 0x08;
                        }
                        break;

                    case 2:
                        if (post != 1)
                        {
                            pass |= 0x08;
                        }
                        break;

                    case 3:
                        if (post != 2)
                        {
                            pass |= 0x08;
                        }
                        break;

                    default:
                        pass |= 0x08;
                        break;
                }
            }
        }
    }
    if ((pass && 0x06) != 0)
    {
        printf("Failed");
    }
    else
    {
        printf("Passed");
    }
    printf("!\n");
    for(ii = 0; ii < AXP_COUNTER_TESTS; ii++)
    {
        iteratorBits[ii] = origIteratorBits[ii];
    }
    printf("Running 3 bit bitwise saturating counter tests...");
    threeBitSaturatingCounter.cnt = 0;
    for(ii = 0; ii < AXP_COUNTER_TESTS; ii++)
    {
        for(jj = 0; jj < 64; jj++)
        {
            increment = (iteratorBits[ii] & 1) != 0;
            pre = threeBitSaturatingCounter.cnt;
            iteratorBits[ii] >>= 1;
            if (increment == true)
            {
                _AXP_3BIT_INCR(threeBitSaturatingCounter);
            }
            else
            {
                _AXP_3BIT_DECR(threeBitSaturatingCounter);
            }
            post = threeBitSaturatingCounter.cnt;
            if (increment == true)
            {
                switch(pre)
                {
                    case 0:
                        if (post != 1)
                        {
                            pass |= 0x10;
                        }
                        break;

                    case 1:
                        if (post != 2)
                        {
                            pass |= 0x10;
                        }
                        break;

                    case 2:
                        if (post != 3)
                        {
                            pass |= 0x10;
                        }
                        break;

                    case 3:
                        if (post != 4)
                        {
                            pass |= 0x10;
                        }
                        break;

                    case 4:
                        if (post != 5)
                        {
                            pass |= 0x10;
                        }
                        break;

                    case 5:
                        if (post != 6)
                        {
                            pass |= 0x10;
                        }
                        break;

                    case 6:
                        if (post != 7)
                        {
                            pass |= 0x10;
                        }
                        break;

                    case 7:
                        if (post != 7)
                        {
                            pass |= 0x10;
                        }
                        break;

                    default:
                        pass |= 0x10;
                        break;
                }
            }
            else
            {
                switch(pre)
                {
                    case 0:
                        if (post != 0)
                        {
                            pass |= 0x20;
                        }
                        break;

                    case 1:
                        if (post != 0)
                        {
                            pass |= 0x20;
                        }
                        break;

                    case 2:
                        if (post != 1)
                        {
                            pass |= 0x20;
                        }
                        break;

                    case 3:
                        if (post != 2)
                        {
                            pass |= 0x20;
                        }
                        break;

                    case 4:
                        if (post != 3)
                        {
                            pass |= 0x20;
                        }
                        break;

                    case 5:
                        if (post != 4)
                        {
                            pass |= 0x20;
                        }
                        break;

                    case 6:
                        if (post != 5)
                        {
                            pass |= 0x20;
                        }
                        break;

                    case 7:
                        if (post != 6)
                        {
                            pass |= 0x20;
                        }
                        break;

                    default:
                        pass |= 0x20;
                        break;
                }
            }
        }
    }
    if ((pass && 0x30) != 0)
    {
        printf("Failed");
    }
    else
    {
        printf("Passed");
    }
    printf("!\n");
    for(ii = 0; ii < AXP_COUNTER_TESTS; ii++)
    {
        iteratorBits[ii] = origIteratorBits[ii];
    }
    printf("Running 3 bit conditional saturating counter tests...");
    threeBitSaturatingCounter.cnt = 0;
    for(ii = 0; ii < AXP_COUNTER_TESTS; ii++)
    {
        for(jj = 0; jj < 64; jj++)
        {
            increment = (iteratorBits[ii] & 1) != 0;
            pre = twoBitSaturatingCounter.cnt;
            iteratorBits[ii] >>= 1;
            if (increment == true)
            {
                AXP_3BIT_INCR(threeBitSaturatingCounter.cnt);
            }
            else
            {
                AXP_3BIT_DECR(threeBitSaturatingCounter.cnt);
            }
            post = threeBitSaturatingCounter.cnt;
            if (increment == true)
            {
                switch(pre)
                {
                    case 0:
                        if (post != 1)
                        {
                            pass |= 0x40;
                        }
                        break;

                    case 1:
                        if (post != 2)
                        {
                            pass |= 0x40;
                        }
                        break;

                    case 2:
                        if (post != 3)
                        {
                            pass |= 0x40;
                        }
                        break;

                    case 3:
                        if (post != 4)
                        {
                            pass |= 0x40;
                        }
                        break;

                    case 4:
                        if (post != 5)
                        {
                            pass |= 0x40;
                        }
                        break;

                    case 5:
                        if (post != 6)
                        {
                            pass |= 0x40;
                        }
                        break;

                    case 6:
                        if (post != 7)
                        {
                            pass |= 0x40;
                        }
                        break;

                    case 7:
                        if (post != 7)
                        {
                            pass |= 0x40;
                        }
                        break;

                    default:
                        pass |= 0x40;
                        break;
                }
            }
            else
            {
                switch(pre)
                {
                    case 0:
                        if (post != 0)
                        {
                            pass |= 0x80;
                        }
                        break;

                    case 1:
                        if (post != 0)
                        {
                            pass |= 0x08;
                        }
                        break;

                    case 2:
                        if (post != 1)
                        {
                            pass |= 0x80;
                        }
                        break;

                    case 3:
                        if (post != 2)
                        {
                            pass |= 0x80;
                        }
                        break;

                    case 4:
                        if (post != 3)
                        {
                            pass |= 0x80;
                        }
                        break;

                    case 5:
                        if (post != 4)
                        {
                            pass |= 0x80;
                        }
                        break;

                    case 6:
                        if (post != 5)
                        {
                            pass |= 0x80;
                        }
                        break;

                    case 7:
                        if (post != 6)
                        {
                            pass |= 0x80;
                        }
                        break;

                    default:
                        pass |= 0x80;
                        break;
                }
            }
        }
    }
    if ((pass && 0x60) != 0)
    {
        printf("Failed");
    }
    else
    {
        printf("Passed");
    }
    printf("!\n");
    if (pass == 0)
    {
        printf("All saturating counter tests passed\n");
    }
    else
    {
        printf("At lease one saturating counter test failed (0x%02x)\n", pass);
    }
    return;
}

/*
 * main
 *  This function is compiled in when unit testing.  It exercises the branch
 *  prediction code, and should be somewhat extensive.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  0 - always a success.
 *  None.
 */
int main()
{
    FILE *fp;
    char *fileNames[] =
    {
        "trace1.txt",
        "trace2.txt",
        "trace3.txt",
        "trace4.txt",
        "trace5.txt",
		"trace-matmul.txt",
		"trace-12queens.txt",
		"trace-fib30.txt",
		"trace-ray.txt"
    };
    char fileName[AXP_MAX_FILENAME_LEN];
    bool taken;
    int takenInt;
    int ii, jj, kk;
    AXP_21264_CPU *cpu;
    AXP_PC vpc = {.pal = 0, .res = 0};
    int vpcInt;
    bool localTaken;
    bool globalTaken;
    bool choice;
    bool prediction;
    int insCnt;
    int predictedCnt;
    int localCnt;
    int globalCnt;
    int choiceUsed;
    int choiceCorrect;
    int totalIns;
    int totalCorrect;
    int totalMispredict;
    float totalPredictAccuracy;
    int totalLocalCorrect;
    int totalGlobalCorrect;
    int totalChoiceUsed;
    int totalChoiceCorrect;
    int totalChoiceWrong;
    int numberOfFiles = (int) (sizeof(fileNames)/sizeof(char *));

    /*
     * NOTE:  The current simulation takes in one instruction at a time.  The
     *        AXP simulator will process four instructions at a time and
     *        potentially out of order.  When a branch instruction is retired,
     *        only then is the
     */
    printf("\nAXP 21264 Predictions Unit Tester\n");
    printf("\nFirst, we run the various implementations of 2 and 3 bit\n");
    printf("saturating counters through various implementations to test\n");
    printf("which is faster.\n");
    TestSaturatingCounters();
    printf("\nFinally, we'll actually test the branch prediction code.\n");
    printf("%d trace files to be processed\n\n",
           (int) (sizeof(fileNames)/sizeof(char *)));
    cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);
    for (kk = 0; kk < 3; kk++)
    {
        totalIns = 0;
        totalCorrect = 0;
        totalMispredict = 0;
        totalPredictAccuracy = 0.0;
        totalLocalCorrect = 0;
        totalGlobalCorrect = 0;
        totalChoiceUsed = 0;
        totalChoiceCorrect = 0;
        totalChoiceWrong = 0;

        /*
         * bp_mode has the following values:
         *  0b00 = Choice selects between local and global history
         *         predictions.
         *  0b01 = Only local prediction is used.
         *  0b1x = Prediction always returns false.
         */
        cpu->iCtl.bp_mode = kk;
        printf("====================================================\n");
        switch(kk)
        {
            case 0x00:
                printf("Choice predictor selects either Local or Global History Predictions\n");
                break;
            case 0x01:
                printf("Only Local Prediction is used\n");
                break;
            case 0x02:
                printf("Predictor always returns false (fall-through)\n");
                break;
        }
        for (ii = 0; ii < numberOfFiles; ii++)
        {
            sprintf(fileName, "%s/%s", AXP_TEST_DATA_FILES, fileNames[ii]);
            fp = fopen(fileName, "r");
            if (fp != NULL)
            {
                int lineCount = 0;
                int ch;

                while(!feof(fp))
                {
                    ch = fgetc(fp);
                    if(ch == '\n')
                    {
                        lineCount++;
                    }
                }
                fseek(fp, 0, SEEK_SET);
                insCnt = 0;
                predictedCnt = 0;
                localCnt = 0;
                globalCnt = 0;
                choiceUsed = 0;
                choiceCorrect = 0;
                printf("\nProcessing trace file: %s (%d)...\n",
                       fileNames[ii],
                       lineCount);
                while (feof(fp) == 0)
                {
                    fscanf(fp, "%d %d\n", &vpcInt, &takenInt);
                    insCnt++;
                    vpc.pc = vpcInt;
                    taken = (takenInt == 1) ? true : false;

                    /*
                     * Predict whether the branch should be taken or not.
                     * We'll get results from the Local and Global Predictor,
                     * and the Choice selected (when the Local and Global do
                     * not agree).
                     */
                    prediction = AXP_Branch_Prediction(cpu,
                                                       vpc,
                                                       &localTaken,
                                                       &globalTaken,
                                                       &choice);
                    if (prediction == taken)
                    {
                        predictedCnt++;
                    }

                    /*
                     * Let's determine how the choice was determined.
                     *
                     * No prediction.  Always returns false (fall-through)
                     */
                    if (kk != 2)
                    {
                        if (kk != 1)    /* Local prediction only. */
                        {
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
                        }
                        else if (taken == localTaken)
                        {
                            localCnt++;
                        }
                    }

                    /*
                     * Update the predictors based on whether the branch was
                     * actually taken or not and considering which of the
                     * predictors was correct.
                     *
                     * NOTE:    Whether choice was used or not is irrelevant.
                     *          The choice is determined by whether the local
                     *          or global were correct.  If both are correct or
                     *          both are incorrect, then the choice was not
                     *          used and thus would not have made a difference.
                     */
                    AXP_Branch_Direction(cpu,
                                         vpc,
                                         taken,
                                         localTaken,
                                         globalTaken);
                }
                fclose(fp);

                /*
                 * Print out what we found.
                 */
                printf("---------------------------------------------\n");
                printf("Total instructions:\t\t\t%d\n", insCnt);
                printf("Correct predictions:\t\t\t%d\n", predictedCnt);
                printf("Mispredictions:\t\t\t\t%d\n", (insCnt - predictedCnt));
                printf("Prediction accuracy:\t\t\t%1.6f\n\n",
                       ((float) predictedCnt / (float) insCnt));
                printf("Times local correct:\t\t\t%d\n", localCnt);
                printf("Times global correct:\t\t\t%d\n", globalCnt);
                printf("Times choice used:\t\t\t%d\n", choiceUsed);
                printf("Times choice selected correctly:\t%d\n",
                       choiceCorrect);
                printf("Times choice was wrong:\t\t\t%d\n",
                       (choiceUsed - choiceCorrect));

                totalIns += insCnt;
                totalCorrect += predictedCnt;
                totalMispredict += (insCnt - predictedCnt);
                totalPredictAccuracy += ((float) predictedCnt / (float) insCnt);
                totalLocalCorrect += localCnt;
                totalGlobalCorrect += globalCnt;
                totalChoiceUsed += choiceUsed;
                totalChoiceCorrect += choiceCorrect;
                totalChoiceWrong += (choiceUsed - choiceCorrect);

                /*
                 * We need to clear out the prediction tables in the CPU
                 * record.
                 */
                cpu->globalPathHistory = 0;
                for (jj = 0; jj < ONE_K; jj++)
                {
                    cpu->localHistoryTable.lcl_history[jj] = 0;
                }
                for (jj = 0; jj < ONE_K; jj++)
                {
                    cpu->localPredictor.lcl_pred[jj] = 0;
                }
                for (jj = 0; jj < FOUR_K; jj++)
                {
                    cpu->globalPredictor.gbl_pred[jj] = 0;
                }
                for (jj = 0; jj < FOUR_K; jj++)
                {
                    cpu->choicePredictor.choice_pred[jj] = 0;
                }
            }
            else
            {
                printf("Unable to open trace file: %s\n", fileName);
            }
        }
        printf("\n---------------------------------------------\n");

        /*
         * Averages for each predictor mode.
         */
        switch(kk)
        {
            case 0x00:
                printf("Choice Predictor Average:\n");
                break;

            case 0x01:
                printf("Only Local Prediction Average:\n");
                break;

            case 0x02:
                printf("Predictor always returns false (Fall-Through)"
                       " Average:\n");
                break;
        }
        printf("Average number of instructions:\t\t%d\n",
               totalIns / numberOfFiles);
        printf("Average correct predictions:\t\t%d\n",
               totalCorrect / numberOfFiles);
        printf("Average mispredictions:\t\t\t%d\n",
               totalMispredict / numberOfFiles);
        printf("Average prediction accuracy:\t\t%1.6f\n\n",
               (float) totalPredictAccuracy / (float) numberOfFiles);
        printf("Average local correct:\t\t\t%d\n",
               totalLocalCorrect / numberOfFiles);
        printf("Average global correct:\t\t\t%d\n",
               totalGlobalCorrect / numberOfFiles);
        printf("Average choice used:\t\t\t%d\n",
               totalChoiceUsed / numberOfFiles);
        printf("Average choice selected Correctly:\t%d\n",
               totalChoiceCorrect / numberOfFiles);
        printf("Average choice wrong:\t\t\t%d\n",
               totalChoiceWrong / numberOfFiles);
    }
    AXP_Deallocate_Block(cpu);
    return(0);
}
