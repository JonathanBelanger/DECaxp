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
 *	Integer Byte Manipulation functionality of the Ebox.
 *
 *	Revision History:
 *
 *	V01.000		24-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21264_Ebox_Byte.h"

/*
 * AXP_CMPBGE
 *	This function implements the Compare Byte instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_CMPBGE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8 *Rav = (u8 *) &instr->src1v;
	u8 *Rbv = (u8 *) (instr->useLiteral ? &instr->literal : &instr->src2v);

	/*
	 * Implement the instruction.
	 */
	instr->destv = (Rav[0] >= Rbv[0] ? 0x01 : 0x00) |
				   (Rav[1] >= Rbv[1] ? 0x02 : 0x00) |
				   (Rav[2] >= Rbv[2] ? 0x04 : 0x00) |
				   (Rav[3] >= Rbv[3] ? 0x08 : 0x00) |
				   (Rav[4] >= Rbv[4] ? 0x10 : 0x00) |
				   (Rav[5] >= Rbv[5] ? 0x20 : 0x00) |
				   (Rav[6] >= Rbv[6] ? 0x40 : 0x00) |
				   (Rav[7] >= Rbv[7] ? 0x80 : 0x00);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_EXTBL
 *	This function implements the Extract Byte Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_EXTBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_BYTE_MASK(instr->src1v >> ((Rbv & AXP_LOW_3BITS) * 8));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_EXTWL
 *	This function implements the Extract Word Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_EXTWL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_WORD_MASK(instr->src1v >> ((Rbv & AXP_LOW_3BITS) * 8));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_EXTLL
 *	This function implements the Extract Long Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_EXTLL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_LONG_MASK(instr->src1v >> ((Rbv & AXP_LOW_3BITS) * 8));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_EXTQL
 *	This function implements the Extract Quad Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_EXTQL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_QUAD_MASK(instr->src1v >> ((Rbv & AXP_LOW_3BITS) * 8));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_EXTWH
 *	This function implements the Extract Word High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_EXTWH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		AXP_WORD_MASK(instr->src1v << ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_EXTLH
 *	This function implements the Extract Long High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_EXTLH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		AXP_LONG_MASK(instr->src1v << ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_EXTQH
 *	This function implements the Extract Quad High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_EXTQH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		AXP_QUAD_MASK(instr->src1v << ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_INSBL
 *	This function implements the Insert Byte Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_INSBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_BYTE_MASK(instr->src1v) << ((Rbv & AXP_LOW_3BITS) * 8);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_INSWL
 *	This function implements the Insert Word Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_INSWL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_WORD_MASK(instr->src1v) << ((Rbv & AXP_LOW_3BITS) * 8);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_INSLL
 *	This function implements the Insert Long Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_INSLL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_LONG_MASK(instr->src1v) << ((Rbv & AXP_LOW_3BITS) * 8);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_INSQL
 *	This function implements the Insert Quad Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_INSQL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_QUAD_MASK(instr->src1v) << ((Rbv & AXP_LOW_3BITS) * 8);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_INSWH
 *	This function implements the Insert Word High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_INSWH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		(Rbv & AXP_LOW_3BITS) ?
		(AXP_WORD_MASK(instr->src1v) >> ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS)) :
		0;
	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_INSLH
 *	This function implements the Insert Long High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_INSLH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		(Rbv & AXP_LOW_3BITS) ?
		(AXP_LONG_MASK(instr->src1v) >> ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS)) :
		0;
	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_INSQH
 *	This function implements the Insert Quad High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_INSQH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		(Rbv & AXP_LOW_3BITS) ?
		(AXP_QUAD_MASK(instr->src1v) >> ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS)) :
		0;
	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MSKBL
 *	This function implements the Mask Byte Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_MSKBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = instr->src1v & ~(AXP_LOW_BYTE << ((Rbv & AXP_LOW_3BITS) * 8));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MSKWL
 *	This function implements the Mask Word Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_MSKWL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = instr->src1v & ~(AXP_LOW_WORD << ((Rbv & AXP_LOW_3BITS) * 8));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MSKLL
 *	This function implements the Mask Long Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_MSKLL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = instr->src1v & ~(AXP_LOW_LONG << ((Rbv & AXP_LOW_3BITS) * 8));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MSKQL
 *	This function implements the Mask Quad Low instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_MSKQL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv = instr->src1v & ~(AXP_LOW_QUAD << ((Rbv & AXP_LOW_3BITS) * 8));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MSKWH
 *	This function implements the Mask Word High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_MSKWH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		(Rbv & AXP_LOW_3BITS) ?
		(instr->src1v & ~(AXP_LOW_WORD >> ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS))) :
		instr->src1v;

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MSKLH
 *	This function implements the Mask Long High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_MSKLH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		(Rbv & AXP_LOW_3BITS) ?
		(instr->src1v & ~(AXP_LOW_LONG >> ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS))) :
		instr->src1v;

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MSKQH
 *	This function implements the Mask Quad High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_MSKQH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * If we are executing in big-endian mode, then we need to do some value
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		Rbv = AXP_BIG_ENDIAN_BYTE(Rbv);

	/*
	 * Implement the instruction.
	 */
	instr->destv =
		(Rbv & AXP_LOW_3BITS) ?
		(instr->src1v & ~(AXP_LOW_QUAD >> ((64 - ((Rbv & AXP_LOW_3BITS) * 8)) & AXP_LOW_6BITS))) :
		instr->src1v;

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_SEXTB
 *	This function implements the Sign Extend Byte instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_SEXTB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_SEXT_BYTE(Rbv);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_SEXTW
 *	This function implements the Sign Extend Word instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_SEXTW(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction.
	 */
	instr->destv = AXP_SEXT_WORD(Rbv);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_ZAP
 *	This function implements the Zero Bytes instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_ZAP(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8 *Rbv = (u8 *) (instr->useLiteral ? &instr->literal : &instr->src2v);

	/*
	 * Implement the instruction.
	 */
	instr->destv = instr->src1v &
			(((Rbv[0] & 0x01) ? 0 : 0x00000000000000ffll) |
			 ((Rbv[0] & 0x02) ? 0 : 0x000000000000ff00ll) |
			 ((Rbv[0] & 0x04) ? 0 : 0x0000000000ff0000ll) |
			 ((Rbv[0] & 0x08) ? 0 : 0x00000000ff000000ll) |
			 ((Rbv[0] & 0x10) ? 0 : 0x000000ff00000000ll) |
			 ((Rbv[0] & 0x20) ? 0 : 0x0000ff0000000000ll) |
			 ((Rbv[0] & 0x40) ? 0 : 0x00ff000000000000ll) |
			 ((Rbv[0] & 0x80) ? 0 : 0xff00000000000000ll));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_ZAPNOT
 *	This function implements the Zero Bytes Not instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_ZAPNOT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8 *Rbv = (u8 *) (instr->useLiteral ? &instr->literal : &instr->src2v);

	/*
	 * Implement the instruction.
	 */
	instr->destv = instr->src1v &
			(((Rbv[0] & 0x01) ? 0x00000000000000ffll : 0) |
			 ((Rbv[0] & 0x02) ? 0x000000000000ff00ll : 0) |
			 ((Rbv[0] & 0x04) ? 0x0000000000ff0000ll : 0) |
			 ((Rbv[0] & 0x08) ? 0x00000000ff000000ll : 0) |
			 ((Rbv[0] & 0x10) ? 0x000000ff00000000ll : 0) |
			 ((Rbv[0] & 0x20) ? 0x0000ff0000000000ll : 0) |
			 ((Rbv[0] & 0x40) ? 0x00ff000000000000ll : 0) |
			 ((Rbv[0] & 0x80) ? 0xff00000000000000ll : 0));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
