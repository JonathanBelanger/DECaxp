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
 *	This header file contains the structures and definitions required to
 *	implement the instruction emulation for the Alpha 21264 (EV68) processor.
 *
 *	Revision History:
 *
 *	V01.000		04-May-2017	Jonathan D. Belanger
 *	Initially written.
 *	(May the 4th be with you).
 *
 *	V01.001		10-May-2017	Jonathan D. Belanger
 *	Included the AXP Utility header file for definitions like u64, i64, etc..
 *
 *	V01.002		14-May-2017	Jonathan D. Belanger
 *	Added some comments about considerations for the next set of additions.
 *
 *	V01.003		25-May-2017	Jonathan D. Belanger
 *	Added the instruction format for the PALcode Reserved Instructions.
 *
 *	V01.004		01-Jan-2018	Jonathan D. Belanger
 *	Changed the way instructions are completed when they need to utilize the
 *	Mbox.
 *
 *	V01.005		11-Mar-2018	Jonathan D. Belanger
 *	Changed the LEN_STALL flag used for the HW_LD/ST and HW_RET instructions
 *	into two separate flags.  One to indicate a quadword len, and the other to
 *	indicate a stall in the Ibox.
 */
#ifndef _AXP_21264_INS_DEFS_
#define _AXP_21264_INS_DEFS_

#include "AXP_Configure.h"
#include "AXP_Utility.h"
#include "AXP_Base_CPU.h"
#include "AXP_21264_IPRs.h"
#include "AXP_21264_RegisterRenaming.h"

/*
 * Define the Alpha Instruction Formats
 *
 * Memory Instruction Format
 */
typedef struct
{
	union
	{
		u16	func : 16;				/* Memory function						*/
		i16 disp : 16;				/* Memory displacement from PC (SEXT)	*/
	} mem;
	 u32	rb : 5;					/* Register b (source)					*/
	 u32	ra : 5;					/* Register a (destination)				*/
	 u32	opcode : 6;				/* Operation code						*/
} AXP_MEM_INS;
#define AXP_LDAH_MULT		65536

/*
 * These macros extract the JMP type and displacement from the disp field in
 * the AXP_MEM_INS format of the Digital Alpha AXP architecture.
 */
#define	AXP_JMP_TYPE(disp)	(((disp) & 0xc000) >> 14)
#define AXP_JMP_HINT(disp)	((disp) & 0x3fff)

/*
 * Branch Instruction Format
 */
typedef struct
{
	 i32	branch_disp : 21;		/* Branch displacement*4 from PC (SEXT) */
	 u32	ra : 5;					/* Register a (destination)				*/
	 u32	opcode : 6;				/* Operation code						*/
} AXP_BR_INS;

/*
 * Integer Operate Instruction Format, with Rb, bit <12> = 0
 */
typedef struct
{
	 u32	rc : 5;					/* Register c (source 2)				*/
	 u32	func : 7;				/* Integer Function						*/
	 u32	fmt : 1;				/* Format Literal=1 or Rb=0				*/
	 u32	sbz : 3;				/* Should Be Zero						*/
	 u32	rb : 5;					/* Register b (source 1)				*/
	 u32	ra : 5;					/* Register a (destination)				*/
	 u32	opcode : 6;				/* Operation code						*/
} AXP_OP1_INS;

/*
 * Integer Operate Instruction Format, with Rb, bit <12> = 1
 */
typedef struct
{
	 u32	rc : 5;					/* Register c (source)					*/
	 u32	func : 7;				/* Integer Function						*/
	 u32	fmt : 1;				/* Format Literal=1 or Rb=0				*/
	 u32	lit : 8;				/* Literal (ZEXT)						*/
	 u32	ra : 5;					/* Register a (destination)				*/
	 u32	opcode : 6;				/* Operation code						*/
} AXP_OP2_INS;

/*
 * FP Operate Instruction Format
 */
typedef struct
{
	 u32	fc : 5;					/* Floating Point Register c (source 2)	*/
	 u32	func : 11;				/* Floating Point Function				*/
	 u32	fb : 5;					/* Floating Point Register b (source 1)	*/
	 u32	fa : 5;					/* Floating Point Register a (target)	*/
	 u32	opcode : 6;				/* Operation Code						*/
} AXP_FP_INS;

/*
 * PALcode Instruction Format
 */
typedef struct
{
	 u32	palcode_func : 26;		/* PALcode Function						*/
	 u32	opcode : 6;				/* Operation Code						*/
} AXP_PAL_INS;

/*
 * Reserved for PALcode Instruction Format
 *
 *	HW_LD Instruction Format (opcode = 0x1b)
 *		For len:
 *			0 = access length is longword
 *			1 = access length is quadword
 *		For type:
 *			000 = Physical
 *			001 = Physical/Lock
 *			010 = Virtual/VPTE (virtual PTE fetch)
 *			100 = Virtual
 *			101 = Virtual/WriteCheck
 *			110 = Virtual/AltMode
 *			111 = Virtual/WriteCheck/AltMode
 */
typedef struct
{
	i32		disp : 12;				/* 12-bit Signed Displacement			*/
	u32		len : 1;				/* Access Length						*/
	u32		type : 3;				/* Type of Address						*/
	u32		rb : 5;					/* Base Register for Memory Address		*/
	u32		ra : 5;					/* Destination Register Number			*/
	u32		opcode : 6;				/* Operation Code						*/
} AXP_HW_LD;

#define AXP_HW_LD_PHYS		0x0		/* Physical								*/
#define AXP_HW_LD_PHYS_LOCK	0x1		/* Physical/Lock						*/
#define AXP_HW_LD_VPTE		0x2		/* Virtual PTE (VPTE)					*/
#define AXP_HW_LD_VIRT		0x4		/* Virtual								*/
#define AXP_HW_LD_VIRT_WCHK	0x5		/* Virtual/WrChk						*/
#define AXP_HW_LD_VIRT_ALT	0x6		/* Virtual/Alt access DTB_ALT_MODE IPR	*/
#define AXP_HW_LD_VIRT_WALT	0x7		/* Virual/WrChk/Alt combination above 2	*/

#define AXP_HW_LD_LONGWORD	0
#define AXP_HW_LD_QUADWORD	1

/*
 * HW_ST Instruction Format (opcode = 0x1f)
 *		For len:
 *			0 = access length is longword
 *			1 = access length is quadword
 *		For type:
 *			000 = Physical
 *			001 = Physical/Cond
 *			010 = Virtual
 *			110 = Virtual/AltMode
 */
typedef struct
{
	i32		disp : 12;				/* 12-bit Signed Displacement			*/
	u32		len : 1;				/* Access Length						*/
	u32		type : 3;				/* Type of Address						*/
	u32		rb : 5;					/* Base Register for Memory Address		*/
	u32		ra : 5;					/* Write Data Register Number			*/
	u32		opcode : 6;				/* Operation Code						*/
} AXP_HW_ST;

#define AXP_HW_ST_PHYS		0x0		/* Physical								*/
#define AXP_HW_ST_PHYS_LOCK	0x1		/* Physical/Conditional					*/
#define AXP_HW_ST_VIRT		0x2		/* Virtual								*/
#define AXP_HW_ST_VIRT_ALT	0x6		/* Virtual/Alt access DTB_ALT_MODE IPR	*/

#define AXP_HW_ST_LONGWORD	0
#define AXP_HW_ST_QUADWORD	1

/*
 * HW_RET Instruction Format (opcode = 0x1e)
 *		For hint:
 *			00 = HW_JMP, PC is not pushed onto the prediction stack
 *			01 = HW_JSR, PC is pushed onto the prediction stack
 *			10 = HW_RET, prediction is popped off the stack as the target
 *			11 = HW_COROUTINE, prediction is popped off the stack and PC is
 *				 pushed on
 */
typedef struct
{
	i32		disp : 13;				/* 13-bit Signed Displacement			*/
	u32		stall : 1;				/* Stall Instruction until retire/abort	*/
	u32		hint : 2;				/* Type of Address						*/
	u32		rb : 5;					/* Base Register for Memory Address		*/
	u32		ra : 5;					/* Write Data Register Number			*/
	u32		opcode : 6;				/* Operation Code						*/
} AXP_HW_RET1;

#define AXP_HW_JMP			0
#define AXP_HW_JSR			1
#define AXP_HW_RET			2
#define AXP_HW_COROUTINE	3

/*
 * HW_MFPR/HW_MTPR Instruction Format (opcode = 0x19/0x1d)
 */
typedef struct
{
	u32		scbd_mask : 8;			/* Scoreboard Mask						*/
	u32		index : 8;				/* IPR Index							*/
	u32		rb : 5;					/* Base Register for Memory Address		*/
	u32		ra : 5;					/* Write Data Register Number			*/
	u32		opcode : 6;				/* Operation Code						*/
} AXP_HW_MXPR;

/*
 * Union of a the 32-bit AXP instruction formats.
 */
typedef union
{
	u32				instr;			/* 32-bit instruction value				*/
	AXP_MEM_INS		mem;			/* Memory Instruction					*/
	AXP_BR_INS		br;				/* Branch Instruction					*/
	AXP_OP1_INS		oper1;			/* Integer Operate Instruction (Rb)		*/
	AXP_OP2_INS		oper2;			/* Integer Operate Instruction (LIT)	*/
	AXP_FP_INS		fp;				/* Floating Point Operate Instruction	*/
	AXP_PAL_INS		pal;			/* PALcode Instruction					*/
	AXP_HW_LD		hw_ld;			/* PALcode Load Instruction				*/
	AXP_HW_ST		hw_st;			/* PALcode Store Instruction			*/
	AXP_HW_RET1		hw_ret;			/* PALcode Return Instruction			*/
	AXP_HW_MXPR		hw_mxpr;		/* PALcode Move From/To Processor Regis	*/
} AXP_INS_FMT;

/*
 * Instruction type
 */
typedef enum
{
	Bra,
	FP,
	FPBra,
	Mem,
	Mfc,
	Mbr,
	Opr,
	Pcd,
	PAL,
	Res,
	Cond
} AXP_INS_TYPE;

/*
 * Operation Type
 */
typedef enum
{
	Load,
	Store,
	Branch,
	Arith,
	Logic,
	Oper,
	Other
} AXP_OPER_TYPE;

/*
 * Instruction Opcode definitions
 */
#define PAL00	0x00	/* Pcd */
#define OPC01	0x01	/* Res */
#define OPC02	0x02	/* Res */
#define OPC03	0x03	/* Res */
#define OPC04	0x04	/* Res */
#define OPC05	0x05	/* Res */
#define OPC06	0x06	/* Res */
#define OPC07	0x07	/* Res */
#define LDA		0x08	/* Mem */
#define LDAH	0x09	/* Mem */
#define LDBU	0x0a	/* Mem */
#define LDQ_U	0x0b	/* Mem */
#define LDW_U	0x0c	/* Mem */
#define STW		0x0d	/* Mem */
#define STB		0x0e	/* Mem */
#define STQ_U	0x0f	/* Mem */
#define INTA	0x10	/* Opr */
#define INTL	0x11	/* Opr */
#define INTS	0x12	/* Opr */
#define INTM	0x13	/* Opr */
#define ITFP	0x14	/* FP */
#define FLTV	0x15	/* FP (VAX) */
#define FLTI	0x16	/* FP (IEEE) */
#define FLTL	0x17	/* FP */
#define MISC	0x18	/* Mfc */
#define HW_MFPR	0x19	/* reserved for PALcode */
#define JMP		0x1a	/* Mbr */
#define HW_LD	0x1b	/* reserved for PALcode */
#define FPTI	0x1c	/* FP & Opr */
#define HW_MTPR	0x1d	/* reserved for PALcode */
#define HW_RET	0x1e	/* reserved for PALcode */
#define HW_ST	0x1f	/* reserved for PALcode */
#define LDF		0x20	/* Mem */
#define LDG		0x21	/* Mem */
#define LDS		0x22	/* Mem */
#define LDT		0x23	/* Mem */
#define STF		0x24	/* Mem */
#define STG		0x25	/* Mem */
#define STS		0x26	/* Mem */
#define STT		0x27	/* Mem */
#define LDL		0x28	/* Mem */
#define LDQ		0x29	/* Mem */
#define LDL_L	0x2a	/* Mem */
#define LDQ_L	0x2b	/* Mem */
#define STL		0x2c	/* Mem */
#define STQ		0x2d	/* Mem */
#define STL_C	0x2e	/* Mem */
#define STQ_C	0x2f	/* Mem */
#define BR		0x30	/* Bra */
#define FBEQ	0x31	/* Bra (FP) */
#define FBLT	0x32	/* Bra (FP) */
#define FBLE	0x33	/* Bra (FP) */
#define BSR		0x34	/* Mbr */
#define FBNE	0x35	/* Bra (FP) */
#define FBGE	0x36	/* Bra (FP) */
#define FBGT	0x37	/* Bra (FP) */
#define BLBC	0x38	/* Bra */
#define BEQ		0x39	/* Bra */
#define BLT		0x3a	/* Bra */
#define BLE		0x3b	/* Bra */
#define BLBS	0x3c	/* Bra */
#define BNE		0x3d	/* Bra */
#define BGE		0x3e	/* Bra */
#define BGT		0x3f	/* Bra */

/*
 * OpenVMS PALcode Opcodes
 */
#define VMS_HALT			0x0000
#define VMS_CFLUSH			0x0001
#define VMS_DRAINA			0x0002
#define VMS_LDQP			0x0003
#define VMS_STQP			0x0004
#define VMS_SWPCTX			0x0005
#define VMS_MFPR_ASN		0x0006
#define VMS_MTPR_ASTEN		0x0007
#define VMS_MTPR_ASTSR		0x0008
#define VMS_CSERVE			0x0009
#define VMS_SWPPAL			0x000a
#define VMS_MFPR_FEN		0x000b
#define VMS_MTPR_FEN		0x000c
#define VMS_MTPR_IPIR		0x000d
#define VMS_MFPR_IPL		0x000e
#define VMS_MTPR_IPL		0x000f
#define VMS_MFPR_MCES		0x0010
#define VMS_MTPR_MCES		0x0011
#define VMS_MFPR_PCBB		0x0012
#define VMS_MFPR_PRBR		0x0013
#define VMS_MTPR_PRBR		0x0014
#define VMS_MFPR_PTBR		0x0015
#define VMS_MFPR_SCBB		0x0016
#define VMS_MTPR_SCBB		0x0017
#define VMS_MTPR_SIRR		0x0018
#define VMS_MFPR_SISR		0x0019
#define VMS_MFPR_TBCHK		0x001a
#define VMS_MTPR_TBIA		0x001b
#define VMS_MTPR_TBIAP		0x001c
#define VMS_MTPR_TBIS		0x001d
#define VMS_MFPR_ESP		0x001e
#define VMS_MTPR_ESP		0x001f
#define VMS_MFPR_SSP		0x0020
#define VMS_MTPR_SSP		0x0021
#define VMS_MFPR_USP		0x0022
#define VMS_MTPR_USP		0x0023
#define VMS_MTPR_TBISD		0x0024
#define VMS_MTPR_TBISI		0x0025
#define VMS_MFPR_ASTEN		0x0026
#define VMS_MFPR_ASTSR		0x0027
#define VMS_MFPR_VPTB		0x0029
#define VMS_MTPR_VPTB		0x002a
#define VMS_MTPR_PERFMON	0x002b
#define VMS_MTPR_DATFX		0x002e
#define VMS_WTINT			0x003e
#define VMS_MFPR_WHAMI		0x003f
#define VMS_BPT				0x0080
#define VMS_BUGCHK			0x0081
#define VMS_CHME			0x0082
#define VMS_CHMK			0x0083
#define VMS_CHMS			0x0084
#define VMS_CHMU			0x0085
#define VMS_IMB				0x0086
#define VMS_INSQHIL			0x0087
#define VMS_INSQTIL			0x0088
#define VMS_INSQHIQ			0x0089
#define VMS_INSQTIQ			0x008a
#define VMS_INSQUEL			0x008b
#define VMS_INSQUEQ			0x008c
#define VMS_INSQUEL_D		0x008d
#define VMS_INSQUEQ_D		0x008e
#define VMS_PROBER			0x008f
#define VMS_PROBEW			0x0090
#define VMS_RD_PS			0x0091
#define VMS_REI				0x0092
#define VMS_REMQHIL			0x0093
#define VMS_REMQTIL			0x0094
#define VMS_REMQHIQ			0x0095
#define VMS_REMQTIQ			0x0096
#define VMS_REMQUEL			0x0097
#define VMS_REMQUEQ			0x0098
#define VMS_REMQUEL_D		0x0099
#define VMS_REMQUEQ_D		0x009a
#define VMS_SWASTEN			0x009b
#define VMS_WR_PS_SW		0x009c
#define VMS_RSCC			0x009d
#define VMS_READ_UNQ		0x009e
#define VMS_WRITE_UNQ		0x009f
#define VMS_AMOVRR			0x00a0
#define VMS_AMOVRM			0x00a1
#define VMS_INSQHILR		0x00a2
#define VMS_INSQTILR		0x00a3
#define VMS_INSQHIQR		0x00a4
#define VMS_INSQTIQR		0x00a5
#define VMS_REMQHILR		0x00a6
#define VMS_REMQTILR		0x00a7
#define VMS_REMQHIQR		0x00a8
#define VMS_REMQTIQR		0x00a9
#define VMS_GENTRAP			0x00aa
#define VMS_CLRFEN			0x00ae

/*
 * Tru64 (OSF) & Linux PALcode Opcodes
 */
#define OSF_HALT			0x0000
#define OSF_CFLUSH			0x0001
#define OSF_DRAINA			0x0002
#define OSF_CSERVE			0x0009
#define OSF_SWPPAL			0x000a
#define OSF_WRIPIR			0x000d
#define OSF_RDMCES			0x0010
#define OSF_WRMCES			0x0011
#define OSF_WRFEN			0x002b
#define OSF_WRVPTPTR		0x002d
#define OSF_WRASN			0x002e	/* Not used by Linux */
#define OSF_SWPCTX			0x0030
#define OSF_WRVAL			0x0031
#define OSF_RDVAL			0x0032
#define OSF_TBI				0x0033
#define OSF_WRENT			0x0034
#define OSF_SWPIPL			0x0035
#define OSF_RDPS			0x0036
#define OSF_WRKGP			0x0037
#define OSF_WRUSP			0x0038
#define OSF_WRPERFMON		0x0039
#define OSF_RDUSP			0x003a
#define OSF_WHAMI			0x003c
#define OSF_RETSYS			0x003d
#define OSF_WTINT			0x003e
#define OSF_RTI				0x003f
#define OSF_BPT				0x0080
#define OSF_BUGCHK			0x0081
#define OSF_CALLSYS			0x0083
#define OSF_IMB				0x0086
#define OSF_URTI			0x0092	/* Not used by Linux */
#define OSF_RDUNIQUE		0x009e
#define OSF_WRUNIQUE		0x009f
#define OSF_GENTRAP			0x00aa
#define OSF_CLRFEN			0x00ae

/*
 * Opcode function codes
 */
#define AXP_FUNC_JMP			0x00
#define AXP_FUNC_ADDL			0x00
#define AXP_FUNC_AND			0x00
#define AXP_FUNC_MULL			0x00
#define AXP_FUNC_SEXTB			0x00
#define AXP_FUNC_ADDF_C			0x000
#define AXP_FUNC_ADDS_C			0x000
#define AXP_FUNC_TRAPB			0x0000
#define AXP_FUNC_SUBS_C			0x001
#define AXP_FUNC_SUBF_C			0x001
#define AXP_FUNC_MULF_C			0x002
#define AXP_FUNC_MULS_C			0x002
#define AXP_FUNC_DIVF_C			0x003
#define AXP_FUNC_DIVS_C			0x003
#define AXP_FUNC_ITOFS			0x004
#define AXP_FUNC_SQRTF_C		0x00a
#define AXP_FUNC_SQRTS_C		0x00b
#define AXP_FUNC_SEXTW			0x01
#define AXP_FUNC_JSR			0x01
#define AXP_FUNC_CVTLQ			0x010
#define AXP_FUNC_ITOFF			0x014
#define AXP_FUNC_CVTDG_C		0x01e
#define AXP_FUNC_MSKBL			0x02
#define AXP_FUNC_S4ADDL			0x02
#define AXP_FUNC_RET			0x02
#define AXP_FUNC_ADDG_C			0x020
#define AXP_FUNC_ADDT_C			0x020
#define AXP_FUNC_CPYS			0x020
#define AXP_FUNC_SUBG_C			0x021
#define AXP_FUNC_SUBT_C			0x021
#define AXP_FUNC_CPYSN			0x021
#define AXP_FUNC_MULG_C			0x022
#define AXP_FUNC_MULT_C			0x022
#define AXP_FUNC_CPYSE			0x022
#define AXP_FUNC_DIVG_C			0x023
#define AXP_FUNC_DIVT_C			0x023
#define AXP_FUNC_MT_FPCR		0x024
#define AXP_FUNC_ITOFT			0x024
#define AXP_FUNC_MF_FPCR		0x025
#define AXP_FUNC_SQRTG_C		0x02a
#define AXP_FUNC_FCMOVEQ		0x02a
#define AXP_FUNC_FCMOVNE		0x02b
#define AXP_FUNC_SQRTT_C		0x02b
#define AXP_FUNC_CVTGF_C		0x02c
#define AXP_FUNC_CVTTS_C		0x02c
#define AXP_FUNC_FCMOVLT		0x02c
#define AXP_FUNC_CVTGD_C		0x02d
#define AXP_FUNC_FCMOVGE		0x02d
#define AXP_FUNC_FCMOVLE		0x02e
#define AXP_FUNC_CVTGQ_C		0x02f
#define AXP_FUNC_FCMOVGT		0x02f
#define AXP_FUNC_CVTTQ_C		0x02f
#define AXP_FUNC_JSR_COROUTINE	0x03
#define AXP_FUNC_CVTQL			0x030
#define AXP_FUNC_CVTQS_C		0x03c
#define AXP_FUNC_CVTQF_C		0x03c
#define AXP_FUNC_CVTQT_C		0x03e
#define AXP_FUNC_CVTQG_C		0x03e
#define AXP_FUNC_ADDS_M			0x040
#define AXP_FUNC_EXCB			0x0400
#define AXP_FUNC_SUBS_M			0x041
#define AXP_FUNC_MULS_M			0x042
#define AXP_FUNC_DIVS_M			0x043
#define AXP_FUNC_SQRTS_M		0x04b
#define AXP_FUNC_EXTBL			0x06
#define AXP_FUNC_ADDT_M			0x060
#define AXP_FUNC_SUBT_M			0x061
#define AXP_FUNC_MULT_M			0x062
#define AXP_FUNC_DIVT_M			0x063
#define AXP_FUNC_SQRTT_M		0x06b
#define AXP_FUNC_CVTTS_M		0x06c
#define AXP_FUNC_CVTTQ_M		0x06f
#define AXP_FUNC_CVTQS_M		0x07c
#define AXP_FUNC_CVTQT_M		0x07e
#define AXP_FUNC_BIC			0x08
#define AXP_FUNC_ADDF			0x080
#define AXP_FUNC_ADDS			0x080
#define AXP_FUNC_SUBS			0x081
#define AXP_FUNC_SUBF			0x081
#define AXP_FUNC_MULS			0x082
#define AXP_FUNC_MULF			0x082
#define AXP_FUNC_DIVF			0x083
#define AXP_FUNC_DIVS			0x083
#define AXP_FUNC_SQRTF			0x08a
#define AXP_FUNC_SQRTS			0x08b
#define AXP_FUNC_SUBL			0x09
#define AXP_FUNC_CVTDG			0x09e
#define AXP_FUNC_ADDT			0x0a0
#define AXP_FUNC_ADDG			0x0a0
#define AXP_FUNC_SUBG			0x0a1
#define AXP_FUNC_SUBT			0x0a1
#define AXP_FUNC_MULG			0x0a2
#define AXP_FUNC_MULT			0x0a2
#define AXP_FUNC_DIVT			0x0a3
#define AXP_FUNC_DIVG			0x0a3
#define AXP_FUNC_CMPTUN			0x0a4
#define AXP_FUNC_CMPGEQ			0x0a5
#define AXP_FUNC_CMPTEQ			0x0a5
#define AXP_FUNC_CMPTLT			0x0a6
#define AXP_FUNC_CMPGLT			0x0a6
#define AXP_FUNC_CMPTLE			0x0a7
#define AXP_FUNC_CMPGLE			0x0a7
#define AXP_FUNC_SQRTG			0x0aa
#define AXP_FUNC_SQRTT			0x0ab
#define AXP_FUNC_CVTGF			0x0ac
#define AXP_FUNC_CVTTS			0x0ac
#define AXP_FUNC_CVTGD			0x0ad
#define AXP_FUNC_CVTTQ			0x0af
#define AXP_FUNC_CVTGQ			0x0af
#define AXP_FUNC_S4SUBL			0x0b
#define AXP_FUNC_INSBL			0x0b
#define AXP_FUNC_CVTQF			0x0bc
#define AXP_FUNC_CVTQS			0x0bc
#define AXP_FUNC_CVTQG			0x0be
#define AXP_FUNC_CVTQT			0x0be
#define AXP_FUNC_ADDS_D			0x0c0
#define AXP_FUNC_SUBS_D			0x0c1
#define AXP_FUNC_MULS_D			0x0c2
#define AXP_FUNC_DIVS_D			0x0c3
#define AXP_FUNC_SQRTS_D		0x0cb
#define AXP_FUNC_ADDT_D			0x0e0
#define AXP_FUNC_SUBT_D			0x0e1
#define AXP_FUNC_MULT_D			0x0e2
#define AXP_FUNC_DIVT_D			0x0e3
#define AXP_FUNC_SQRTT_D		0x0eb
#define AXP_FUNC_CVTTS_D		0x0ec
#define AXP_FUNC_CVTTQ_D		0x0ef
#define AXP_FUNC_CMPBGE			0x0f
#define AXP_FUNC_CVTQS_D		0x0fc
#define AXP_FUNC_CVTQT_D		0x0fe
#define AXP_FUNC_ADDS_UC		0x100
#define AXP_FUNC_ADDF_UC		0x100
#define AXP_FUNC_SUBF_UC		0x101
#define AXP_FUNC_SUBS_UC		0x101
#define AXP_FUNC_MULF_UC		0x102
#define AXP_FUNC_MULS_UC		0x102
#define AXP_FUNC_DIVS_UC		0x103
#define AXP_FUNC_DIVF_UC		0x103
#define AXP_FUNC_SQRTF_UC		0x10a
#define AXP_FUNC_SQRTS_UC		0x10b
#define AXP_FUNC_CVTDG_UC		0x11e
#define AXP_FUNC_S8ADDL			0x12
#define AXP_FUNC_MSKWL			0x12
#define AXP_FUNC_ADDG_UC		0x120
#define AXP_FUNC_ADDT_UC		0x120
#define AXP_FUNC_SUBG_UC		0x121
#define AXP_FUNC_SUBT_UC		0x121
#define AXP_FUNC_MULG_UC		0x122
#define AXP_FUNC_MULT_UC		0x122
#define AXP_FUNC_DIVG_UC		0x123
#define AXP_FUNC_DIVT_UC		0x123
#define AXP_FUNC_SQRTG_UC		0x12a
#define AXP_FUNC_SQRTT_UC		0x12b
#define AXP_FUNC_CVTGF_UC		0x12c
#define AXP_FUNC_CVTTS_UC		0x12c
#define AXP_FUNC_CVTGD_UC		0x12d
#define AXP_FUNC_CVTTQ_VC		0x12f
#define AXP_FUNC_CVTGQ_VC		0x12f
#define AXP_FUNC_CVTQL_V		0x130
#define AXP_FUNC_CMOVLBS		0x14
#define AXP_FUNC_ADDS_UM		0x140
#define AXP_FUNC_SUBS_UM		0x141
#define AXP_FUNC_MULS_UM		0x142
#define AXP_FUNC_DIVS_UM		0x143
#define AXP_FUNC_SQRTS_UM		0x14b
#define AXP_FUNC_EXTWL			0x16
#define AXP_FUNC_CMOVLBC		0x16
#define AXP_FUNC_ADDT_UM		0x160
#define AXP_FUNC_SUBT_UM		0x161
#define AXP_FUNC_MULT_UM		0x162
#define AXP_FUNC_DIVT_UM		0x163
#define AXP_FUNC_SQRTT_UM		0x16b
#define AXP_FUNC_CVTTS_UM		0x16c
#define AXP_FUNC_CVTTQ_VM		0x16f
#define AXP_FUNC_ADDF_U			0x180
#define AXP_FUNC_ADDS_U			0x180
#define AXP_FUNC_SUBF_U			0x181
#define AXP_FUNC_SUBS_U			0x181
#define AXP_FUNC_MULF_U			0x182
#define AXP_FUNC_MULS_U			0x182
#define AXP_FUNC_DIVF_U			0x183
#define AXP_FUNC_DIVS_U			0x183
#define AXP_FUNC_SQRTF_U		0x18a
#define AXP_FUNC_SQRTS_U		0x18b
#define AXP_FUNC_CVTDG_U		0x19e
#define AXP_FUNC_ADDT_U			0x1a0
#define AXP_FUNC_ADDG_U			0x1a0
#define AXP_FUNC_SUBT_U			0x1a1
#define AXP_FUNC_SUBG_U			0x1a1
#define AXP_FUNC_MULG_U			0x1a2
#define AXP_FUNC_MULT_U			0x1a2
#define AXP_FUNC_DIVT_U			0x1a3
#define AXP_FUNC_DIVG_U			0x1a3
#define AXP_FUNC_SQRTG_U		0x1aa
#define AXP_FUNC_SQRTT_U		0x1ab
#define AXP_FUNC_CVTTS_U		0x1ac
#define AXP_FUNC_CVTGF_U		0x1ac
#define AXP_FUNC_CVTGD_U		0x1ad
#define AXP_FUNC_CVTGQ_V		0x1af
#define AXP_FUNC_CVTTQ_V		0x1af
#define AXP_FUNC_S8SUBL			0x1b
#define AXP_FUNC_INSWL			0x1b
#define AXP_FUNC_ADDS_UD		0x1c0
#define AXP_FUNC_SUBS_UD		0x1c1
#define AXP_FUNC_MULS_UD		0x1c2
#define AXP_FUNC_DIVS_UD		0x1c3
#define AXP_FUNC_SQRTS_UD		0x1cB
#define AXP_FUNC_CMPULT			0x1d
#define AXP_FUNC_ADDT_UD		0x1e0
#define AXP_FUNC_SUBT_UD		0x1e1
#define AXP_FUNC_MULT_UD		0x1e2
#define AXP_FUNC_DIVT_UD		0x1e3
#define AXP_FUNC_SQRTT_UD		0x1eb
#define AXP_FUNC_CVTTS_UD		0x1ec
#define AXP_FUNC_CVTTQ_VD		0x1ef
#define AXP_FUNC_ADDQ			0x20
#define AXP_FUNC_BIS			0x20
#define AXP_FUNC_MULQ			0x20
#define AXP_FUNC_MSKLL			0x22
#define AXP_FUNC_S4ADDQ			0x22
#define AXP_FUNC_CMOVEQ			0x24
#define AXP_FUNC_EXTLL			0x26
#define AXP_FUNC_CMOVNE			0x26
#define AXP_FUNC_ORNOT			0x28
#define AXP_FUNC_SUBQ			0x29
#define AXP_FUNC_CVTST			0x2ac
#define AXP_FUNC_S4SUBQ			0x2b
#define AXP_FUNC_INSLL			0x2b
#define AXP_FUNC_CMPEQ			0x2d
#define AXP_FUNC_ZAP			0x30
#define AXP_FUNC_UMULH			0x30
#define AXP_FUNC_CTPOP			0x30
#define AXP_FUNC_ZAPNOT			0x31
#define AXP_FUNC_PERR			0x31
#define AXP_FUNC_CTLZ			0x32
#define AXP_FUNC_S8ADDQ			0x32
#define AXP_FUNC_MSKQL			0x32
#define AXP_FUNC_CTTZ			0x33
#define AXP_FUNC_SRL			0x34
#define AXP_FUNC_UNPKBW			0x34
#define AXP_FUNC_UNPKBL			0x35
#define AXP_FUNC_EXTQL			0x36
#define AXP_FUNC_PKWB			0x36
#define AXP_FUNC_PKLB			0x37
#define AXP_FUNC_MINSB8			0x38
#define AXP_FUNC_MINSW4			0x39
#define AXP_FUNC_SLL			0x39
#define AXP_FUNC_MINUB8			0x3a
#define AXP_FUNC_S8SUBQ			0x3b
#define AXP_FUNC_INSQL			0x3b
#define AXP_FUNC_MINUW4			0x3b
#define AXP_FUNC_SRA			0x3c
#define AXP_FUNC_MAXUB8			0x3c
#define AXP_FUNC_CMPULE			0x3d
#define AXP_FUNC_MAXUW4			0x3d
#define AXP_FUNC_MAXSB8			0x3e
#define AXP_FUNC_MAXSW4			0x3f
#define AXP_FUNC_XOR			0x40
#define AXP_FUNC_ADDL_V			0x40
#define AXP_FUNC_MULL_V			0x40
#define AXP_FUNC_ADDF_SC		0x400
#define AXP_FUNC_MB				0x4000
#define AXP_FUNC_SUBF_SC		0x401
#define AXP_FUNC_MULF_SC		0x402
#define AXP_FUNC_DIVF_SC		0x403
#define AXP_FUNC_SQRTF_SC		0x40a
#define AXP_FUNC_CVTDG_SC		0x41e
#define AXP_FUNC_ADDG_SC		0x420
#define AXP_FUNC_SUBG_SC		0x421
#define AXP_FUNC_MULG_SC		0x422
#define AXP_FUNC_DIVG_SC		0x423
#define AXP_FUNC_SQRTG_SC		0x42a
#define AXP_FUNC_CVTGF_SC		0x42c
#define AXP_FUNC_CVTGD_SC		0x42d
#define AXP_FUNC_CVTGQ_SC		0x42f
#define AXP_FUNC_CMOVLT			0x44
#define AXP_FUNC_WMB			0x4400
#define AXP_FUNC_CMOVGE			0x46
#define AXP_FUNC_EQV			0x48
#define AXP_FUNC_ADDF_S			0x480
#define AXP_FUNC_SUBF_S			0x481
#define AXP_FUNC_MULF_S			0x482
#define AXP_FUNC_DIVF_S			0x483
#define AXP_FUNC_SQRTF_S		0x48a
#define AXP_FUNC_SUBL_V			0x49
#define AXP_FUNC_CVTDG_S		0x49e
#define AXP_FUNC_ADDG_S			0x4a0
#define AXP_FUNC_SUBG_S			0x4a1
#define AXP_FUNC_MULG_S			0x4a2
#define AXP_FUNC_DIVG_S			0x4a3
#define AXP_FUNC_CMPGEQ_S		0x4a5
#define AXP_FUNC_CMPGLT_S		0x4a6
#define AXP_FUNC_CMPGLE_S		0x4a7
#define AXP_FUNC_SQRTG_S		0x4aa
#define AXP_FUNC_CVTGF_S		0x4ac
#define AXP_FUNC_CVTGD_S		0x4ad
#define AXP_FUNC_CVTGQ_S		0x4af
#define AXP_FUNC_CMPLT			0x4d
#define AXP_FUNC_ADDF_SUC		0x500
#define AXP_FUNC_ADDS_SUC		0x500
#define AXP_FUNC_SUBF_SUC		0x501
#define AXP_FUNC_SUBS_SUC		0x501
#define AXP_FUNC_MULF_SUC		0x502
#define AXP_FUNC_MULS_SUC		0x502
#define AXP_FUNC_DIVF_SUC		0x503
#define AXP_FUNC_DIVS_SUC		0x503
#define AXP_FUNC_SQRTF_SUC		0x50a
#define AXP_FUNC_SQRTS_SUC		0x50b
#define AXP_FUNC_CVTDG_SUC		0x51e
#define AXP_FUNC_MSKWH			0x52
#define AXP_FUNC_ADDT_SUC		0x520
#define AXP_FUNC_ADDG_SUC		0x520
#define AXP_FUNC_SUBT_SUC		0x521
#define AXP_FUNC_SUBG_SUC		0x521
#define AXP_FUNC_MULG_SUC		0x522
#define AXP_FUNC_MULT_SUC		0x522
#define AXP_FUNC_DIVT_SUC		0x523
#define AXP_FUNC_DIVG_SUC		0x523
#define AXP_FUNC_SQRTG_SUC		0x52a
#define AXP_FUNC_SQRTT_SUC		0x52b
#define AXP_FUNC_CVTTS_SUC		0x52c
#define AXP_FUNC_CVTGF_SUC		0x52c
#define AXP_FUNC_CVTGD_SUC		0x52d
#define AXP_FUNC_CVTGQ_SVC		0x52f
#define AXP_FUNC_CVTTQ_SVC		0x52f
#define AXP_FUNC_CVTQL_SV		0x530
#define AXP_FUNC_ADDS_SUM		0x540
#define AXP_FUNC_SUBS_SUM		0x541
#define AXP_FUNC_MULS_SUM		0x542
#define AXP_FUNC_DIVS_SUM		0x543
#define AXP_FUNC_SQRTS_SUM		0x54b
#define AXP_FUNC_ADDT_SUM		0x560
#define AXP_FUNC_SUBT_SUM		0x561
#define AXP_FUNC_MULT_SUM		0x562
#define AXP_FUNC_DIVT_SUM		0x563
#define AXP_FUNC_SQRTT_SUM		0x56b
#define AXP_FUNC_CVTTS_SUM		0x56c
#define AXP_FUNC_CVTTQ_SVM		0x56f
#define AXP_FUNC_INSWH			0x57
#define AXP_FUNC_ADDS_SU		0x580
#define AXP_FUNC_ADDF_SU		0x580
#define AXP_FUNC_SUBF_SU		0x581
#define AXP_FUNC_SUBS_SU		0x581
#define AXP_FUNC_MULS_SU		0x582
#define AXP_FUNC_MULF_SU		0x582
#define AXP_FUNC_DIVS_SU		0x583
#define AXP_FUNC_DIVF_SU		0x583
#define AXP_FUNC_SQRTF_SU		0x58a
#define AXP_FUNC_SQRTS_SU		0x58b
#define AXP_FUNC_CVTDG_SU		0x59e
#define AXP_FUNC_EXTWH			0x5a
#define AXP_FUNC_ADDG_SU		0x5a0
#define AXP_FUNC_ADDT_SU		0x5a0
#define AXP_FUNC_SUBT_SU		0x5a1
#define AXP_FUNC_SUBG_SU		0x5a1
#define AXP_FUNC_MULG_SU		0x5a2
#define AXP_FUNC_MULT_SU		0x5a2
#define AXP_FUNC_DIVG_SU		0x5a3
#define AXP_FUNC_DIVT_SU		0x5a3
#define AXP_FUNC_CMPTUN_SU		0x5a4
#define AXP_FUNC_CMPTEQ_SU		0x5a5
#define AXP_FUNC_CMPTLT_SU		0x5a6
#define AXP_FUNC_CMPTLE_SU		0x5a7
#define AXP_FUNC_SQRTG_SU		0x5aa
#define AXP_FUNC_SQRTT_SU		0x5ab
#define AXP_FUNC_CVTTS_SU		0x5ac
#define AXP_FUNC_CVTGF_SU		0x5ac
#define AXP_FUNC_CVTGD_SU		0x5ad
#define AXP_FUNC_CVTGQ_SV		0x5af
#define AXP_FUNC_CVTTQ_SV		0x5af
#define AXP_FUNC_ADDS_SUD		0x5c0
#define AXP_FUNC_SUBS_SUD		0x5c1
#define AXP_FUNC_MULS_SUD		0x5c2
#define AXP_FUNC_DIVS_SUD		0x5c3
#define AXP_FUNC_SQRTS_SUD		0x5cb
#define AXP_FUNC_ADDT_SUD		0x5e0
#define AXP_FUNC_SUBT_SUD		0x5e1
#define AXP_FUNC_MULT_SUD		0x5e2
#define AXP_FUNC_DIVT_SUD		0x5e3
#define AXP_FUNC_SQRTT_SUD		0x5eb
#define AXP_FUNC_CVTTS_SUD		0x5ec
#define AXP_FUNC_CVTTQ_SVD		0x5ef
#define AXP_FUNC_MULQ_V			0x60
#define AXP_FUNC_ADDQ_V			0x60
#define AXP_FUNC_AMASK			0x61
#define AXP_FUNC_MSKLH			0x62
#define AXP_FUNC_CMOVLE			0x64
#define AXP_FUNC_CMOVGT			0x66
#define AXP_FUNC_INSLH			0x67
#define AXP_FUNC_SUBQ_V			0x69
#define AXP_FUNC_EXTLH			0x6a
#define AXP_FUNC_CVTST_S		0x6ac
#define AXP_FUNC_IMPLVER		0x6c
#define AXP_FUNC_CMPLE			0x6d
#define AXP_FUNC_FTOIT			0x70
#define AXP_FUNC_ADDS_SUIC		0x700
#define AXP_FUNC_SUBS_SUIC		0x701
#define AXP_FUNC_MULS_SUIC		0x702
#define AXP_FUNC_DIVS_SUIC		0x703
#define AXP_FUNC_SQRTS_SUIC		0x70b
#define AXP_FUNC_MSKQH			0x72
#define AXP_FUNC_ADDT_SUIC		0x720
#define AXP_FUNC_SUBT_SUIC		0x721
#define AXP_FUNC_MULT_SUIC		0x722
#define AXP_FUNC_DIVT_SUIC		0x723
#define AXP_FUNC_SQRTT_SUIC		0x72b
#define AXP_FUNC_CVTTS_SUIC		0x72c
#define AXP_FUNC_CVTTQ_SVIC		0x72f
#define AXP_FUNC_CVTQS_SUIC		0x73c
#define AXP_FUNC_CVTQT_SUIC		0x73e
#define AXP_FUNC_ADDS_SUIM		0x740
#define AXP_FUNC_SUBS_SUIM		0x741
#define AXP_FUNC_MULS_SUIM		0x742
#define AXP_FUNC_DIVS_SUIM		0x743
#define AXP_FUNC_SQRTS_SUIM		0x74b
#define AXP_FUNC_ADDT_SUIM		0x760
#define AXP_FUNC_SUBT_SUIM		0x761
#define AXP_FUNC_MULT_SUIM		0x762
#define AXP_FUNC_DIVT_SUIM		0x763
#define AXP_FUNC_SQRTT_SUIM		0x76b
#define AXP_FUNC_CVTTS_SUIM		0x76c
#define AXP_FUNC_CVTTQ_SVIM		0x76f
#define AXP_FUNC_INSQH			0x77
#define AXP_FUNC_CVTQS_SUIM		0x77c
#define AXP_FUNC_CVTQT_SUIM		0x77e
#define AXP_FUNC_FTOIS			0x78
#define AXP_FUNC_ADDS_SUI		0x780
#define AXP_FUNC_SUBS_SUI		0x781
#define AXP_FUNC_MULS_SUI		0x782
#define AXP_FUNC_DIVS_SUI		0x783
#define AXP_FUNC_SQRTS_SUI		0x78b
#define AXP_FUNC_EXTQH			0x7a
#define AXP_FUNC_ADDT_SUI		0x7a0
#define AXP_FUNC_SUBT_SUI		0x7a1
#define AXP_FUNC_MULT_SUI		0x7a2
#define AXP_FUNC_DIVT_SUI		0x7a3
#define AXP_FUNC_SQRTT_SUI		0x7ab
#define AXP_FUNC_CVTTS_SUI		0x7ac
#define AXP_FUNC_CVTTQ_SVI		0x7af
#define AXP_FUNC_CVTQS_SUI		0x7bc
#define AXP_FUNC_CVTQT_SUI		0x7be
#define AXP_FUNC_ADDS_SUID		0x7c0
#define AXP_FUNC_SUBS_SUID		0x7c1
#define AXP_FUNC_MULS_SUID		0x7c2
#define AXP_FUNC_DIVS_SUID		0x7c3
#define AXP_FUNC_SQRTS_SUID		0x7cb
#define AXP_FUNC_ADDT_SUID		0x7e0
#define AXP_FUNC_SUBT_SUID		0x7e1
#define AXP_FUNC_MULT_SUID		0x7e2
#define AXP_FUNC_DIVT_SUID		0x7e3
#define AXP_FUNC_SQRTT_SUID		0x7eb
#define AXP_FUNC_CVTTS_SUID		0x7ec
#define AXP_FUNC_CVTTQ_SVID		0x7ef
#define AXP_FUNC_CVTQS_SUID		0x7fc
#define AXP_FUNC_CVTQT_SUID		0x7fe
#define AXP_FUNC_FETCH			0x8000
#define AXP_FUNC_FETCH_M		0xa000
#define AXP_FUNC_RPCC			0xc000
#define AXP_FUNC_RC				0xe000
#define AXP_FUNC_ECB			0xe800
#define AXP_FUNC_RS				0xf000
#define AXP_FUNC_WH64			0xf800
#define AXP_FUNC_WH64EN			0xfc00

/*
 *
 * Table Instruction Name, Pipeline, and Types
 *		U0 = Upper Sub-cluster of Integer Cluster 0
 *		L0 = Lower Sub-cluster of Integer Cluster 0
 *		U1 = Upper Sub-cluster of Integer Cluster 1
 *		L1 = Lower Sub-cluster of Integer Cluster 1
 *		FA = ?
 *		FST0 = ?
 *		FST1 = ?
 *
 *	Class
 *	Name		Pipeline			Instruction Type
 *	--------	------------------	------------------------------------------------------
 *	ild			L0, L1				All integer load instructions
 *	fld			L0, L1				All floating-point load instructions
 *	ist			L0, L1				All integer store instructions
 *	fst			FST0, FST1, L0, L1	All floating-point store instructions
 *	lda			L0, L1, U0, U1		LDA, LDAH
 *	mem_misc	L1					WH64, ECB, WMB
 *	rpcc		L1					RPCC
 *	rx			L1					RS, RC
 *	mxpr		L0, L1				HW_MTPR, HW_MFPR
 *				(depends on IPR)
 *	ibr			U0, U1				Integer conditional branch instructions
 *	jsr			L0					BR, BSR, JMP, CALL, RET, COR, HW_RET,
 *									CALL_PAL
 *	iadd		L0, U0, L1, U1		Instructions with opcode 0x10, except CMPBGE
 *	ilog		L0, U0, L1, U1		AND, BIC, BIS, ORNOT, XOR, EQV, CMPBGE
 *	ishf		U0, U1				Instructions with opcode 0x12
 *	cmov		L0, U0, L1, U1		Integer CMOV - either cluster
 *	imul		U1					Integer multiply instructions
 *	imisc		U0					PERR, MINxxx, MAXxxx, PKxx, UNPKxx
 *	fbr			FA					Floating-point conditional branch instructions
 *	fadd		FA					All floating-point operate instructions except multiply,
 *									divide, square root, and conditional move instructions
 *	fmul		FM					Floating-point multiply instruction
 *	fcmov1		FA					Floating-point CMOV first half
 *	fcmov2		FA					Floating-point CMOV second half
 *	fdiv		FA					Floating-point divide instruction
 *	fsqrt		FA					Floating-point square root instruction
 *	nop			None				TRAP, EXCB, UNOP - LDQ_U R31, 0(Rx)
 *	ftoi		FST0, FST1, L0, L1	FTOIS, FTOIT
 *	itof		L0, L1				ITOFS, ITOFF, ITOFT
 *	mx_fpcr		FM					Instructions that move data from the floating-point
 *									control register
 */
typedef enum
{
	PipelineNone,	/* R/F31 as a dest	*/
	EboxU0,			/* U0				*/
	EboxU1,			/* U1				*/
	EboxU0U1,		/* U0, U1			*/
	EboxL0,			/* L0				*/
	EboxL1,			/* L1				*/
	EboxL0L1,		/* L0, L1			*/
	EboxL0L1U0U1,	/* L0, L1, U0, U1	*/
	FboxMul,		/* FM				*/
	FboxOther		/* FA, FST0, FST1	*/
} AXP_PIPELINE;

#define AXP_U0_PIPELINE			0
#define AXP_U1_PIPELINE			1
#define AXP_L0_PIPELINE			2
#define AXP_L1_PIPELINE			3
#define AXP_EBOX_PIPELINE_MAX	4

typedef enum
{
	Retired,
	Queued,
	Executing,
	WaitingRetirement,
	Aborted
} AXP_INS_STATE;

/*
 * This structure is what will be put into the Reorder Buffer.  A queue entry
 * in the Integer or Floating-point Queues (IQ or FQ) will point to the queue
 * entry.  It contains a single decoded instruction that has had it's
 * architectural registers renamed to physical ones.  And a state value
 * indicating if the instruction is Queued, Executing, WaitingRetirement, or
 * Retired.
 */
typedef struct
{
	AXP_REGISTER	src1v;		/* Value from src1 register */
	AXP_REGISTER	src2v;		/* Value from src2 register */
	AXP_REGISTER	destv;		/* Value to dest register */
	AXP_INS_TYPE	format;		/* Instruction format */
	AXP_OPER_TYPE	type;
	AXP_REG_DECODE	decodedReg;	/* which registers are used for what */
	AXP_PC			pc;
	AXP_PC			branchPC;
	AXP_INS_STATE	state;
	AXP_FBOX_FPCR	insFpcr;
	AXP_IBOX_EXC_SUM excSum;
	AXP_EXCEPTIONS	excRegMask;	/* Exception Register Mask */
	AXP_INS_FMT		instr;
	i64				displacement;/* Displacement from PC + 4 */
	u64				literal;	/* Literal value */
	u64				prevDestValue;/* Previous value in destination register */
	u32				function;	/* Function code for operation */
	u32				slot;		/* Assigned Load/Store slot */
	u16				aSrc1;		/* Architectural register R0-R30 or F0-F30 */
	u16				src1;		/* Physical register PR0-PR79, PF0-PF71 */
	u16				aSrc2;		/* Architectural register R0-R30 or F0-F30 */
	u16				src2;		/* Physical register PR0-PR79, PF0-PF71 */
	u16				aDest;		/* Architectural register R0-R30 or F0-F30 */
	u16				dest;		/* Physical register PR0-PR79, PF0-PF71 */
	u16				prevDestMap;/* Previous physical register mapping */
	u8				uniqueID;	/* A unique id for each instruction */
	u8				opcode;		/* Operation code */
	u8				type_hint_index; /* HW_LD/ST type, HW_RET hint, HW_MxPR idx */
	u8				scbdMask;	/* HW_MxPR scbd_mask */
	bool			useLiteral;	/* Indicator that the literal value is valid */
	bool			branchPredict; /* Prediction logic result */
	bool			localPredict; /* Local branch predict */
	bool			globalPredict; /* Global branch predict */
	bool			stall;		/* Stall Ibox until IQ/FQ are empty */
	bool			quadword;	/* HW_LD/ST len */
} AXP_INSTRUCTION;

#endif /* _AXP_21264_INS_DEFS_ */
