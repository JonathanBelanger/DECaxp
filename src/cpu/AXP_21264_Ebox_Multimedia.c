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
 *	Multimedia instructions of the 21264 Alpha AXP CPU.
 *
 *	Revision History:
 *
 *	V01.000		19-Jul-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ebox_Multimedia.h"

/*
 * AXP_MINUB8
 *	This function implements the Minimum Unsigned Byte instruction of the Alpha
 *	AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MINUB8(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8	*Rav = (u8 *) &instr->src1v.r.uq;
	u8	*Rbv = (u8 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	u8	*Rcv = (u8 *) &instr->destv.r.uq;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(u64)/sizeof(u8)); ii++)
		Rcv[ii] = Rav[ii] < Rbv[ii] ? Rav[ii] : Rbv[ii];

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
 * AXP_MINUS8
 *	This function implements the Minimum Signed Byte instruction of the Alpha
 *	AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MINSB8(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	i8	*Rav = (i8 *) &instr->src1v.r.uq;
	i8	*Rbv = (i8 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	i8	*Rcv = (i8 *) &instr->destv.r.uq;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(i64)/sizeof(i8)); ii++)
		Rcv[ii] = Rav[ii] < Rbv[ii] ? Rav[ii] : Rbv[ii];

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
 * AXP_MINUW4
 *	This function implements the Minimum Unsigned Word instruction of the Alpha
 *	AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MINUW4(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u16	*Rav = (u16 *) &instr->src1v.r.uq;
	u16	*Rbv = (u16 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	u16	*Rcv = (u16 *) &instr->destv.r.uq;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(u64)/sizeof(u16)); ii++)
		Rcv[ii] = Rav[ii] < Rbv[ii] ? Rav[ii] : Rbv[ii];

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
 * AXP_MINSW4
 *	This function implements the Minimum Signed Word instruction of the Alpha
 *	AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MINSW4(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	i16	*Rav = (i16 *) &instr->src1v.r.uq;
	i16	*Rbv = (i16 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	i16	*Rcv = (i16 *) &instr->destv.r.uq;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(i64)/sizeof(i16)); ii++)
		Rcv[ii] = Rav[ii] < Rbv[ii] ? Rav[ii] : Rbv[ii];

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
 * AXP_MAXUB8
 *	This function implements the Maximum Unsigned Byte instruction of the Alpha
 *	AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MAXUB8(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8	*Rav = (u8 *) &instr->src1v.r.uq;
	u8	*Rbv = (u8 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	u8	*Rcv = (u8 *) &instr->destv.r.uq;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(u64)/sizeof(u8)); ii++)
		Rcv[ii] = Rav[ii] > Rbv[ii] ? Rav[ii] : Rbv[ii];

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
 * AXP_MAXSB8
 *	This function implements the Maximum Signed Byte instruction of the Alpha
 *	AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MAXSB8(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	i8	*Rav = (i8 *) &instr->src1v.r.uq;
	i8	*Rbv = (i8 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	i8	*Rcv = (i8 *) &instr->destv.r.uq;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(i64)/sizeof(i8)); ii++)
		Rcv[ii] = Rav[ii] > Rbv[ii] ? Rav[ii] : Rbv[ii];

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
 * AXP_MAXUW4
 *	This function implements the Maximum Unsigned Word instruction of the Alpha
 *	AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MAXUW4(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u16	*Rav = (u16 *) &instr->src1v.r.uq;
	u16	*Rbv = (u16 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	u16	*Rcv = (u16 *) &instr->destv.r.uq;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(u64)/sizeof(u16)); ii++)
		Rcv[ii] = Rav[ii] > Rbv[ii] ? Rav[ii] : Rbv[ii];

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
 * AXP_MAXSW4
 *	This function implements the Maximum Signed Word instruction of the Alpha
 *	AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MAXSW4(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	i16	*Rav = (i16 *) &instr->src1v.r.uq;
	i16	*Rbv = (i16 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	i16	*Rcv = (i16 *) &instr->destv.r.uq;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(i64)/sizeof(i16)); ii++)
		Rcv[ii] = Rav[ii] > Rbv[ii] ? Rav[ii] : Rbv[ii];

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
 * AXP_PERR
 *	This function implements the Pixel Error instruction of the Alpha AXP
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_PERR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8	*Rav = (u8 *) &instr->src1v.r.uq;
	u8	*Rbv = (u8 *) (instr->useLiteral ? &instr->literal : &instr->src2v.r.uq);
	u64	temp = 0;
	int	ii;

	/*
	 * Execute the instruction.
	 */
	for (ii = 0; ii < (sizeof(u64)/sizeof(u8)); ii++)
		temp += abs(Rav[ii] - Rbv[ii]);
	instr->destv.r.uq = temp;

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
 * AXP_PKLB
 *	This function implements the Pack Longwords to Bytes instruction of the
 *	Alpha AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_PKLB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8	*Rbv = (u8 *) &instr->src2v.r.uq;
	u8	*Rcv = (u8 *) &instr->destv.r.uq;

	/*
	 * Execute the instruction.
	 */
	instr->destv.r.uq = 0;
	Rcv[0] = Rbv[0];
	Rcv[1] = Rbv[8];

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
 * AXP_PKWB
 *	This function implements the Pack Longwords to Bytes instruction of the
 *	Alpha AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_PKWB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8	*Rbv = (u8 *) &instr->src2v.r.uq;
	u8	*Rcv = (u8 *) &instr->destv.r.uq;

	/*
	 * Execute the instruction.
	 */
	instr->destv.r.uq = 0;
	Rcv[0] = Rbv[0];
	Rcv[1] = Rbv[2];
	Rcv[2] = Rbv[4];
	Rcv[3] = Rbv[8];

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
 * AXP_UNPKBL
 *	This function implements the Unpack Bytes to Longwords instruction of the
 *	Alpha AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_UNPKBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8	*Rbv = (u8 *) &instr->src2v.r.uq;
	u8	*Rcv = (u8 *) &instr->destv.r.uq;

	/*
	 * Execute the instruction.
	 */
	instr->destv.r.uq = 0;
	Rcv[0] = Rbv[0];
	Rcv[8] = Rbv[1];

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
 * AXP_UNPKBW
 *	This function implements the Unpack Bytes to Words instruction of the
 *	Alpha AXP processor.
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
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_UNPKBW(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u8	*Rbv = (u8 *) &instr->src2v.r.uq;
	u8	*Rcv = (u8 *) &instr->destv.r.uq;

	/*
	 * Execute the instruction.
	 */
	instr->destv.r.uq = 0;
	Rcv[0] = Rbv[0];
	Rcv[2] = Rbv[1];
	Rcv[3] = Rbv[2];
	Rcv[6] = Rbv[3];

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
