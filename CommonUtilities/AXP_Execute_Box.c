/*
 * Copyright (C) Jonathan D. Belanger 2018-2019.
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
 *  This source file contains the functions needed to implement the instruction
 *  execution loop for both the Ebox and Fbox.
 *
 *  Revision History:
 *
 *  V01.000 26-Jan-2018 Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001 11-Mar-2018 Jonathan D. Belanger
 *  Introduced a flag to indicate that though there may be instructions to
 *  process, we need to wait until signaled to be able to process them  This
 *  avoids having the code loop until there is something to process.  Now it'll
 *  just wait on its condition variable.
 *
 *  V01.002 11-May-2019 Jonathan D. Belanger
 *  GCC 7.4.0, and possibly earlier, turns on strict-aliasing rules by default.
 *  There are a number of issues in this module where the address of one
 *  variable is cast to extract a value in a different format.  In this module
 *  these all appear to be when trying to get the 64-bit value equivalent of
 *  the 64-bit long PC structure.  We will use shifts (in a macro) instead of
 *  the casts.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "CPU/Fbox/AXP_21264_Fbox.h"
#include "CPU/Ibox/AXP_21264_Ibox.h"
#include "CPU/Ibox/AXP_21264_Ibox_InstructionInfo.h"
#include "CommonUtilities/AXP_Trace.h"

#define AXP_PIPE_OPTIONS    10
static AXP_PIPELINE pipeCond[AXP_PIPE_OPTIONS][3] =
{
    {PipelineNone,  PipelineNone,   PipelineNone},
    {EboxU0,        EboxU0U1,       EboxL0L1U0U1},
    {EboxU1,        EboxU0U1,       EboxL0L1U0U1},
    {PipelineNone,  PipelineNone,   PipelineNone},
    {EboxL0,        EboxL0L1,       EboxL0L1U0U1},
    {EboxL1,        EboxL0L1,       EboxL0L1U0U1},
    {PipelineNone,  PipelineNone,   PipelineNone},
    {PipelineNone,  PipelineNone,   PipelineNone},
    {FboxMul,       FboxMul,        FboxMul},
    {FboxOther,     FboxOther,      FboxOther}
};
static char *pipelineStr[] =
{
    "None",
    "Ebox U0",
    "Ebox U1",
    "",
    "Ebox L0",
    "Ebox L1",
    "",
    "",
    "Fbox Multiply",
    "Fbox Other"
};
static char *insPipelineStr[] =
{
    "None",
    "U0",
    "U1",
    "U0, U1",
    "L0",
    "L1",
    "L0, L1",
    "L0, L1, U0, U1",
    "Multiply",
    "Other"
};
static char *insStateStr[] =
{
    "Retired",
    "Queued",
    "Executing",
    "WaitingRetirement",
    "Aborted"
};
static char *regStateStr[] = {"Free", "Pending Update", "Valid"};
static char *eBoxClusterStr[] = {"L0", "L1", "U0", "U1"};
static char *fBoxClusterStr[] = {"MULTIPLY", "OTHER"};

/*
 * AXP_RegisterReady
 *  This function is called to determine if a queued instruction's registers
 *  are ready for execution.  If one or more registers is waiting for a
 *  previous instruction to finish its execution and store the value this
 *  instruction needs.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  entry:
 *      A pointer to the entry containing all the pre-parsed information of the
 *      instruction so that we can determine which physical registers are being
 *      used and which are needed for this instruction.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *   true:      The registers for instruction execution are ready.
 *   false:     The registers for instruction execution are NOT ready.
 */
static bool AXP_RegistersReady(AXP_21264_CPU *cpu, AXP_QUEUE_ENTRY *entry)
{
    AXP_REGISTERS *src1Reg;
    AXP_REGISTERS *src2Reg;
    AXP_REGISTERS *destReg;
    char *warn = "";
    bool src1Float;
    bool src2Float;
    bool destFloat;
    bool retVal;

    src1Float = ((entry->ins->decodedReg.bits.src1 & AXP_REG_FP) == AXP_REG_FP);
    src2Float = ((entry->ins->decodedReg.bits.src2 & AXP_REG_FP) == AXP_REG_FP);
    destFloat = ((entry->ins->decodedReg.bits.dest & AXP_REG_FP) == AXP_REG_FP);

    src1Reg = (src1Float ? cpu->pf : cpu->pr);
    src2Reg = (src2Float ? cpu->pf : cpu->pr);
    destReg = (destFloat ? cpu->pf : cpu->pr);
    if (destReg[entry->ins->dest].state !=
            ((entry->ins->dest == AXP_UNMAPPED_REG) ? Valid : PendingUpdate))
    {
        warn = "******";
    }
    if (AXP_UTL_OPT2)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("AXP_RegistersReady checking registers at pc = "
                       "0x%016llx, opcode = 0x%02x:",
                       AXP_GET_PC(entry->ins->pc),
                       (u32) entry->ins->opcode);
        AXP_TraceWrite("\tSrc1 (%c%02u) = %s",
                       (src1Float ? 'F' : 'R'),
                       entry->ins->aSrc1,
                       regStateStr[src1Reg[entry->ins->src1].state],
                       (src1Float ? 'F' : 'R'),
                       entry->ins->src1);
        AXP_TraceWrite("\tSrc2 (%c%02u) = %s",
                       (src2Float ? 'F' : 'R'),
                       entry->ins->aSrc2,
                       regStateStr[src2Reg[entry->ins->src2].state],
                       (src2Float ? 'F' : 'R'),
                       entry->ins->src2);
        AXP_TraceWrite("\tDest (%c%02u) = %s (P%c%02u) %s",
                       (destFloat ? 'F' : 'R'),
                       entry->ins->aDest,
                       regStateStr[destReg[entry->ins->dest].state],
                       (destFloat ? 'F' : 'R'),
                       entry->ins->dest,
                       warn);
        AXP_TRACE_END();
    }

    retVal = ((src1Reg[entry->ins->src1].state == Valid) &&
              (src2Reg[entry->ins->src2].state == Valid) &&
              (destReg[entry->ins->dest].state ==
                  ((entry->ins->dest == AXP_UNMAPPED_REG) ?
                          Valid :
                          PendingUpdate)));

    /*
     * Move the contents of the source registers into the location where the
     * instruction execution expects to find them.
     */
    if (src1Float)
    {
        entry->ins->src1v.fp.uq = src1Reg[entry->ins->src1].value;
    }
    else
    {
        entry->ins->src1v.r.uq = src1Reg[entry->ins->src1].value;
    }
    if (src2Float)
    {
        entry->ins->src1v.fp.uq = src2Reg[entry->ins->src1].value;
    }
    else
    {
        entry->ins->src1v.r.uq = src2Reg[entry->ins->src1].value;
    }

    /*
     * Return the result back to the caller.
     */
    return (retVal);
}

/*
 * AXP_Execution_Box
 *  This function is called by both the Ebox and Fbox.  The processing loops
 *  for both of them are incredibly similar.  The only real differences are the
 *  determination if a particular pipeline is allowed to execute a particular
 *  instruction, and returning a completed instruction queue entry back to the
 *  pool for a subsequent instruction.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the CPU structure where the instruction queues are
 *      located.
 *  pipeline:
 *      A value indicating the pipeline this function is to process.  This can
 *      be one of the following:
 *          EboxU0
 *          EboxU1
 *          EboxL0
 *          EboxL1
 *          FboxMul
 *          FboxOther
 *  cond:
 *      A pointer to the condition variable associated with the pipeline.
 *  mutex:
 *      A pointer to the mutex variable associated with the pipeline.
 *  regCheckEntry:
 *      A pointer to the function to determine if the registers are ready for
 *      the instruction to be allowed to execute.
 *  returnEntry:
 *      A pointer to the function to return the dequeued entry back to the
 *      pool for a later instruction to be executed.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_Execution_Box(AXP_21264_CPU *cpu,
                       AXP_PIPELINE pipeline,
                       AXP_COUNTED_QUEUE *queue,
                       pthread_cond_t *cond,
                       pthread_mutex_t *mutex,
                       void (*returnEntry)(AXP_21264_CPU *, AXP_QUEUE_ENTRY *))
{
    AXP_QUEUE_ENTRY *entry, *next;
    u16 *clusterCounter;
    AXP_INS_STATE state;
    int clusterCountIdx;
    bool fpEnable;
    bool eBox;
    bool nothingReadyForMe = false;

    switch (pipeline)
    {
        case EboxL0:
            clusterCountIdx = AXP_21264_EBOX_L0;
            clusterCounter = cpu->eBoxClusterCounter;
            eBox = true;
            break;

        case EboxL1:
            clusterCountIdx = AXP_21264_EBOX_L1;
            clusterCounter = cpu->eBoxClusterCounter;
            eBox = true;
            break;

        case EboxU0:
            clusterCountIdx = AXP_21264_EBOX_U0;
            clusterCounter = cpu->eBoxClusterCounter;
            eBox = true;
            break;

        case EboxU1:
            clusterCountIdx = AXP_21264_EBOX_U1;
            clusterCounter = cpu->eBoxClusterCounter;
            eBox = true;
            break;

        case FboxMul:
            clusterCountIdx = AXP_21264_FBOX_MULTIPLY;
            clusterCounter = cpu->fBoxClusterCounter;
            eBox = false;
            break;

        case FboxOther:
        default:    /* This is just to keep the compiler from complaining */
            clusterCountIdx = AXP_21264_FBOX_OTHER;
            clusterCounter = cpu->fBoxClusterCounter;
            eBox = false;
            break;
    }

    /*
     * Before we go into the loop, lock the E/Fbox mutex.
     */
    pthread_mutex_lock(mutex);

    /*
     * While we are not shutting down, we'll continue to try and process
     * instructions.
     */
    while (cpu->cpuState != ShuttingDown)
    {

        /*
         * Next we need to do is see if there is nothing to process,
         * then wait for something to get queued up.
         */
        while ((cpu->cpuState != ShuttingDown) &&
               ((AXP_CountedQueueFull(queue, 0) == 1) ||
                (clusterCounter[clusterCountIdx] == 0) ||
                (nothingReadyForMe == true)))
        {
            nothingReadyForMe = false;
            pthread_cond_wait(cond, mutex);
            if (AXP_UTL_OPT2)
            {
                AXP_TRACE_BEGIN();
                AXP_TraceWrite("%s signaled [%s] = %u.",
                               pipelineStr[pipeline],
                               (eBox ? eBoxClusterStr[clusterCountIdx] :
                                       fBoxClusterStr[clusterCountIdx]),
                               clusterCounter[clusterCountIdx]);
                AXP_TRACE_END();
            }
        }

        /*
         * If we are not shutting down, then we may have something to process.
         * Let's go looking for trouble.
         */
        if (cpu->cpuState != ShuttingDown)
        {

            /*
             * We need to prevent multiple threads trying to update this queue.
             */
            AXP_LockCountedQueue(queue);
            entry = (AXP_QUEUE_ENTRY *) queue->flink;

            /*
             * Search through the queue of pending integer pipeline
             * instructions.  If we find one for this cluster, then break out
             * of this loop.  Otherwise, move on to the next entry in the
             * queue.  Since the queue eventually points back to the
             * parent/header, this will exit the loop as well.
             */
            while ((AXP_COUNTED_QUEUE *) entry != queue)
            {

                /*
                 * TODO:    We need to take into account the scoreboard bits.
                 *
                 * HRM: 6.5.1 IPR Scoreboard Bits (page 6-8)
                 *
                 * In previous Alpha implementations, IPR registers were not
                 * scoreboarded in hardware.  Software was required to schedule
                 * HW_MTPR and HW_MFPR instructions for each machine’s pipeline
                 * organization in order to ensure correct behavior. This
                 * software scheduling task is more difficult in the 21264
                 * because the Ibox performs dynamic scheduling. Hence, eight
                 * extra scoreboard bits are used within the IQ to help
                 * maintain correct IPR access order. The HW_MTPR and HW_MFPR
                 * instruction formats contain an 8-bit field that is used as
                 * an IPR scoreboard bit mask to specify which of the eight IPR
                 * scoreboard bits are to be applied to the instruction.
                 *
                 * If any of the unmasked scoreboard bits are set when an
                 * instruction is about to enter the IQ, then the instruction,
                 * and those behind it, are stalled outside the IQ until all
                 * the unmasked scoreboard bits are clear and the queue does
                 * not contain any implicit or explicit readers that were
                 * dependent on those bits when they entered the queue. When
                 * all the unmasked scoreboard bits are clear, and the queue
                 * does not contain any of those readers, the instruction
                 * enters the IQ and the unmasked scoreboard bits are set.
                 *
                 * HW_MFPR instructions are stalled in the IQ until all their
                 * unmasked IPR scoreboard bits are clear.
                 *
                 * When scoreboard bits [3:0] and [7:4] are set, their effect
                 * on other instructions is different, and they are cleared in
                 * a different manner.  If any of scoreboard bits [3:0] are set
                 * when a load or store instruction enters the IQ, that load or
                 * store instruction will not be issued from the IQ until those
                 * scoreboard bits are clear.
                 *
                 * Scoreboard bits [3:0] are cleared when the HW_MTPR
                 * instructions that set them are issued (or are aborted).
                 * Bits [7:4] are cleared when the HW_MTPR instructions that
                 * set them are retired (or are aborted).
                 *
                 * Bits [3:0] are used for the DTB_TAG and DTB_PTE register
                 * pairs within the DTB fill flows. These bits can be used to
                 * order writes to the DTB for load and store instructions.
                 * See Sections 5.3.1 and 6.9.1.
                 *
                 * Bit [0] is used in both DTB and ITB fill flows to trigger,
                 * in hardware, a lightweight memory barrier (TB-MB) to be
                 * inserted between a LD_VPTE and the corresponding
                 * virtual-mode load instruction that missed in the TB.
                 *
                 * NOTE: Because of the out-of-order execution of the IQ and
                 *       FQ, this code needs to keep track of the scoreboard
                 *       bits as the instruction is considered for execution.
                 *       What we don't want to happen is have a load/store
                 *       executed before the HW_MTPR DTB_TAG and DTB_PTE that
                 *       could change the addresses the load/store utilize.  We
                 *       also need to maintain a current scoreboard, that is
                 *       will be used by the Ibox to know when to queue up or
                 *       not more instructions that may depend on the value of
                 *       the IPRs.
                 */

                /*
                 * Get the next queued entry, because if an instruction was
                 * aborted, but not yet dequeued, we are going to have to get
                 * rid of this entry and not process it.
                 */
                next = (AXP_QUEUE_ENTRY *) entry->header.flink;

                if (AXP_UTL_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("%s queue = 0x%016llx, entry = 0x%016llx, "
                                   "next = 0x%016llx",
                                   pipelineStr[pipeline],
                                   queue,
                                   entry,
                                   next);
                    AXP_TraceWrite("%s checking at "
                                   "pc = 0x%016llx, "
                                   "opcode = 0x%02x, "
                                   "pipeline = %s, "
                                   "state = %s.",
                                   pipelineStr[pipeline],
                                   AXP_GET_PC(entry->ins->pc),
                                   (u32) entry->ins->opcode,
                                   insPipelineStr[entry->pipeline],
                                   insStateStr[entry->ins->state]);
                    AXP_TRACE_END();
                }

                /*
                 * If this entry can be executed by this pipeline, and the
                 * registers are all ready, and some other pipeline has not
                 * already started processing this entry, or the instruction is
                 * being aborted, then we should process/abort it now.
                 *
                 * NOTE:    Because of the way we have to lock/unlock/lock the
                 *          eBoxMutex/fBoxMutex and the robMutex, it is
                 *          possible for an instruction that can be executed in
                 *          more than one pipeline to have already been picked
                 *          up for processing/aborting.
                 */
                if (((((entry->pipeline == pipeCond[pipeline][0]) ||
                       (entry->pipeline == pipeCond[pipeline][1]) ||
                       (entry->pipeline == pipeCond[pipeline][2])) &&
                      (AXP_RegistersReady(cpu, entry) == true)) ||
                     (entry->ins->state == Aborted)) &&
                    (entry->processing == false))
                {
                    entry->processing = true;
                    break;
                }

                pthread_mutex_lock(&cpu->iBoxIPRMutex);

                /*
                 * HRM 5.2.14 - Ibox Control Register (page 5-18)
                 *
                 * If we are in Single Issue Mode, when set, this bit forces
                 * instructions to issue only from the bottom-most entries of
                 * the IQ and FQ.  Bottom-most in this implementation is
                 * pointed to by the forward link of the queue head.  Setting
                 * the next entry to look at as the queue head will cause this
                 * while loop to exit.
                 */
                if (cpu->iCtl.single_issue_h == 1)
                {
                    entry = (AXP_QUEUE_ENTRY *) queue;
                }
                else
                {
                    entry = next;
                }
                pthread_mutex_unlock(&cpu->iBoxIPRMutex);
            }

            /*
             * If we did not find an instruction to execute, then go back to
             * the beginning of the loop.  Since we did not unlock the mutex,
             * we do not need to lock it now.
             */
            if ((AXP_COUNTED_QUEUE *) entry == queue)
            {
                if (AXP_UTL_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("%s has nothing to process.",
                                   pipelineStr[pipeline]);
                    AXP_TRACE_END();
                }
                nothingReadyForMe = true;
                AXP_UnlockCountedQueue(queue);
                continue;
            }

            /*
             * First we need to lock the ROB mutex.  We don't want some
             * other thread changing the contents while we are looking at
             * it.  We are looking to see if the instruction was aborted.
             */
            pthread_mutex_lock(&cpu->robMutex);
            if ((state = entry->ins->state) == Queued)
            {
                entry->ins->state = Executing;
            }
            pthread_mutex_unlock(&cpu->robMutex);
            if (state == Aborted)
            {
                AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry, true);
                if ((entry->pipeline == EboxU0) ||
                    (entry->pipeline == EboxU0U1) ||
                    (entry->pipeline == EboxL0L1U0U1))
                {
                    cpu->eBoxClusterCounter[AXP_21264_EBOX_U0]--;
                }
                if ((entry->pipeline == EboxU1) ||
                    (entry->pipeline == EboxU0U1) ||
                    (entry->pipeline == EboxL0L1U0U1))
                {
                    cpu->eBoxClusterCounter[AXP_21264_EBOX_U1]--;
                }
                if ((entry->pipeline == EboxL0) ||
                    (entry->pipeline == EboxL0L1) ||
                    (entry->pipeline == EboxL0L1U0U1))
                {
                    cpu->eBoxClusterCounter[AXP_21264_EBOX_L0]--;
                }
                if ((entry->pipeline == EboxL1) ||
                    (entry->pipeline == EboxL0L1) ||
                    (entry->pipeline == EboxL0L1U0U1))
                {
                    cpu->eBoxClusterCounter[AXP_21264_EBOX_L1]--;
                }
                if (entry->pipeline == FboxMul)
                {
                    cpu->fBoxClusterCounter[AXP_21264_FBOX_MULTIPLY]--;
                }
                else if (entry->pipeline == FboxOther)
                {
                    cpu->fBoxClusterCounter[AXP_21264_FBOX_OTHER]--;
                }
                entry->processing = false;
                (*returnEntry)(cpu, entry);
                continue;
            }

            /*
             * OK, we have something to execute.  Mark the entry as such and
             * dequeue it from the queue.  Then, dispatch it to the function
             * to execute the instruction.
             */
            if (AXP_UTL_OPT2)
            {
                AXP_TRACE_BEGIN();
                AXP_TraceWrite("%s has something to process at "
                               "pc = 0x%016llx, opcode = 0x%02x.",
                               pipelineStr[pipeline],
                               AXP_GET_PC(entry->ins->pc),
                               (u32) entry->ins->opcode);
                AXP_TRACE_END();
            }
            AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry, true);
            if ((entry->pipeline == EboxU0) ||
                (entry->pipeline == EboxU0U1) ||
                (entry->pipeline == EboxL0L1U0U1))
            {
                cpu->eBoxClusterCounter[AXP_21264_EBOX_U0]--;
            }
            if ((entry->pipeline == EboxU1) ||
                (entry->pipeline == EboxU0U1) ||
                (entry->pipeline == EboxL0L1U0U1))
            {
                cpu->eBoxClusterCounter[AXP_21264_EBOX_U1]--;
            }
            if ((entry->pipeline == EboxL0) ||
                (entry->pipeline == EboxL0L1) ||
                (entry->pipeline == EboxL0L1U0U1))
            {
                cpu->eBoxClusterCounter[AXP_21264_EBOX_L0]--;
            }
            if ((entry->pipeline == EboxL1) ||
                (entry->pipeline == EboxL0L1) ||
                (entry->pipeline == EboxL0L1U0U1))
            {
                cpu->eBoxClusterCounter[AXP_21264_EBOX_L1]--;
            }
            if (entry->pipeline == FboxMul)
            {
                cpu->fBoxClusterCounter[AXP_21264_FBOX_MULTIPLY]--;
            }
            else if (entry->pipeline == FboxOther)
            {
                cpu->fBoxClusterCounter[AXP_21264_FBOX_OTHER]--;
            }

            /*
             * If Floating-Point instructions are enabled, then call the
             * dispatcher to dispatch this instruction to the correct function
             * to execute the instruction.  Otherwise, set the appropriate
             * exception value.  To keep the following code simpler, we set the
             * fpEnable flag to true for all integer instructions.
             */
            if ((pipeline == FboxMul) || (pipeline == FboxOther))
            {
                pthread_mutex_lock(&cpu->iBoxIPRMutex);
                fpEnable = cpu->pCtx.fpe
                           == 1;
                pthread_mutex_unlock(&cpu->iBoxIPRMutex);
            }
            else
            {
                fpEnable = true;
            }

            if (fpEnable == true)
            {

                /*
                 * Call the dispatcher to dispatch this instruction to the correct
                 * function to execute the instruction.
                 */
                if (AXP_UTL_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("%s dispatching instruction, opcode = 0x%02x",
                                   pipelineStr[pipeline],
                                   entry->ins->opcode);
                    AXP_TRACE_END();
                }

                /*
                 * Now we can call the dispatcher to execute the instruction.
                 */
                AXP_Dispatcher(cpu, entry->ins);
                if (AXP_UTL_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("%s dispatched instruction, opcode = 0x%02x",
                                   pipelineStr[pipeline],
                                   entry->ins->opcode);
                    AXP_TRACE_END();
                }
            }
            else
            {
                if (AXP_UTL_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("Fbox %s : Floating point instructions are "
                                   "currently disabled.",
                                   pipelineStr[pipeline]);
                    AXP_TRACE_END();
                }
                pthread_mutex_lock(&cpu->robMutex);
                entry->ins->excRegMask = FloatingDisabledFault;
                entry->ins->state = WaitingRetirement;
                pthread_mutex_unlock(&cpu->robMutex);
            }

            /*
             * Return the entry back to the pool for future instructions.
             */
            entry->processing = false;
            (*returnEntry)(cpu, entry);

            /*
             * Before we go process any more instructions, let's make sure that
             * the iBox is not stalled.  If it is, then it may have an
             * instruction that it can retire.
             *
             * NOTE:    We intentionally do not have the mutex locked.  Doing
             *          so causes the emulator to get locked up (the Ibox
             *          rarely unlocks the mutex, so we'd effectively get
             *          ourselves into a deadlock.
             */
            if (cpu->stallWaitingRetirement == true)
            {
                pthread_cond_signal(&cpu->iBoxCondition);
            }
        }
    }

    /*
     * Last things last, lock the Ebox mutex.
     */
    pthread_mutex_unlock(mutex);

    /*
     * Return back to the caller.
     */
    return;
}
