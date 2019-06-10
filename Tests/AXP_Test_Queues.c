/*
 * Copyright (C) Jonathan D. Belanger 2019.
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
 *  The C99 standard introduced strict-aliasing, and the more recent versions
 *  of GCC make this the default when optimizing code with -O2 and above.
 *  Therefore, I am writing this code to test out the queue functionality to
 *  make sure that strict-aliasing causes any problems.  A queue is defined as
 *  follows:
 *
 *      struct queue {
 *          struct queue *flink;
 *          struct queue *blink;
 *      };
 *
 *  This structure is used throughout the code to have queues of structures,
 *  with the head defined as:
 *
 *      struct queue head;
 *
 *  We need to make sure that strict-aliasing is not going to cause any issues
 *  with regard to this, as this is ubiquitous in the code.
 *
 *  NOTE:   We are going to use as few other parts of DECaxp as possible, so
 *          that we can focus this test on just the one thing.
 *
 * Revision History:
 *
 *  V01.000	11-May-2019 Jonathan D. Belanger
 *  Initially written.
 */
#include "CommonUtilities/AXP_Utility.h"

/*
 * This structure contains a queue as a header.
 */
typedef struct
{
    AXP_QUEUE_HDR head;
    int entry;
} RANDOM_QUEUE;

#define QUEUE_COUNT 100

int main(void)
{
    AXP_QUEUE_HDR *next = NULL;
    AXP_QUEUE_HDR *tail;
    AXP_QUEUE_HDR head;
    int retVal = 0;
    int ii;

    /*
     * First test, allocate a list of just queue items and then count the
     * number of items put onto the queue.
     */
    AXP_INIT_QUE(head);
    tail = &head;
    printf("\nTesting simple queue\n");
    printf("    Creating a queue of 100 entries\n");
    for (ii = 0; ii < QUEUE_COUNT; ii++)
    {
        next = calloc(1, sizeof(AXP_QUEUE_HDR));
        AXP_INSQUE(tail, next);
        tail = next;
    }
    printf("    Testing the queue for 100 entries\n");
    next = &head;
    if (AXP_QUE_EMPTY(head))
    {
        printf("    Queue came back empty.  This is not good.\n");
        retVal = -1;
    }
    else if (AXP_QUEP_EMPTY(next))
    {
        printf("    Pointer to queue came back empty.  This is not good.\n");
        retVal = -1;
    }
    else
    {
        ii = 0;
        next = next->flink;
        printf("    Counting the queue's entries\n");
        while (next != &head)
        {
            ii++;
            next = next->flink;
        }
        if (QUEUE_COUNT != ii)
        {
            printf("    Expected %d, got %d, simple queue items.  "
                   "This is not good.\n",
                   QUEUE_COUNT,
                   ii);
            retVal = -1;
        }
        else
        {
            printf("    Deallocating simple queue entries\n");
            while (head.flink != &head)
            {
                next = head.flink;
                AXP_REMQUE(next);
                free(next);
            }
            printf("Simple queue tests passed\n");
        }
    }

    /*
     * If the above test(s) passed, let's test allocating something other than
     * just a queue header works.
     */
    if (retVal == 0)
    {
        RANDOM_QUEUE *item;

        printf("\nTesting non-simple queue\n");
        printf("    Creating a queue of 100 entries\n");
        tail = &head;
        for (ii = 0; ii < QUEUE_COUNT; ii++)
        {
            item = calloc(1, sizeof(RANDOM_QUEUE));
            item->entry = ii;
            AXP_INSQUE(tail, &item->head);
            tail = &item->head;
        }
        printf("    Testing queue for 100 entries\n");
        tail = &head;
        if (AXP_QUE_EMPTY(head))
        {
            printf("    Queue came back empty.  This is not good.\n");
            retVal = -1;
        }
        else if (AXP_QUEP_EMPTY(tail))
        {
            printf("    Pointer queue came back empty.  This is not good.\n");
            retVal = -1;
        }
        else
        {
            ii = 0;
            item = (RANDOM_QUEUE *) head.flink;
            printf("    Counting and verifying queue's entries\n");
            while ((item != (RANDOM_QUEUE *) &head) && (retVal == 0))
            {
                if (item->entry != ii)
                {
                    printf("    Queue items not in order at %d\n", ii);
                    retVal = -1;
                }
                ii++;
                item = (RANDOM_QUEUE *) item->head.flink;
            }
            if ((QUEUE_COUNT != ii) && (retVal == 0))
            {
                printf("    Expected %d, got %d, non-simple queue items.  "
                       "This is not good.\n",
                       QUEUE_COUNT,
                       ii);
                retVal = -1;
            }
            else if (retVal == 0)
            {
                printf("    Deallocating queue's entries\n");
                while (head.flink != &head)
                {
                    item = (RANDOM_QUEUE *) head.flink;
                    AXP_REMQUE((AXP_QUEUE_HDR *) item);
                    free(item);
                }
                printf("Non-simple queue tests passed\n");
            }
        }
    }

    /*
     * Print final results.
     */
    if (retVal == 0)
    {
        printf("\nAll tests passed!\n");
    }
    else
    {
        printf("\nAt least one test failed!\n");
    }

    /*
     * Return the status of this back
     */
    return retVal;
}
