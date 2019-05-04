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
 *	This file contains the utility functions needed throughout the Alpha AXP
 *	emulation code.
 *
 * Revision History:
 *
 *	V01.000		29-May-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Exceptions.h"

/*
 * AXP_SetException
 * 	This function will be utilized throughout the code to set the exception
 * 	bits in the Instruction structure.  When the instruction is retired, this
 * 	information will be used to determine the next course of action.  If
 * 	another instruction that generated an exception retires before the one we
 * 	are currently processing, then this information will be flushed along with
 * 	any subsequent set of instructions.
 *
 * Input Parameters:
 * 	instr:
 * 		A pointer to a structure containing the information needed to report
 * 		the exception upon instruction retirement.
 * 	exception:
 * 		A value indicating the bit fields to set in the exception summary
 * 		register.
 *
 * Output Parameters:
 * 	instr:
 * 		The excSum and excRegMask will be updated, based on the exception and
 * 		the instruction (FP vs others).
 *
 * Return Value:
 * 	None.
 */
void AXP_SetException(AXP_INSTRUCTION *instr, u32 exception)
{

    /*
     * First, set the exception summary register
     */
    if (exception & AXP_EXC_SW_COMPL)
  instr->excSum.swc = 1;
    if (exception & AXP_EXC_INV_OPER)
  instr->excSum.inv = 1;
    if (exception & AXP_EXC_DIV_BY_ZERO)
  instr->excSum.dze = 1;
    if (exception & AXP_EXC_FP_OVERFLOW)
  instr->excSum.ovf = 1;
    if (exception & AXP_EXC_UNDERFLOW)
  instr->excSum.unf = 1;
    if (exception & AXP_EXC_INEXACT_RES)
  instr->excSum.ine = 1;
    if (exception & AXP_EXC_INT_OVERFLOW)
  instr->excSum.iov = 1;

    /*
     * If this was the result of a floating-point operation, also set both the
     * set_* fields in the excSum and the correct register mask.
     */
    if (((instr->opcode >= ITFP) && (instr->opcode <= FLTL))
      || ((instr->opcode >= LDF) && (instr->opcode <= STT))
      || ((instr->opcode >= FBEQ) && (instr->opcode <= FBGT)
              && (instr->opcode != BSR))
      || ((instr->opcode == FPTI)
              && ((instr->function == AXP_FUNC_FTOIT)
                      || (instr->function == AXP_FUNC_FTOIS))))
    {
  if (exception & AXP_EXC_INV_OPER)
      instr->excSum.set_inv = 1;
  ;
  if (exception & AXP_EXC_DIV_BY_ZERO)
      instr->excSum.set_dze = 1;
  if (exception & AXP_EXC_FP_OVERFLOW)
      instr->excSum.set_ovf = 1;
  if (exception & AXP_EXC_UNDERFLOW)
      instr->excSum.set_unf = 1;
  if (exception & AXP_EXC_INEXACT_RES)
      instr->excSum.set_ine = 1;
  if (exception & AXP_EXC_INT_OVERFLOW)
  {
      instr->excSum.set_iov = 1;
      instr->excSum.sext_set_iov = 0xffff; /* Sign-extend set_iov */
  }

  /*
   * Set the correct FP register mask (destination).
   */
  instr->excRegMask = 0x100000000ll << instr->aDest;
    }
    else
    {

  /*
   * Set the correct register mask (destination).
   */
  instr->excRegMask = 0x1 << instr->aDest;
    }

    /*
     * Return back to the caller.
     */
    return;
}
