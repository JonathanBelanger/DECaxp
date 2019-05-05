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
 *	This source file contains the functions needed to implement the instruction
 *	decoding functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		15-Jan-2018	Jonathan D. Belanger
 *	Initially written from functions originally defined in AXP_21264_Ibox.c.
 *
 *	V01.001		11-Mar-2018	Jonathan D. Belanger
 *	Changed the way the len_stall flag worked.  It is now 2 flags.  The stall
 *	flag will be used to instruct the Ibox to stall until the IQ and FQ or the
 *	HW_RET_STALL instruction has been retired.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "CPU/Ibox/AXP_21264_Ibox.h"
#include "CPU/Ibox/AXP_21264_Ibox_InstructionDecoding.h"
#include "CommonUtilities/AXP_Trace.h"

/*
 * Prototypes for local functions
 */
static AXP_OPER_TYPE AXP_DecodeOperType(u8, u32);

/* Functions to decode which instruction registers are used for which purpose
 * for the complex instructions (opcodes that have different ways to use
 * registers).
 */
static u16 AXP_RegisterDecodingOpcode11(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode14(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode15_16(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode17(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode18(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode1c(AXP_INS_FMT);
static void AXP_RenameRegisters(AXP_21264_CPU *, AXP_INSTRUCTION *);

/*
 * The following module specific structure and variable are used to be able to
 * decode opcodes that have differing ways register are utilized.  The index
 * into this array is the opcodeRegDecode fields in the bits field of the
 * register mapping in the above table.  There is no entry [0], so that is
 * just NULL and should never get referenced.
 */
typedef u16 (*regDecodeFunc)(AXP_INS_FMT);
regDecodeFunc decodeFuncs[] =
{
    NULL,
    AXP_RegisterDecodingOpcode11,
    AXP_RegisterDecodingOpcode14,
    AXP_RegisterDecodingOpcode15_16,
    AXP_RegisterDecodingOpcode15_16,
    AXP_RegisterDecodingOpcode17,
    AXP_RegisterDecodingOpcode18,
    AXP_RegisterDecodingOpcode1c
};
static char *regStateStr[] =
{
"Free", "Pending Update", "Valid"
};

/*
 * AXP_Decode_Rename
 * 	This function is called to take a set of 4 instructions and decode them
 * 	and then rename the architectural registers to physical ones.  The results
 * 	are put onto either the Integer Queue or Floating-point Queue (FQ) for
 * 	execution.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to
 *		emulate an Alpha AXP 21264 CPU.
 *	next:
 *		A pointer to a location to receive the next 4 instructions to be
 *		processed.
 *	nextInstr:
 *		A value indicating which of the 4 instructions is to be processed.
 *
 * Output Parameters:
 * 	decodedInsr:
 * 		A pointer to the decoded version of the instruction.
 * 	pipeline:
 * 		A pointer to a location to receive the pipelines this instruction is
 * 		allowed to execute.
 *
 * Return value:
 * 	None.
 */
void AXP_Decode_Rename(
    AXP_21264_CPU *cpu,
    AXP_INS_LINE *next,
    int nextInstr,
    AXP_INSTRUCTION *decodedInstr,
    AXP_PIPELINE *pipeline)
{
    bool callingPAL = false;
    bool src1Float = false;
    bool src2Float = false;
    bool destFloat = false;
    u32 function;

    /*
     * Decode the next instruction.
     *
     * First, Assign a unique ID to this instruction (the counter should
     * auto-wrap) and initialize some of the other fields within the decoded
     * instruction.
     */
    decodedInstr->uniqueID = cpu->instrCounter++;
    decodedInstr->excRegMask = NoException;

    /*
     * Let's, decode the instruction.
     */
    decodedInstr->instr.instr = next->instructions[nextInstr].instr;
    decodedInstr->format = next->instrType[nextInstr];
    decodedInstr->opcode = next->instructions[nextInstr].pal.opcode;
    decodedInstr->stall = false;
    switch (decodedInstr->format)
    {
  case Bra:
  case FPBra:
      decodedInstr->displacement = next->instructions[nextInstr].br
    .branch_disp;
      break;

  case FP:
      decodedInstr->function = next->instructions[nextInstr].fp.func;
      break;

  case Mem:
  case Mbr:
      decodedInstr->displacement = next->instructions[nextInstr].mem.mem
    .disp;
      decodedInstr->stall = ((decodedInstr->opcode == STL_C)
    || (decodedInstr->opcode == STQ_C));
      break;

  case Mfc:
      decodedInstr->function = next->instructions[nextInstr].mem.mem.func;
      decodedInstr->stall = ((decodedInstr->opcode == MISC)
    && (decodedInstr->function == AXP_FUNC_MB));
      break;

  case Opr:
      decodedInstr->function = next->instructions[nextInstr].oper1.func;
      decodedInstr->useLiteral = next->instructions[nextInstr].oper1.fmt
    == 1;
      break;

  case Pcd:
      decodedInstr->function = next->instructions[nextInstr].pal
    .palcode_func;
      callingPAL = true;
      break;

  case PAL:
      switch (decodedInstr->opcode)
      {
    case HW_LD:
    case HW_ST:
        decodedInstr->displacement = next->instructions[nextInstr]
      .hw_ld.disp;
        decodedInstr->type_hint_index =
      next->instructions[nextInstr].hw_ld.type;
        decodedInstr->quadword = (next->instructions[nextInstr]
      .hw_ld.len == 1);
        break;

    case HW_RET:
        decodedInstr->displacement = next->instructions[nextInstr]
      .hw_ret.disp;
        decodedInstr->type_hint_index =
      next->instructions[nextInstr].hw_ret.hint;
        decodedInstr->stall = (next->instructions[nextInstr].hw_ret
      .stall == 1);
        break;

    case HW_MFPR:
    case HW_MTPR:
        decodedInstr->type_hint_index =
      next->instructions[nextInstr].hw_mxpr.index;
        decodedInstr->scbdMask = next->instructions[nextInstr]
      .hw_mxpr.scbd_mask;
        break;

    default:
        break;
      }
      break;

  default:
      break;
    }
    decodedInstr->type = AXP_OperationType(decodedInstr->opcode);
    if ((decodedInstr->type == Other) && (decodedInstr->format != Res))
  decodedInstr->type = AXP_DecodeOperType(
      decodedInstr->opcode,
      decodedInstr->function);
    decodedInstr->decodedReg = AXP_RegisterDecoding(decodedInstr->opcode);
    if (decodedInstr->decodedReg.bits.opcodeRegDecode != 0)
  decodedInstr->decodedReg.raw = decodeFuncs[decodedInstr->decodedReg.bits
      .opcodeRegDecode](next->instructions[nextInstr]);
    if ((decodedInstr->opcode == HW_MFPR) || (decodedInstr->opcode == HW_MFPR))
  function = decodedInstr->type_hint_index;
    else
  function = decodedInstr->function;
    *pipeline = AXP_InstructionPipeline(decodedInstr->opcode, function);

    /*
     * Decode destination register
     */
    switch (decodedInstr->decodedReg.bits.dest)
    {
  case AXP_REG_RA:
      decodedInstr->aDest = next->instructions[nextInstr].oper1.ra;
      break;

  case AXP_REG_RB:
      decodedInstr->aDest = next->instructions[nextInstr].oper1.rb;
      break;

  case AXP_REG_RC:
      decodedInstr->aDest = next->instructions[nextInstr].oper1.rc;
      break;

  case AXP_REG_FA:
      decodedInstr->aDest = next->instructions[nextInstr].fp.fa;
      destFloat = true;
      break;

  case AXP_REG_FB:
      decodedInstr->aDest = next->instructions[nextInstr].fp.fb;
      destFloat = true;
      break;

  case AXP_REG_FC:
      decodedInstr->aDest = next->instructions[nextInstr].fp.fc;
      destFloat = true;
      break;

  default:

      /*
       *  If the instruction being decoded is a CALL_PAL, then there is a
       *  linkage register (basically a return address after the CALL_PAL
       *  has completed).  For Jumps, the is usually specified in the
       *  register fields of the instruction.  For CALL_PAL, this is
       *  either R23 or R27, depending upon the setting of the
       *  call_pal_r23 in the I_CTL IPR.
       */
      if (decodedInstr->opcode == PAL00)
      {
    if (cpu->iCtl.call_pal_r23 == 1)
        decodedInstr->aDest = 23;
    else
        decodedInstr->aDest = 27;
      }
      else
    decodedInstr->aDest = AXP_UNMAPPED_REG;
      break;
    }

    /*
     * Decode source1 register
     */
    switch (decodedInstr->decodedReg.bits.src1)
    {
  case AXP_REG_RA:
      decodedInstr->aSrc1 = next->instructions[nextInstr].oper1.ra;
      break;

  case AXP_REG_RB:
      decodedInstr->aSrc1 = next->instructions[nextInstr].oper1.rb;
      break;

  case AXP_REG_RC:
      decodedInstr->aSrc1 = next->instructions[nextInstr].oper1.rc;
      break;

  case AXP_REG_FA:
      decodedInstr->aSrc1 = next->instructions[nextInstr].fp.fa;
      src1Float = true;
      break;

  case AXP_REG_FB:
      decodedInstr->aSrc1 = next->instructions[nextInstr].fp.fb;
      src1Float = true;
      break;

  case AXP_REG_FC:
      decodedInstr->aSrc1 = next->instructions[nextInstr].fp.fc;
      src1Float = true;
      break;

  default:
      decodedInstr->aSrc1 = AXP_UNMAPPED_REG;
      break;
    }

    /*
     * Decode source2 register
     */
    switch (decodedInstr->decodedReg.bits.src2)
    {
  case AXP_REG_RA:
      decodedInstr->aSrc2 = next->instructions[nextInstr].oper1.ra;
      break;

  case AXP_REG_RB:
      if (decodedInstr->useLiteral == true)
      {
    decodedInstr->literal = next->instructions[nextInstr].oper2.lit;
    decodedInstr->aSrc2 = AXP_UNMAPPED_REG;
      }
      else
    decodedInstr->aSrc2 = next->instructions[nextInstr].oper1.rb;
      break;

  case AXP_REG_RC:
      decodedInstr->aSrc2 = next->instructions[nextInstr].oper1.rc;
      break;

  case AXP_REG_FA:
      decodedInstr->aSrc2 = next->instructions[nextInstr].fp.fa;
      src2Float = true;
      break;

  case AXP_REG_FB:
      decodedInstr->aSrc2 = next->instructions[nextInstr].fp.fb;
      src2Float = true;
      break;

  case AXP_REG_FC:
      decodedInstr->aSrc2 = next->instructions[nextInstr].fp.fc;
      src2Float = true;
      break;

  default:
      decodedInstr->aSrc2 = AXP_UNMAPPED_REG;
      break;
    }

    /*
     * When running in PALmode, the shadow registers may come into play.  If we
     * are in PALmode, then the PALshadow registers may come into play.  If so,
     * we need to replace the specified register with the PALshadow one.
     *
     * There is no such thing as Floating Point PALshadow registers, so the
     * register specified on the instruction is the one to use.  Now need to
     * check.
     */
    decodedInstr->pc = next->instrPC[nextInstr];
    callingPAL |= (decodedInstr->pc.pal == AXP_PAL_MODE);
    if (src1Float == false)
  decodedInstr->aSrc1 = AXP_REG(decodedInstr->aSrc1, callingPAL);
    if (src2Float == false)
  decodedInstr->aSrc2 = AXP_REG(decodedInstr->aSrc2, callingPAL);
    if (destFloat == false)
  decodedInstr->aDest = AXP_REG(decodedInstr->aDest, callingPAL);

    /*
     * We need to rename the architectural registers to physical
     * registers, now that we know which one, if any, is the
     * destination register and which one(s) is(are) the source
     * register(s).
     */
    AXP_RenameRegisters(cpu, decodedInstr);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DecodeOperType
 * 	This function is called to convert an operation type of 'Other' to a more
 * 	usable value.  The opcode and funcCode are used in combination to determine
 * 	the operation type.
 *
 * Input Parameters:
 * 	opCode:
 * 		A byte value specifying the opcode to be used to determine the
 * 		operation type.
 * 	funcCode:
 * 		A 32-bit value specifying the function code used to determine the
 * 		operation type.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	The operation type value for the opCode/FuncCode pair.
 */
static AXP_OPER_TYPE AXP_DecodeOperType(u8 opCode, u32 funcCode)
{
    AXP_OPER_TYPE retVal = Other;

    switch (opCode)
    {
  case INTA: /* OpCode == 0x10 */
      if (funcCode == AXP_FUNC_CMPBGE)
    retVal = Logic;
      else
    retVal = Arith;
      break;

  case INTL: /* OpCode == 0x11 */
      if ((funcCode == AXP_FUNC_AMASK) || (funcCode == AXP_FUNC_IMPLVER))
    retVal = Oper;
      else
    retVal = Logic;
      break;

  case FLTV: /* OpCode == 0x15 */
      if ((funcCode == AXP_FUNC_CMPGEQ) || (funcCode == AXP_FUNC_CMPGLT)
    || (funcCode == AXP_FUNC_CMPGLE)
    || (funcCode == AXP_FUNC_CMPGEQ_S)
    || (funcCode == AXP_FUNC_CMPGLT_S)
    || (funcCode == AXP_FUNC_CMPGLE_S))
    retVal = Logic;
      else
    retVal = Arith;
      break;

  case FLTI: /* OpCode == 0x16 */
      if ((funcCode == AXP_FUNC_CMPTUN) || (funcCode == AXP_FUNC_CMPTEQ)
    || (funcCode == AXP_FUNC_CMPTLT)
    || (funcCode == AXP_FUNC_CMPTLE)
    || (funcCode == AXP_FUNC_CMPTUN_SU)
    || (funcCode == AXP_FUNC_CMPTEQ_SU)
    || (funcCode == AXP_FUNC_CMPTLT_SU)
    || (funcCode == AXP_FUNC_CMPTLE_SU))
    retVal = Logic;
      else
    retVal = Arith;
      break;

  case FLTL: /* OpCode == 0x17 */
      if (funcCode == AXP_FUNC_MT_FPCR)
    retVal = Load;
      else if (funcCode == AXP_FUNC_MF_FPCR)
    retVal = Store;
      else
    retVal = Arith;
      break;

  case MISC: /* OpCode == 0x18 */
      if ((funcCode == AXP_FUNC_RPCC) || (funcCode == AXP_FUNC_RC)
    || (funcCode == AXP_FUNC_RS))
    retVal = Load;
      else
    retVal = Store;
      break;
    }

    /*
     * Return back to the caller with what we determined.
     */
    return (retVal);
}

/*
 * AXP_RegisterDecodingOpcode11
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x11.
 *
 * Input Parameters:
 * 	instr:
 * 		A value of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode11(AXP_INS_FMT instr)
{
    u16 retVal = 0;

    switch (instr.oper1.func)
    {
  case 0x61: /* AMASK */
      retVal = AXP_DEST_RC | AXP_SRC1_RB;
      break;

  case 0x6c: /* IMPLVER */
      retVal = AXP_DEST_RC;
      break;

  default: /* All others */
      retVal = AXP_DEST_RC | AXP_SRC1_RA | AXP_SRC2_RB;
      break;
    }
    return (retVal);
}

/*
 * AXP_RegisterDecodingOpcode14
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x14.
 *
 * Input Parameters:
 * 	instr:
 * 		A value of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode14(AXP_INS_FMT instr)
{
    u16 retVal = AXP_DEST_FC;

    if ((instr.oper1.func & 0x00f) != 0x004)
  retVal |= AXP_SRC1_FB;
    else
  retVal |= AXP_SRC1_RB;
    return (retVal);
}

/*
 * AXP_RegisterDecodingOpcode15_16
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcodes associated with this instruction
 * 	are 0x15 and 0x16.
 *
 * Input Parameters:
 * 	instr:
 * 		A value of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode15_16(AXP_INS_FMT instr)
{
    u16 retVal = AXP_DEST_FC;

    if ((instr.fp.func & 0x008) == 0)
  retVal |= (AXP_SRC1_FA | AXP_SRC2_FB);
    else
  retVal |= AXP_SRC1_FB;
    return (retVal);
}

/*
 * AXP_RegisterDecodingOpcode17
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x17.
 *
 * Input Parameters:
 * 	instr:
 * 		A value of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode17(AXP_INS_FMT instr)
{
    u16 retVal = 0;

    switch (instr.fp.func)
    {
  case 0x010:
  case 0x030:
  case 0x130:
  case 0x530:
      retVal = AXP_DEST_FC | AXP_SRC1_FB;
      break;

  case 0x024:
      retVal = AXP_DEST_FA;
      break;

  case 0x025:
      retVal = AXP_SRC1_FA;
      break;

  default: /* All others */
      retVal = AXP_DEST_FC | AXP_SRC1_FA | AXP_SRC2_FB;
      break;
    }
    return (retVal);
}

/*
 * AXP_RegisterDecodingOpcode18
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x18.
 *
 * Input Parameters:
 * 	instr:
 * 		A value of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode18(AXP_INS_FMT instr)
{
    u16 retVal = 0;

    if ((instr.mem.mem.func & 0x8000) != 0)
    {
  if ((instr.mem.mem.func == 0xc000) || (instr.mem.mem.func == 0xe000)
      || (instr.mem.mem.func == 0xf000))
      retVal = AXP_DEST_RA;
  else
      retVal = AXP_SRC1_RB;
    }
    return (retVal);
}

/*
 * AXP_RegisterDecodingOpcode1c
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x1c.
 *
 * Input Parameters:
 * 	instr:
 * 		A value of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode1c(AXP_INS_FMT instr)
{
    u16 retVal = AXP_DEST_RC;

    switch (instr.oper1.func)
    {
  case 0x31:
  case 0x37:
  case 0x38:
  case 0x39:
  case 0x3a:
  case 0x3b:
  case 0x3c:
  case 0x3d:
  case 0x3e:
  case 0x3f:
      retVal |= (AXP_SRC1_RA | AXP_SRC2_RB);
      break;

  case 0x70:
  case 0x78:
      retVal |= AXP_SRC1_FA;
      break;

  default: /* All others */
      retVal |= AXP_SRC1_RB;
      break;
    }
    return (retVal);
}

/*
 * AXP_RenameRegisters
 *	This function is called to map the instruction registers from architectural
 *	to physical ones.  For the destination register, we get the next one off
 *	the free list.  We also differentiate between integer and floating point
 *	registers at this point (previously, we just noted it).
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	decodedInstr:
 *		A pointer to the structure containing a decoded representation of the
 *		Alpha AXP instruction.
 *
 * Output Parameters:
 *	decodedInstr:
 *		A pointer to structure that will be updated to indicate which physical
 *		registers are being used for this particular instruction.
 *
 * Return Value:
 *	None.
 */
static void AXP_RenameRegisters(
    AXP_21264_CPU *cpu,
    AXP_INSTRUCTION *decodedInstr)
{
    AXP_REGISTERS *src1Phys, *src2Phys, *destPhys;
    u16 *src1Map, *src2Map, *destMap;
    u16 *destFreeList, *flStart, *flEnd;
    bool src1Float = ((decodedInstr->decodedReg.bits.src1 & AXP_REG_FP)
  == AXP_REG_FP);
    bool src2Float = ((decodedInstr->decodedReg.bits.src2 & AXP_REG_FP)
  == AXP_REG_FP);
    bool destFloat = ((decodedInstr->decodedReg.bits.dest & AXP_REG_FP)
  == AXP_REG_FP);

    /*
     * If we are in PALmode and PALshadow registers are in use, then adjust the
     * actual register being utilized.  PALShadow registers are only for the
     * integer pipeline.
     *
     * Also, set some pointers to the floating-point or integer register
     * information.
     */
    if (src1Float == false)
    {
  decodedInstr->aSrc1 = AXP_REG(
      decodedInstr->aSrc1,
      decodedInstr->pc.pal);
  src1Phys = cpu->pr;
  src1Map = cpu->prMap;
    }
    else
    {
  src1Phys = cpu->pf;
  src1Map = cpu->pfMap;
    }
    if (src2Float == false)
    {
  decodedInstr->aSrc2 = AXP_REG(
      decodedInstr->aSrc2,
      decodedInstr->pc.pal);
  src2Phys = cpu->pr;
  src2Map = cpu->prMap;
    }
    else
    {
  src2Phys = cpu->pf;
  src2Map = cpu->pfMap;
    }
    if (destFloat == false)
    {
  decodedInstr->aDest = AXP_REG(
      decodedInstr->aDest,
      decodedInstr->pc.pal);
  destPhys = cpu->pr;
  destMap = cpu->prMap;
  destFreeList = cpu->prFreeList;
  flStart = &cpu->prFlStart;
  flEnd = &cpu->prFlEnd;
    }
    else
    {
  destPhys = cpu->pf;
  destMap = cpu->pfMap;
  destFreeList = cpu->pfFreeList;
  flStart = &cpu->pfFlStart;
  flEnd = &cpu->pfFlEnd;
    }

    /*
     * The source registers just use the current register mapping (integer or
     * floating-point).  If the register number is 31, it is always mapped to
     * physical register 31.  If the current register mapping indicates that
     * this register is doubly mapped, we need to use the dest one.
     */
    decodedInstr->src1 = src1Map[decodedInstr->aSrc1];
    src1Phys[decodedInstr->src1].refCount++;
    decodedInstr->src2 = src2Map[decodedInstr->aSrc2];
    src2Phys[decodedInstr->src2].refCount++;

    /*
     * First, save the previous mapping and value for the destination register.
     */
    decodedInstr->prevDestMap = destMap[decodedInstr->aDest];
    decodedInstr->prevDestValue = destPhys[decodedInstr->prevDestMap].value;

    /*
     * The destination register needs a little more work.  If the register
     * number is 31, then it is always mapped to the same physical register.
     */
    if (decodedInstr->aDest == AXP_UNMAPPED_REG)
    {

  /*
   * No need to re-map the destination register.  Also, note, R31 and F31
   * are always mapped to PR31 and PF31, respectively.  There is no need
   * to change this.
   */
  decodedInstr->dest = destMap[decodedInstr->aDest];
  destPhys[decodedInstr->dest].refCount++;
    }
    else
    {

  /*
   * Next determine if we need to put the currently mapped register onto
   * the free list.
   */
  if (destPhys[destMap[decodedInstr->aDest]].refCount == 0)
  {
      if (AXP_IBOX_OPT2)
      {
    AXP_TRACE_BEGIN();
    AXP_TraceWrite(
        "AXP_RenameRegisters freeing P%c%02d back onto the "
      "p%cFreeList[%u]",
        (destFloat ? 'F' : 'R'),
        destMap[decodedInstr->aDest],
        (destFloat ? 'f' : 'r'),
        *flEnd);
    AXP_TRACE_END()
    ;
      }
      destPhys[destMap[decodedInstr->aDest]].state = Free;
      destFreeList[*flEnd] = destMap[decodedInstr->aDest];
      *flEnd = (*flEnd + 1) % AXP_I_FREELIST_SIZE;
  }

  if (AXP_IBOX_OPT2)
  {
      AXP_TRACE_BEGIN();
      AXP_TraceWrite(
    "AXP_RenameRegisters mapping %c%02d --> P%c%02d from "
        "p%cFreeList[%u]",
    (destFloat ? 'F' : 'R'),
    decodedInstr->aDest,
    (destFloat ? 'F' : 'R'),
    destFreeList[*flStart],
    (destFloat ? 'f' : 'r'),
    *flStart);
      AXP_TRACE_END()
      ;
  }

  /*
   * Get the next register off of the free-list, and move the start index
   * to the next free register.
   */
  decodedInstr->dest = destFreeList[*flStart];
  *flStart = (*flStart + 1) % AXP_F_FREELIST_SIZE;
  destMap[decodedInstr->aDest] = decodedInstr->dest;
  destPhys[decodedInstr->dest].state = PendingUpdate;
  destPhys[decodedInstr->dest].refCount = 1;
  destPhys[decodedInstr->dest].value = 0;
    }

    /*
     * Return back to the caller.
     */
    if (AXP_IBOX_OPT1)
    {

  AXP_TRACE_BEGIN();
  AXP_TraceWrite("AXP_RenameRegisters returning for pc: 0x%016llx, "
      "with mapping of:", decodedInstr->pc);
  AXP_TraceWrite(
      "\t%c%02u --> P%c%02u (%s)",
      (src1Float ? 'F' : 'R'),
      decodedInstr->aSrc1,
      (src1Float ? 'F' : 'R'),
      decodedInstr->src1,
      regStateStr[src1Phys[decodedInstr->src1].state]);
  AXP_TraceWrite(
      "\t%c%02u --> P%c%02u (%s)",
      (src2Float ? 'F' : 'R'),
      decodedInstr->aSrc2,
      (src2Float ? 'F' : 'R'),
      decodedInstr->src2,
      regStateStr[src2Phys[decodedInstr->src2].state]);
  AXP_TraceWrite(
      "\t%c%02u --> P%c%02u (%s)",
      (destFloat ? 'F' : 'R'),
      decodedInstr->aDest,
      (destFloat ? 'F' : 'R'),
      decodedInstr->dest,
      regStateStr[destPhys[decodedInstr->dest].state]);
  AXP_TRACE_END()
  ;
    }

#ifdef AXP_VERIFY_REGISTERS

    /*
     * This is just for debugging the code.
     */
    AXP_RegisterRename_IntegrityCheck(cpu);
#endif

    return;
}

/*
 * AXP_UpdateRegisters
 *	This function is called when an instruction is retired.  It dereferences
 *	the source register, stores the result into the destination register, and
 *	then dereferences the destination register.  If after dereferencing a
 *	register, the reference count goes to zero and the current register mapping
 *	does not have the architectural register mapped to the same physical
 *	register, then the physical register is put back onto the free list.  As
 *	usual, R31 and F31 are ignored, except for the reference count.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	instr:
 *		A pointer to the structure containing a decoded representation of the
 *		Alpha AXP instruction.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	0:	Don't signal the Ebox or Fbox.
 *	1:	Only Signal the Ebox.
 *	2:	Only Signal the Fbox.
 */
u32 AXP_UpdateRegisters(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_REGISTERS *src1Phys, *src2Phys, *destPhys;
    u16 *src1Map, *src2Map, *destMap;
    u16 *src1FreeList, *src2FreeList, *destFreeList;
    u16 *src1FlEnd, *src2FlEnd, *destFlEnd;
    u32 retVal = AXP_SIGNAL_NONE;
    bool src1Float = ((instr->decodedReg.bits.src1 & AXP_REG_FP) == AXP_REG_FP);
    bool src2Float = ((instr->decodedReg.bits.src2 & AXP_REG_FP) == AXP_REG_FP);
    bool destFloat = ((instr->decodedReg.bits.dest & AXP_REG_FP) == AXP_REG_FP);

    if (AXP_IBOX_CALL)
    {
  AXP_TRACE_BEGIN();
  AXP_TraceWrite("AXP_UpdateRegisters called");
  AXP_TRACE_END()
  ;
    }

    /*
     * Set up some pointers to simplify the code following this..
     */
    if (src1Float == false)
    {
  src1Phys = cpu->pr;
  src1Map = cpu->prMap;
  src1FreeList = cpu->prFreeList;
  src1FlEnd = &cpu->prFlEnd;
    }
    else
    {
  src1Phys = cpu->pf;
  src1Map = cpu->pfMap;
  src1FreeList = cpu->pfFreeList;
  src1FlEnd = &cpu->pfFlEnd;
    }
    if (src2Float == false)
    {
  src2Phys = cpu->pr;
  src2Map = cpu->prMap;
  src2FreeList = cpu->prFreeList;
  src2FlEnd = &cpu->prFlEnd;
    }
    else
    {
  src2Phys = cpu->pf;
  src2Map = cpu->pfMap;
  src2FreeList = cpu->pfFreeList;
  src2FlEnd = &cpu->pfFlEnd;
    }
    if (destFloat == false)
    {
  destPhys = cpu->pr;
  destMap = cpu->prMap;
  destFreeList = cpu->prFreeList;
  destFlEnd = &cpu->prFlEnd;
    }
    else
    {
  destPhys = cpu->pf;
  destMap = cpu->pfMap;
  destFreeList = cpu->pfFreeList;
  destFlEnd = &cpu->pfFlEnd;
    }

    /*
     * Dereference the source registers.
     */
    src1Phys[instr->src1].refCount--;
    src2Phys[instr->src1].refCount--;

    /*
     * Move the value from executing the instruction into the physical
     * register.
     */
    if (instr->aDest != AXP_UNMAPPED_REG)
    {
  if (AXP_IBOX_OPT2)
  {
      AXP_TRACE_BEGIN();
      AXP_TraceWrite(
    "AXP_UpdateRegisters saving %c%02d (P%c%02d) = 0x%016llx",
    (destFloat ? 'F' : 'R'),
    instr->aDest,
    (destFloat ? 'F' : 'R'),
    instr->dest,
    (destFloat ? instr->destv.fp.uq : instr->destv.r.uq));
      AXP_TRACE_END()
      ;
  }

  destPhys[instr->dest].value =
      destFloat ? instr->destv.fp.uq : instr->destv.r.uq;
  destPhys[instr->dest].state = Valid;
    }
    destPhys[instr->dest].refCount--;

    /*
     * OK, now see if any of the registers mapped at the time of this
     * instruction's initial decoding can be returned to the free list.  We do
     * this by the following logic (all must be true):
     *
     *	1) The register is not R31 or F31
     *	2) The current register mapping is not the same since originally mapped
     *	3) The reference counter is down to zero.
     *
     * Only return a register back onto the free list once.
     */
    if ((instr->aSrc1 != AXP_UNMAPPED_REG)
  && (src1Map[instr->aSrc1] != instr->src1)
  && (src1Phys[instr->src1].refCount == 0))
    {
  if (AXP_IBOX_OPT2)
  {
      AXP_TRACE_BEGIN();
      AXP_TraceWrite(
    "AXP_UpdateRegisters freeing P%c%02d back onto the "
        "p%cFreeList[%u] : (src1)",
    (src1Float ? 'F' : 'R'),
    instr->src1,
    (src1Float ? 'f' : 'r'),
    *src1FlEnd);
      AXP_TRACE_END()
      ;
  }
  src1Phys[instr->src1].state = Free;
  src1FreeList[*src1FlEnd] = instr->src1;
  *src1FlEnd = (*src1FlEnd + 1) % AXP_I_FREELIST_SIZE;
    }
    if ((instr->aSrc2 != AXP_UNMAPPED_REG)
  && (src2Map[instr->aSrc2] != instr->src2)
  && (instr->src2 != instr->src1)
  && (src2Phys[instr->src2].refCount == 0))
    {
  if (AXP_IBOX_OPT2)
  {
      AXP_TRACE_BEGIN();
      AXP_TraceWrite(
    "AXP_UpdateRegisters freeing P%c%02d back onto the "
        "p%cFreeList[%u] : (src2)",
    (src2Float ? 'F' : 'R'),
    instr->src2,
    (src2Float ? 'f' : 'r'),
    *src2FlEnd);
      AXP_TRACE_END()
      ;
  }
  src2Phys[instr->src2].state = Free;
  src2FreeList[*src2FlEnd] = instr->src2;
  *src2FlEnd = (*src2FlEnd + 1) % AXP_I_FREELIST_SIZE;
    }
    if ((instr->aDest != AXP_UNMAPPED_REG)
  && (destMap[instr->aDest] != instr->dest)
  && (instr->dest != instr->src1) && (instr->dest != instr->src2)
  && (destPhys[instr->dest].refCount == 0))
    {
  if (AXP_IBOX_OPT2)
  {
      AXP_TRACE_BEGIN();
      AXP_TraceWrite(
    "AXP_UpdateRegisters freeing P%c%02d back onto the "
        "p%cFreeList[%u] : (dest)",
    (destFloat ? 'F' : 'R'),
    instr->dest,
    (destFloat ? 'f' : 'r'),
    *destFlEnd);
      AXP_TRACE_END()
      ;
  }
  destPhys[instr->dest].state = Free;
  destFreeList[*destFlEnd] = instr->dest;
  *destFlEnd = (*destFlEnd + 1) % AXP_I_FREELIST_SIZE;
    }

    /*
     * Signal all threads in either the Ebox or Fbox, depending upon whether
     * the destination register was a floating point register or not.  Also, we
     * don't concern ourselves about R31 or F31, as these are always zero and
     * always valid.
     */
    if (instr->aDest != AXP_UNMAPPED_REG)
    {
  if (destFloat == true)
      retVal = AXP_SIGNAL_FBOX;
  else
      retVal = AXP_SIGNAL_EBOX;
    }

#ifdef AXP_VERIFY_REGISTERS

    /*
     * This is just for debugging the code.
     */
    AXP_RegisterRename_IntegrityCheck(cpu);
#endif

    /*
     * Return back to the caller.
     */
    return (retVal);
}

/*
 * AXP_AbortInstructions
 *	This function is called when an instruction causes all subsequent queued
 *	instructions to be aborted (execution canceled or pulled from the queue).
 *	The instruction passed on the call is the one to which we want to rollback.
 *	While rolling back, the architectural to physical mapping will also be
 *	rolled back and the value at the time a destination register was assigned
 *	will be reestablished.  If an instruction is still in the either the IQ or
 *	FQ, then entry will be indicated so that the Ibox and Fbox can detect when
 *	an instruction was aborted prior to being executed.  When we detect an
 *	instruction is on one of these queues, the mutex for that queue will be
 *	broadcast.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	instr:
 *		A pointer to the structure containing a decoded representation of the
 *		Alpha AXP instruction.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	An instruction that stalled the Ibox was aborted.
 *	false:	No instructions that stalled the Ibox were aborted.
 */
bool AXP_AbortInstructions(AXP_21264_CPU *cpu, AXP_INSTRUCTION *inst)
{
    AXP_INSTRUCTION *rob;
    AXP_REGISTERS *src1Phys, *src2Phys, *destPhys;
    u16 *destMap;
    u16 *destFreeList;
    u16 *destFlStart;
    u16 *destFlEnd;
    u32 endIdx = cpu->robEnd;
    bool rollbackRegisterMap;
    bool retVal = false;

    if (AXP_IBOX_CALL)
    {
  AXP_TRACE_BEGIN();
  AXP_TraceWrite(
      "AXP_AbortInstructions called robStart = %u : robEnd = %u",
      cpu->robStart,
      cpu->robEnd);
  AXP_TRACE_END()
  ;
    }

    /*
     * The robMutex is already locked when we get here.  So, don't try and lock
     * it again.
     *
     * The rob indexes wrap around from the end to the beginning.  The end
     * always points to the next available entry.  So we need to look at the
     * entry previous to the one pointed to by the end index, but this,
     * numerically, may be the last address, which is greater than the end.
     */
    endIdx = ((endIdx == 0) ? AXP_INFLIGHT_MAX : endIdx) - 1;

    /*
     * We continue to loop until either the end index equals the start index or
     * we find the entry to which we were to rollback.
     */
    rob = &cpu->rob[endIdx];
    while ((endIdx != cpu->robStart)
  && (*((u64 *) &rob->pc) != *((u64 *) &inst->pc)))
    {
  rollbackRegisterMap = false;

  switch (rob->state)
  {

      /*
       * If the entry is queued or executing, the Ebox or Fbox have it.
       * Setting the state to Aborted, indicates to them that the\
       * execution of this instruction needs to be aborted.  We need to
       * rollback the register mapping.
       */
      case Queued:
      case Executing:
    rob->state = Aborted;
    rollbackRegisterMap = true;
    break;

    /*
     * If it is waiting for retirement is retired.  We need to rollback
     * the register mapping.
     */
      case WaitingRetirement:
    rob->state = Retired;
    rollbackRegisterMap = true;
    break;

    /*
     * For Retired or Aborted, there is noting else to do.  We don't
     * even have to rollback the register mapping.
     */
      case Retired:
      case Aborted:
    rollbackRegisterMap = false;
    break;
  }

  /*
   * This is kind of what we really came here for.  If we need to
   * rollback the mapping, then we need to make the physical registers
   * look like they did prior to this instruction getting decoded, with
   * the same value.  Additionally, we need to find the previous mapping
   * and adjust the free list and mapping.
   */
  if (rollbackRegisterMap == true)
  {

      if (AXP_IBOX_OPT2)
      {
    AXP_TRACE_BEGIN();
    AXP_TraceWrite("AXP_AbortInstructions @ pc 0x%016llx, "
        "opcode = 0x%02x", *((u64 *) &rob->pc), rob->opcode);
    AXP_TRACE_END()
    ;
      }

      /*
       * Before we go too far, this is a good place to determine if we
       * have or are aborting an instruction that stalled the Ibox.
       */
      if (retVal == false)
    retVal = rob->stall;

      /*
       * The code for floating point and integer register mapping is
       * virtually identical.  So, set up some pointed to allow the code
       * to be a little more straightforward.
       */
      if ((rob->decodedReg.bits.src1 & AXP_REG_FP) == AXP_REG_FP)
    src1Phys = cpu->pf;
      else
    src1Phys = cpu->pr;
      if ((rob->decodedReg.bits.src2 & AXP_REG_FP) == AXP_REG_FP)
    src2Phys = cpu->pf;
      else
    src2Phys = cpu->pr;
      if ((rob->decodedReg.bits.dest & AXP_REG_FP) == AXP_REG_FP)
      {
    destPhys = cpu->pf;
    destMap = cpu->pfMap;
    destFreeList = cpu->pfFreeList;
    destFlStart = &cpu->pfFlStart;
    destFlEnd = &cpu->pfFlEnd;
      }
      else
      {
    destPhys = cpu->pr;
    destMap = cpu->prMap;
    destFreeList = cpu->prFreeList;
    destFlStart = &cpu->prFlStart;
    destFlEnd = &cpu->prFlEnd;
      }

      /*
       * When we did the mapping, we incremented the reference count for
       * the source registers.  Now we need to decrement it.
       */
      src1Phys[rob->src1].refCount--;
      src2Phys[rob->src2].refCount--;

      /*
       * Now we need to undo the destination mapping.  If we are mapped
       * to R31 or F31, then we do not need to add the register map back
       * onto the free list.  Otherwise, we do.
       */
      destPhys[rob->dest].refCount--;

      /*
       * If we ended up freeing the previous entry, then we need to
       * pull this off the list.  Just decrement the end index.
       */
      if (destPhys[rob->prevDestMap].state == Free)
    *destFlEnd = ((*destFlEnd == 0) ?
    AXP_I_FREELIST_SIZE :
              *destFlEnd) - 1;

      /*
       * If the destination is not R31 or F31, then we need to put it
       * back on the free list.
       */
      if (rob->dest != AXP_UNMAPPED_REG)
      {
    if (AXP_IBOX_OPT2)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite(
      "AXP_AbortInstructions freeing P%c%02d back onto "
          "the p%cFreeList[%u]",
      rob->src1,
      ((destPhys == cpu->pf) ? 'f' : 'r'),
      *destFlStart);
        AXP_TRACE_END()
        ;
    }

    *destFlStart = ((*destFlStart == 0) ?
    AXP_I_FREELIST_SIZE :
                  *destFlStart) - 1;
    destFreeList[*destFlStart] = rob->dest;
    destPhys[rob->dest].state = Free;
      }

      /*
       * Update the mapping to what it was previously, including the
       * value, but not if it is R31 or F31.  Also, force the state to
       * valid.
       */
      destMap[rob->aDest] = rob->prevDestMap;
      if (rob->prevDestMap != AXP_UNMAPPED_REG)
    destPhys[rob->prevDestMap].value = rob->prevDestValue;
      destPhys[rob->prevDestMap].state = Valid;
  }

  /*
   * Move to the next previous entry.
   */
  cpu->robEnd = endIdx;
  endIdx = ((endIdx == 0) ? AXP_INFLIGHT_MAX : endIdx) - 1;
  rob = &cpu->rob[endIdx];
    }

#ifdef AXP_VERIFY_REGISTERS

    /*
     * This is just for debugging the code.
     */
    AXP_RegisterRename_IntegrityCheck(cpu);
#endif

    /*
     * Return the results of this processing back to the caller.
     */
    return (retVal);
}

#ifdef AXP_VERIFY_REGISTERS

/*
 * AXP_RegisterRename_IntegrityCheck
 */
void AXP_RegisterRename_IntegrityCheck(AXP_21264_CPU *cpu)
{
    u8 physInUse[AXP_INT_PHYS_REG];
    int ii, end;
    bool wrap;

    /*
     * First Integer Registers
     */
    for (ii = 0; ii < AXP_INT_PHYS_REG; ii++)
    physInUse[ii] = 0;
    for (ii = 0; ii < AXP_MAX_INT_REGISTERS; ii++)
    physInUse[cpu->prMap[ii]]++;
    wrap = cpu->prFlStart > cpu->prFlEnd;
    end = wrap ? AXP_I_FREELIST_SIZE : cpu->prFlEnd;
    ii = cpu->prFlStart;
    while (ii < end)
    {
  physInUse[cpu->prFreeList[ii]]++;
  ii++;
  if ((ii == AXP_I_FREELIST_SIZE) && (wrap == true))
  {
      end = cpu->prFlEnd;
      ii = 0;
      wrap = false;
  }
    }

    for (ii = 0; ii < AXP_INT_PHYS_REG; ii++)
    if (physInUse[ii] != 1)
    {
  if (AXP_IBOX_OPT2)
  {
      AXP_TRACE_BEGIN();
      AXP_TraceWrite(
    ">>>> Physical Register PR%02d is referenced "
    "%u times.",
    ii,
    physInUse[ii]);
      AXP_TRACE_END();
  }
    }

    /*
     * Last Floating Registers
     */
    for (ii = 0; ii < AXP_FP_PHYS_REG; ii++)
    physInUse[ii] = 0;
    for (ii = 0; ii < AXP_MAX_FP_REGISTERS; ii++)
    physInUse[cpu->pfMap[ii]]++;
    wrap = cpu->pfFlStart > cpu->pfFlEnd;
    end = wrap ? AXP_I_FREELIST_SIZE : cpu->pfFlEnd;
    ii = cpu->pfFlStart;
    while (ii < end)
    {
  physInUse[cpu->pfFreeList[ii]]++;
  ii++;
  if ((ii == AXP_I_FREELIST_SIZE) && (wrap == true))
  {
      end = cpu->pfFlEnd;
      ii = 0;
      wrap = false;
  }
    }
    for (ii = 0; ii < AXP_FP_PHYS_REG; ii++)
    if (physInUse[ii] != 1)
    {
  if (AXP_IBOX_OPT2)
  {
      AXP_TRACE_BEGIN();
      AXP_TraceWrite(
    ">>>> Physical Register PF%02d is referenced "
    "%u times.",
    ii,
    physInUse[ii]);
      AXP_TRACE_END();
  }
    }
    return;
}
#endif
