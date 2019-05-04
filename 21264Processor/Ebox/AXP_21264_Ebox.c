/*
 * Copyright (C) Jonathan D. Belanger 2017-2018.
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
 *	functionality of the Ebox.
 *
 * Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		02-Nov-2017	Jonathan D. Belanger
 *	Coded the initialization code.
 *
 *	V01.002		01-Jan-2018	Jonathan D. Belanger
 *	Changed the way instructions are completed when they need to utilize the
 *	Mbox.
 *
 *	V01.003		06-Jan-2018	Jonathan D. Belanger
 *	Continuing to implement the main function for the integer pipelines.  At
 *	some point I'll add a main for each pipeline, each of which calls a common
 *	main with parameters to have the common main execute specific to the
 *	pipeline main that called it.
 *
 *	V01.004		27-Feb-2018	Jonathan D. Belanger
 *	The EboxMain and FboxMain functions were nearly identical, so they were
 *	combined into one that is now in COMUTL.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "21264Processor/Ebox/AXP_21264_Ebox.h"
#include "21264Processor/Ibox/AXP_21264_Ibox_InstructionInfo.h"
#include "CommonUtilities/AXP_Trace.h"
#include "CommonUtilities/AXP_Execute_Box.h"

/*
 * AXP_21264_Ebox_Compl
 *	This function is called by the Mbox for Integer Store operations.  This is
 *	very similar to the Integer Load Complete functions that are individually
 *	written for each kind of unique load instruction.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to complete
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	None.
 */
void AXP_21264_Ebox_Compl(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * If no exception occurred, then we have the data we need and just need to
     * store the value that is going to be put into the destination register
     * when the instruction is retired.  There is nothing specific that needs
     * to happen to store instructions.
     *
     * NOTE: Any exception will be handled in the retirement code.
     */
    if (instr->excRegMask == NoException)
    {
  switch (instr->opcode)
  {
      case LDBU:
    instr->destv.r.uq = AXP_ZEXT_BYTE(instr->destv.r.uq);
    break;

      case LDW_U:
    instr->destv.r.uq = AXP_ZEXT_WORD(instr->destv.r.uq);
    break;

      case LDL:
      case LDL_L:
    instr->destv.r.uq = AXP_SEXT_LONG(instr->destv.r.uq);
    break;

      case HW_LD:
    if (instr->quadword == false)
        instr->destv.r.uq = AXP_SEXT_LONG(instr->destv.r.uq);
    break;

      case STL_C:
      case STQ_C:
    instr->destv.r.uq = 1;
    break;

      default:
    break;
  }
    }

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * We want the Ebox threads to handle their own completion.  The Mbox has
     * done what is was supposed to and now we need to tell the Ebox that there
     * is something to retire.
     */
    pthread_mutex_lock(&cpu->eBoxMutex);
    cpu->eBoxWaitingRetirement = true;
    pthread_cond_signal(&cpu->eBoxCondition);
    pthread_mutex_unlock(&cpu->eBoxMutex);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_21264_Ebox_Init
 *	This function is called to initialize the Ebox.  It will set the IPRs
 *	associated with the Ebox to their initial/reset values.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	Failed to perform all initialization processing.
 *	false:	Normal Successful Completion.
 */
bool AXP_21264_Ebox_Init(AXP_21264_CPU *cpu)
{
    bool retVal = false;

    if (AXP_EBOX_OPT1)
    {
  AXP_TRACE_BEGIN();
  AXP_TraceWrite("Ebox is initializing");
  AXP_TRACE_END()
  ;
    }

    /*
     * This bit us used when emulating the RC and BC VAX Compatibility
     * instructions used by VAX-to-Alpha translator software.  ARM 4.12
     */
    cpu->VAXintrFlag = false;

    /*
     * Initialize the Ebox IPRs.
     * NOTE: These will get real values from the PALcode.
     */
    cpu->cc.counter = 0;
    cpu->cc.offset = 0;
    cpu->ccCtl.res_1 = 0;
    cpu->ccCtl.counter = 0;
    cpu->ccCtl.cc_ena = 0;
    cpu->ccCtl.res_2 = 0;
    cpu->va = 0;
    cpu->vaCtl.b_endian = 0;
    cpu->vaCtl.va_48 = 0;
    cpu->vaCtl.va_form_32 = 0;
    cpu->vaCtl.res = 0;
    cpu->vaCtl.vptb = 0;
    cpu->vaForm.form00.res = 0;
    cpu->vaForm.form00.va = 0;
    cpu->vaForm.form00.vptb = 0;

    if (AXP_EBOX_OPT1)
    {
  AXP_TRACE_BEGIN();
  AXP_TraceWrite("Ebox has initialized");
  AXP_TRACE_END()
  ;
    }

    return (retVal);
}

/*
 * AXP_21264_EboxU0Main
 *	This is the main function for the Upper 0 Cluster of the Ebox (Integer)
 *	pipeline.  It is called by the pthread_create function.  It calls the Ebox
 *	main function to perform the instruction execution for the Upper 0 Cluster
 *	of the Digital Alpha AXP 21264 Central Processing Unit emulation.
 *
 * Input Parameters:
 *	voidPtr:
 *		A void pointer which is actually the address of the Digital Alpha AXP
 *		21264 cpu data structure.  It'll be recast within the code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void *AXP_21264_EboxU0Main(void *voidPtr)
{
    AXP_21264_CPU *cpu = (AXP_21264_CPU *) voidPtr;

    if (AXP_EBOX_CALL)
    {
  AXP_TRACE_BEGIN();
  AXP_TraceWrite("Ebox U0 is starting");
  AXP_TRACE_END()
  ;
    }

    /*
     * Call the actual main function with the information it needs to be able
     * to execute instructions for a specific Integer Pipeline.
     */
    AXP_Execution_Box(
  cpu,
  EboxU0,
  &cpu->iq,
  &cpu->eBoxCondition,
  &cpu->eBoxMutex,
  &AXP_ReturnIQEntry);

    /*
     * Return back to the caller.
     */
    return (NULL);
}

/*
 * AXP_21264_EboxU1Main
 *	This is the main function for the Upper 1 Cluster of the Ebox (Integer)
 *	pipeline.  It is called by the pthread_create function.  It calls the Ebox
 *	main function to perform the instruction execution for the Upper 1 Cluster
 *	of teh Digital Alpha AXP 21264 Central Processing Unit emulation.
 *
 * Input Parameters:
 *	voidPtr:
 *		A void pointer which is actually the address of the Digital Alpha AXP
 *		21264 cpu data structure.  It'll be recast within the code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void *AXP_21264_EboxU1Main(void *voidPtr)
{
    AXP_21264_CPU *cpu = (AXP_21264_CPU *) voidPtr;

    if (AXP_EBOX_CALL)
    {
  AXP_TRACE_BEGIN();
  AXP_TraceWrite("Ebox U1 is starting");
  AXP_TRACE_END()
  ;
    }

    /*
     * Call the actual main function with the information it needs to be able
     * to execute instructions for a specific Integer Pipeline.
     */
    AXP_Execution_Box(
  cpu,
  EboxU1,
  &cpu->iq,
  &cpu->eBoxCondition,
  &cpu->eBoxMutex,
  &AXP_ReturnIQEntry);

    /*
     * Return back to the caller.
     */
    return (NULL);
}

/*
 * AXP_21264_EboxL0Main
 *	This is the main function for the Lower 0 Cluster of the Ebox (Integer)
 *	pipeline.  It is called by the pthread_create function.  It calls the Ebox
 *	main function to perform the instruction execution for the Lower 0 Cluster
 *	of the Digital Alpha AXP 21264 Central Processing Unit emulation.
 *
 * Input Parameters:
 *	voidPtr:
 *		A void pointer which is actually the address of the Digital Alpha AXP
 *		21264 cpu data structure.  It'll be recast within the code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void *AXP_21264_EboxL0Main(void *voidPtr)
{
    AXP_21264_CPU *cpu = (AXP_21264_CPU *) voidPtr;

    if (AXP_EBOX_CALL)
    {
  AXP_TRACE_BEGIN();
  AXP_TraceWrite("Ebox L0 is starting");
  AXP_TRACE_END()
  ;
    }

    /*
     * Call the actual main function with the information it needs to be able
     * to execute instructions for a specific Integer Pipeline.
     */
    AXP_Execution_Box(
  cpu,
  EboxL0,
  &cpu->iq,
  &cpu->eBoxCondition,
  &cpu->eBoxMutex,
  &AXP_ReturnIQEntry);

    /*
     * Return back to the caller.
     */
    return (NULL);
}

/*
 * AXP_21264_EboxL1Main
 *	This is the main function for the Lower 1 Cluster of the Ebox (Integer)
 *	pipeline.  It is called by the pthread_create function.  It calls the Ebox
 *	main function to perform the instruction execution for the Lower 1 Cluster
 *	of the Digital Alpha AXP 21264 Central Processing Unit emulation.
 *
 * Input Parameters:
 *	voidPtr:
 *		A void pointer which is actually the address of the Digital Alpha AXP
 *		21264 cpu data structure.  It'll be recast within the code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void *AXP_21264_EboxL1Main(void *voidPtr)
{
    AXP_21264_CPU *cpu = (AXP_21264_CPU *) voidPtr;

    if (AXP_EBOX_CALL)
    {
  AXP_TRACE_BEGIN();
  AXP_TraceWrite("Ebox L1 is starting");
  AXP_TRACE_END();
    }

    /*
     * Call the actual main function with the information it needs to be able
     * to execute instructions for a specific Integer Pipeline.
     */
    AXP_Execution_Box(
  cpu,
  EboxL1,
  &cpu->iq,
  &cpu->eBoxCondition,
  &cpu->eBoxMutex,
  &AXP_ReturnIQEntry);

    /*
     * Return back to the caller.
     */
    return (NULL);
}
