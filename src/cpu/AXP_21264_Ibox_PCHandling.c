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
 *	functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		10-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		14-May-2017	Jonathan D. Belanger
 *	Cleaned up some compiler errors.  Ran some tests, and corrected some coding
 *	mistakes.  The prediction code correctly predicts a branch instruction
 *	between 95.0% and 99.1% of the time.
 *
 *	V01.002		20-May-2017	Jonathan D. Belanger
 *	Did a bit of reorganization.  This module will contain all the
 *	functionality that is described as being part of the Ibox.  I move the
 *	prediction code here.
 *
 *	V01.003		22-May-2017	Jonathan D. Belanger
 *	Moved the main() function out of this module and into its own.
 *
 *	V01.004		24-May-2017	Jonathan D. Belanger
 *	Added some instruction cache (icache) code and definitions into this code.
 *	The iCache structure should be part of the CPU structure and the iCache
 *	code should not be adding a cache entry on a miss.  There should be a
 *	separate fill cache function.  Also, the iCache look-up code should return
 *	the requested instructions on a hit.
 *
 *	V01.005		01-Jun-2017	Jonathan D. Belanger
 *	Added a function to add an Icache line/block.  There are a couple to do
 *	items for ASM and ASN in this function.
 *
 *	V01.006		11-Jun-2017	Jonathan D. Belanger
 *	Started working on the instruction decode and register renaming code.
 *
 *	V01.007		12-Jun-2017	Jonathan D. Belanger
 *	Forgot to include the generation of a unique ID (8-bits long) to associate
 *	with each decoded instruction.
 *	BTW: We need a better way to decode an instruction.  The decoded
 *	instruction does not require the architectural register number, so we
 *	should not save them.  Also, renaming can be done while we are decoding the
 *	instruction.  We are also going to have to special case some instructions,
 *	such as PAL instructions, CMOVE, and there are exceptions for instruction
 *	types (Ra is a destination register for a Load, but a source register for a
 *	store.
 *
 *	V01.008		15-Jun-2017	Jonathan D. Belanger
 *	Finished defining structure array to be able to normalize how a register is
 *	used in an instruction.  Generally speaking, register 'c' (Rc or Fc) is a
 *	destination register.  In load operations, register 'a' (Ra or Fa) is the
 *	destination register.  Register 'b' (Rb or Fb) is never used as a
 *	designation (through I have implemented provisions for this).
 *	Unfortunately, sometimes a register is not used, or 'b' may be source 1 and
 *	other times it is source 2.  Sometimes not register is utilized (e.g. MB).
 *	The array structures assist in these differences so that in the end we end
 *	up with a destination (not not), a source 1 (or not), and/or a source 2
 *	(or not) set of registers.  If we have a destination register, then
 *	register renaming needs to allocate a new register.  IF we have source
 *	register, then renaming needs to associate the current register mapping to
 *	this register.
 *
 *	V01.009		16-Jun-2017	Jonathan D. Belanger
 *	Finished the implementation of normalizing registers, as indicated above,
 *	and then mapping them from architectural to physical registers, generating
 *	a new mapping for the destination register.
 *
 *	V01.010		23-Jun-2017	Jonathan D. Belanger
 *	The way I'm handling the Program Counter (PC/VPC) is not correct.  When
 *	instructions are decoded, queued, and executed, their results are not
 *	realized until the instruction is retired.  The Integer Control
 *	instructions currently determine the next PC and set it immediately. not
 *	are retirement.  This is because I tried to do to much in the code that
 *	handles the PC/VPC.  The next PC should only be updated as a part of normal
 *	instruction queuing or instruction retirement.  We should only have one
 *	way to "set" the PC, but other functions to calculate one, based on various
 *	criteria.
 *
 *	V01.011		18-Jul-2017	Jonathan D. Belanger
 *	Updated the instruction decoding code to replace instruction indicated
 *	registers for PALshadow registers when we are in, or going to be in,
 *	PALmode.  Also, do not include floating point registers, as there are no
 *	PALshadow registers for these.
 *
 *	V01.012		15-Jan-2018	Jonathan D. Belanger
 *	Move the instruction decoding functions to their own module.  This was
 *	getting too large for eclipse to handle without crashing.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ibox.h"

/*
 * A local structure used to calculate the PC for a CALL_PAL function.
 */
struct palBaseBits21264
{
	u64	res : 15;
	u64	highPC : 49;
};
struct palBaseBits21164
{
	u64	res : 14;
	u64	highPC : 50;
};

typedef union
{
	 struct palBaseBits21164	bits21164;
	 struct palBaseBits21264	bits21264;
	 u64						palBaseAddr;

}AXP_IBOX_PALBASE_BITS;

struct palPCBits21264
{
	u64	palMode : 1;
	u64 mbz_1 : 5;
	u64 func_5_0 : 6;
	u64 func_7 : 1;
	u64 mbo : 1;
	u64 mbz_2 : 1;
	u64 highPC : 49;
};
struct palPCBits21164
{
	u64	palMode : 1;
	u64 mbz : 5;
	u64 func_5_0 : 6;
	u64 func_7 : 1;
	u64 mbo : 1;
	u64 highPC : 50;
};

typedef union
{
	struct palPCBits21164	bits21164;
	struct palPCBits21264	bits21264;
	AXP_PC					vpc;
} AXP_IBOX_PAL_PC;

struct palFuncBits
{
	u32 func_5_0 : 6;
	u32 res_1 : 1;
	u32 func_7 : 1;
	u32 res_2 : 24;
};

typedef union
{
	struct palFuncBits	bits;
	u32					func;
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

	/*
	 * Return the composed VPC it back to the caller.
	 */
	return(pc.vpc);
}

/*
 * AXP_21264_GetPALBaseVPC
 * 	This function is called to get the Virtual Program Counter (VPC) to a
 * 	specific offset from the address specified in the PAL_BASE register.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	offset:
 *		An offset value, from PAL_BASE, of the next VPC to be entered into the
 *		VPC list.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The value that the PC should be set to to call the requested offset.
 */
AXP_PC AXP_21264_GetPALBaseVPC(AXP_21264_CPU *cpu, u64 offset)
{
	u64 pc;

	pc = cpu->palBase.pal_base_pc + offset;

	/*
	 * Get the VPC set with the correct PALmode bit and return it back to the
	 * caller.
	 */
	return(AXP_21264_GetVPC(cpu, pc, AXP_PAL_MODE));
}

/*
 * AXP_21264_GetVPC
 * 	This function is called to get the Virtual Program Counter (VPC) to a
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
AXP_PC AXP_21264_GetVPC(AXP_21264_CPU *cpu, u64 pc, u8 pal)
{
	union
	{
		u64		pc;
		AXP_PC	vpc;
	} vpc;

	vpc.pc = pc;
	vpc.vpc.res = 0;
	vpc.vpc.pal = pal & AXP_PAL_MODE;

	/*
	 * Return back to the caller.
	 */
	return(vpc.vpc);
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
	u32	prevVPC;

	/*
	 * The End, points to the next location to be filled.  Therefore, the
	 * previous location is the next VPC to be executed.
	 */
	prevVPC = ((cpu->vpcEnd != 0) ? cpu->vpcEnd : AXP_INFLIGHT_MAX) - 1;
	retVal = cpu->vpc[prevVPC];

	/*
	 * Return what we found back to the caller.
	 */
	return(retVal);
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

	/*
	 * Store it on the VPC List and return to the caller.
	 */
	return(vpc);
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
AXP_PC AXP_21264_DisplaceVPC(AXP_21264_CPU *cpu, i64 displacement)
{
	AXP_PC vpc;

	/*
	 * Get the PC for the instruction just executed.
	 */
	vpc = AXP_21264_GetNextVPC(cpu);

	/*
	 * Increment and then add the displacement.
	 */
	vpc.pc = vpc.pc + 1 + displacement;

	/*
	 * Return back to the caller.
	 */
	return(vpc);
}
