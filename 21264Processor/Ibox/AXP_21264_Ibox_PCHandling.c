/*
 * Copyright (C) Jonathan D. Belanger 2018.
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
 *	PC handling functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		15-Jan-2018	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		20-Jan-2018	Jonathan D. Belanger
 *	The AXP_21264_GetPALBaseVPC function was not being called.  The
 *	functionality in this function was somewhat redundant with the
 *	AXP_21264_GetPALFuncVPC function.  So, the former was removed.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Trace.h"
#include "21264Processor/Ibox/AXP_21264_Ibox.h"

/*
 * A local structure used to calculate the PC for a CALL_PAL function.
 */
struct palBaseBits21264
{
    u64 res :15;
    u64 highPC :49;
};
struct palBaseBits21164
{
    u64 res :14;
    u64 highPC :50;
};

typedef union
{
    struct palBaseBits21164 bits21164;
    struct palBaseBits21264 bits21264;
    u64 palBaseAddr;

} AXP_IBOX_PALBASE_BITS;

struct palPCBits21264
{
    u64 palMode :1;
    u64 mbz_1 :5;
    u64 func_5_0 :6;
    u64 func_7 :1;
    u64 mbo :1;
    u64 mbz_2 :1;
    u64 highPC :49;
};
struct palPCBits21164
{
    u64 palMode :1;
    u64 mbz :5;
    u64 func_5_0 :6;
    u64 func_7 :1;
    u64 mbo :1;
    u64 highPC :50;
};

typedef union
{
    struct palPCBits21164 bits21164;
    struct palPCBits21264 bits21264;
    AXP_PC vpc;
} AXP_IBOX_PAL_PC;

struct palFuncBits
{
    u32 func_5_0 :6;
    u32 res_1 :1;
    u32 func_7 :1;
    u32 res_2 :24;
};

typedef union
{
    struct palFuncBits bits;
    u32 func;
} AXP_IBOX_PAL_FUNC_BITS;

/*
 * AXP_21264_AddVPC
 * 	This function is called to add a Virtual Program Counter (VPC) to the list
 * 	of VPCs.  This is a round-robin list.  The End points to the next entry to
 * 	be written to.  The Start points to the least recent VPC, which is the one
 * 	immediately after the End.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	vpc:
 *		A value of the next VPC to be entered into the VPC list.
 *
 * Output Parameters:
 * 	cpu:
 * 		The vpc list will be updated with the newly added VPC and the Start and
 * 		End indexes will be updated appropriately.
 *
 * Return Value:
 * 	None.
 */
void AXP_21264_AddVPC(AXP_21264_CPU *cpu, AXP_PC vpc)
{
    if (AXP_IBOX_OPT2)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
	    "Adding vPC[%d] 0x%016llx",
	    cpu->vpcEnd,
	    *((u64 *) &vpc));
	AXP_TRACE_END()
	;
    }
    cpu->vpc[cpu->vpcEnd] = vpc;
    cpu->vpcEnd = (cpu->vpcEnd + 1) % AXP_INFLIGHT_MAX;
    if (cpu->vpcEnd == cpu->vpcStart)
	cpu->vpcStart = (cpu->vpcStart + 1) % AXP_INFLIGHT_MAX;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_21264_GetPALFuncVPC
 * 	This function is called to get the Virtual Program Counter (VPC) to a
 * 	specific PAL function which is an offset from the address specified in the
 *	PAL_BASE register.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	func:
 *		The value of the function field in the PALcode Instruction Format.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The value that the PC should be set to to call the requested function.
 */
AXP_PC AXP_21264_GetPALFuncVPC(AXP_21264_CPU *cpu, u32 func)
{
    AXP_IBOX_PAL_PC pc;
    AXP_IBOX_PALBASE_BITS palBase;
    AXP_IBOX_PAL_FUNC_BITS palFunc;

    palBase.palBaseAddr = cpu->palBase.pal_base_pc;
    palFunc.func = func;

    /*
     * We assume that the function supplied follows any of the following
     * criteria:
     *
     *		Is in the range of 0x40 and 0x7f, inclusive
     *		Is greater than 0xbf
     *		Is between 0x00 and 0x3f, inclusive, and IER_CM[CM] is not equal to
     *			the kernel mode value (0).
     *
     * Now, let's compose the PC for the PALcode function we are being
     * requested to call.
     */
    if (cpu->majorType >= EV6)
    {
	pc.bits21264.highPC = palBase.bits21264.highPC;
	pc.bits21264.mbz_2 = 0;
	pc.bits21264.mbo = 1;
	pc.bits21264.func_7 = palFunc.bits.func_7;
	pc.bits21264.func_5_0 = palFunc.bits.func_5_0;
	pc.bits21264.mbz_1 = 0;
	pc.bits21264.palMode = AXP_PAL_MODE;
    }
    else
    {
	pc.bits21164.highPC = palBase.bits21164.highPC;
	pc.bits21164.mbo = 1;
	pc.bits21164.func_7 = palFunc.bits.func_7;
	pc.bits21164.func_5_0 = palFunc.bits.func_5_0;
	pc.bits21164.mbz = 0;
	pc.bits21164.palMode = AXP_PAL_MODE;
    }

    if (AXP_IBOX_OPT2)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("Generated PAL vPC 0x%016llx", *((u64 *) &pc));
	AXP_TRACE_END()
	;
    }

    /*
     * Return the composed VPC it back to the caller.
     */
    return (pc.vpc);
}

/*
 * AXP_21264_MakeVPC
 * 	This function is called to set a Virtual Program Counter (VPC) to a
 * 	specific value.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	addr:
 *		A value of the next VPC to be entered into the VPC list.
 *	palMode:
 *		A value to indicate if we will be running in PAL mode.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The calculated VPC, based on a target virtual address.
 */
AXP_PC AXP_21264_MakeVPC(AXP_21264_CPU *cpu, u64 pc, u8 pal)
{
    union
    {
	u64 pc;
	AXP_PC vpc;
    } vpc;

    vpc.pc = pc;
    vpc.vpc.res = 0;
    vpc.vpc.pal = pal & AXP_PAL_MODE;

    if (AXP_IBOX_OPT2)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("Getting vPC 0x%016llx", *((u64 *) &vpc));
	AXP_TRACE_END()
	;
    }

    /*
     * Return back to the caller.
     */
    return (vpc.vpc);
}

/*
 * AXP_21264_GetNextVPC
 * 	This function is called to retrieve the VPC for the next set of
 * 	instructions to be fetched.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	The VPC of the next set of instructions to fetch from the cache..
 */
AXP_PC AXP_21264_GetNextVPC(AXP_21264_CPU *cpu)
{
    AXP_PC retVal;
    u32 prevVPC;

    /*
     * The End, points to the next location to be filled.  Therefore, the
     * previous location is the next VPC to be executed.
     */
    prevVPC = ((cpu->vpcEnd != 0) ? cpu->vpcEnd : AXP_INFLIGHT_MAX) - 1;
    retVal = cpu->vpc[prevVPC];

    if (AXP_IBOX_OPT2)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
	    "Getting Next vPC[%d] 0x%016llx",
	    prevVPC,
	    *((u64 *) &retVal));
	AXP_TRACE_END()
	;
    }

    /*
     * Return what we found back to the caller.
     */
    return (retVal);
}

/*
 * AXP_21264_IncrementVPC
 * 	This function is called to increment Virtual Program Counter (VPC).
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The value of the incremented PC.
 */
AXP_PC AXP_21264_IncrementVPC(AXP_21264_CPU *cpu)
{
    AXP_PC vpc;

    /*
     * Get the PC for the instruction just executed.
     */
    vpc = AXP_21264_GetNextVPC(cpu);

    /*
     * Increment it.
     */
    vpc.pc++;

    if (AXP_IBOX_OPT2)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("Incremented vPC 0x%016llx", *((u64 *) &vpc));
	AXP_TRACE_END()
	;
    }

    /*
     * Store it on the VPC List and return to the caller.
     */
    return (vpc);
}

/*
 * AXP_21264_DisplaceVPC
 * 	This function is called to add a displacement value to the VPC.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	displacement:
 *		A signed 64-bit value to be added to the VPC.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The value of the PC with the displacement.
 */
AXP_PC AXP_21264_DisplaceVPC(AXP_21264_CPU *cpu, AXP_PC pc, i64 displacement)
{
    AXP_PC vpc = pc;

    /*
     * Increment and then add the displacement.
     */
    vpc.pc = vpc.pc + displacement;

    if (AXP_IBOX_OPT2)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
	    "Displacement vPC 0x%016llx (0x%016llx)",
	    *((u64 *) &vpc),
	    displacement);
	AXP_TRACE_END()
	;
    }

    /*
     * Return back to the caller.
     */
    return (vpc);
}
