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
 *	This header file contains the structures and definitions required to
 *	implement the instruction emulation for the Alpha 21264 (EV68) processor.
 *
 *	Revision History:
 *
 *	V01.000		04-May-2017	Jonathan D. Belanger
 *	Initially written.
 *	(May the 4th be with you).
 *
 *	V01.001		14-May-2017	Jonathan D. Belanger
 *	Included the AXP_Base_CPU header file and a location for the Program
 *	Counter (PC) in the CPU record.
 */
#ifndef _AXP_21264_CPU_DEFS_
#define _AXP_21264_CPU_DEFS_

#include "AXP_Blocks.h"
#include "AXP_Base_CPU.h"
#include "AXP_21264_IPRs.h"
#include "AXP_21264_Predictions.h"
#include "AXP_21264_Instructions.h"
#include "AXP_21264_ICache.h"

#define AXP_RESULTS_REG		41
#define AXP_NUM_FETCH_INS	4
#define AXP_IQ_LEN			20
#define AXP_FQ_LEN			15
#define AXP_SHADOW_REG		8
#define AXP_R04_SHADOW		(AXP_MAX_REGISTERS + 0)
#define AXP_R05_SHADOW		(AXP_MAX_REGISTERS + 1)
#define AXP_R06_SHADOW		(AXP_MAX_REGISTERS + 2)
#define AXP_R07_SHADOW		(AXP_MAX_REGISTERS + 3)
#define AXP_R20_SHADOW		(AXP_MAX_REGISTERS + 4)
#define AXP_R21_SHADOW		(AXP_MAX_REGISTERS + 5)
#define AXP_R22_SHADOW		(AXP_MAX_REGISTERS + 6)
#define AXP_R23_SHADOW		(AXP_MAX_REGISTERS + 7)
#define AXP_TB_LEN			128
#define AXP_ICB_INS_CNT		16
#define AXP_21264_PAGE_SIZE	8192	// 8KB page size
#define AXP_INT_PHYS_REG	AXP_MAX_REGISTERS + AXP_SHADOW_REG + AXP_RESULTS_REG - 1
#define AXP_FP_PHYS_REG		AXP_MAX_REGISTERS + AXP_RESULTS_REG - 1

/*
 * This structure is a buffer to contain the next set of instructions to get
 * queued up for execution.
 */
typedef struct
{
	bool			branch2bTaken;
	bool			linePrediction;
	bool			setPrediction;
	u64				retPredStack;
	AXP_INS_FMT		instructions[AXP_NUM_FETCH_INS];
	AXP_INS_TYPE	instrType[AXP_NUM_FETCH_INS];
	AXP_PC			instrPC[AXP_NUM_FETCH_INS];
} AXP_INS_LINE;

/*
 * This structure is what will be queued to the Integer or Floating-point
 * Queue (IQ or FQ) for execution.  It contains a single decoded instruction
 * that has had it's architectural registers renamed to physical ones.
 */
#define AXP_UNMAPPED_REG	31	/* R31 and F31 are never renamed/mapped */
typedef struct
{
	u8				opcode;		/* Operation code */
	u8				aSrc1;		/* Architectural register R0-R30 or F0-F30 */
	u8				src1;		/* Physical register PR0-PR79, PF0-PF71 */
	u8				aSrc2;		/* Architectural register R0-R30 or F0-F30 */
	u8				src2;		/* Physical register PR0-PR79, PF0-PF71 */
	u8				aDest;		/* Architectural register R0-R30 or F0-F30 */
	u8				dest;		/* Physical register PR0-PR79, PF0-PF71 */
	bool			useLiteral;	/* Indicator that the literal value is valid */
	u32				function;	/* Function code for operation */
	i64				displacement;/* Displacement from PC + 4 */
	u64				literal;	/* Literal value */
	AXP_INS_TYPE	format;		/* Instruction format */
	AXP_OPER_TYPE	type;
	AXP_PC			pc;
} AXP_INSTRUCTION;

typedef struct
{
	/*
	 * This structure needs to be at the top of all data blocks/structures
	 * that need to be specifically allocated by the Blocks module.
	 */
	AXP_BLOCK_DSC header;

	/**************************************************************************
	 *	Ibox Definitions													  *
	 *																		  *
	 *	The Ibox is responsible for instuction processing.  It maintains the  *
	 *	VPC Queue, ITB, Branch Prediction, Instruction Predecode, Instruction *
	 *	decode and register renaming, Instruction Cache, Instruction		  *
	 *	Retirement, and the Integer and Floating-Point Instruction Queues.	  *
	 *																		  *
	 *	The Ibox interfaces with with the Cbox, Ebox, and Fbox.  The Cbox	  *
	 *	provides the next set of instructions when an Icache miss occurs.	  *
	 *	The set of instructions are provided to the Ibox for predecoding and  *
	 *	entry into the Icache.  The Ebox reads instructions of the Integer	  *
	 *	Issue Queue (IQ) into upto 4 integer processors.  The Fbox reads	  *
	 *	instructions from the FP Issue Queue (FQ) into upto 2 FP processors.  *
	 **************************************************************************/

	/*
	 * The following definitions are used by the branch prediction code.
	 */
	LHT			localHistoryTable;
	LPT			localPredictor;
	GPT			globalPredictor;
	CPT			choicePredictor;
	u16			globalPathHistory;

	/*
	 * Architectural (virtual) registers.
	 */
	u64			r[AXP_MAX_REGISTERS + AXP_SHADOW_REG];	/* Integer (Virt) Reg */
	u64 		f[AXP_MAX_REGISTERS];	/* Floating-point (Virtual) Registers */

	/*
	 * Virtual Program Counter Queue
	 */
	AXP_PC		vpc[AXP_IQ_LEN];
	u32			vpcIdx;

	/*
	 * Instruction Queues (Integer and Floating-Point).
	 */
	AXP_INSTRUCTION	iq[AXP_IQ_LEN];
	u32				iqStart;
	u32				iqEnd;
	AXP_INSTRUCTION	fq[AXP_FQ_LEN];
	u32				fqStart;
	u32				fqEnd;

	/*
	 * Ibox Internal Processor Registers (IPRs)
	 */
	AXP_IBOX_ITB_TAG	itbTag;		/* ITB tag array write					*/
	AXP_IBOX_ITB_IS		itbIs;		/* ITB invalidate single				*/
	AXP_IBOX_EXC_ADDR	exeAddr;	/* Exception address					*/
	AXP_IBOX_IVA_FORM	ivaForm;	/* Instruction VA format				*/
	AXP_IBOX_IER_CM		ierCm;		/* Interrupt enable and current mode	*/
	AXP_IBOX_SIRR		sirr;		/* Software interrupt request			*/
	AXP_IBOX_ISUM		iSum;		/* Interrupt summary					*/
	AXP_IBOX_HW_INT_CLR	hwIntClr;	/* Hardware interrupt clear				*/
	AXP_IBOX_EXC_SUM	excSum;		/* Exception summary					*/
	AXP_IBOX_PAL_BASE	palBase;	/* PAL base address						*/
	AXP_IBOX_I_CTL		iCtl;		/* Ibox control							*/
	AXP_IBOX_I_STAT		iStat;		/* Ibox status							*/
	AXP_IBOX_PCTX		pCtx;		/* Process context register				*/
	AXP_IBOX_PCTR_CTL	pCtrCtl;	/* Performance counter control			*/

	/*
	 * This is the Instruction Cache.  It is 64K bytes in size.  For this emulation
	 * each line/block is 128 bytes in size, which makes for 512 total entries, with
	 * 256 for each association (this is a 2-way cache).
	 */
	AXP_ICACHE_LINE 	iCache[AXP_21264_ICACHE_SIZE][AXP_2_WAY_ICACHE];

	/*
	 * This is the Instruction Address Translation (Look-aside) Table (ITB).
	 * It is 128 entries in size, and is allocated in a round-robin scheme.
	 * We, therefore, maintain a start and end index (both start at 0).
	 */
	AXP_ICACHE_ITB		itb[AXP_TB_LEN];
	u32					itbStart;	/* Since round-robin, need to know start */
	u32					itbEnd;		/* and end entries in the ITB */

	/**************************************************************************
	 *	Ebox Definitions													  *
	 *																		  *
	 *	The Ebox is responsible for processing instructions from the IQ.  It  *
	 *	maintains 2 sets of Physical Integer Registers, which are copies of	  *
	 *	one another.  It can handle upto 4 simultaneous instructions.		  *
	 *																		  *
	 *	The Ebox interfaces with the Ibox (see above for more information),	  *
	 *	the Fbox and the Mbox.  The Fbox and Ebox are allowed to move values  *
	 *	from a register in one to the other.  This is does for Integer/FP to  *
	 *	FP/Integer conversion and FP branch operations.  The Mbox provides	  *
	 *	data to the Ebox from memory, via that data cache (Dcache).			  *
	 **************************************************************************/

	/*
	 * Physical registers.
	 *
	 * There are 80 register file entries for integer registers.  This is the
	 * 31 Integer registers (R31 is not stored), plus the 8 shadow registers,
	 * plus the 41 results for instructions that can potentially have not been
	 * retired.
	 *
	 * In a real implementation, there would be 2 register files for each
	 * integer pipe.  This is to simplify the data paths needed in silicon.
	 * In silicon, both files are maintained with the same contents, and when
	 * renaming registers, the entry in both files are considered one.
	 *
	 * In this emulation, there is only 1 register file to be used by both
	 * integer pipes.  This simplifies the coding needed to keep 2 files
	 * constantly synchronized.
	 */
	u64			pr[AXP_INT_PHYS_REG];

	/*
	 * Ebox IPRs
	 */
	AXP_EBOX_CC			cc;			/* Cycle counter						*/
	AXP_EBOX_CC_CTL		ccCtl;		/* Cycle counter control				*/
	AXP_EBOX_VA			va;			/* Virtual address						*/
	AXP_EBOX_VA_CTL		vaCtl;		/* Virtual address control				*/
	AXP_EBOX_VA_FORM	vaForm;		/* Virtual address format				*/

	/**************************************************************************
	 *	Fbox Definitions													  *
	 *																		  *
	 *	The Fbox is responsible for processing instructions from the FQ.  It  *
	 *	maintains a sets of Physical Integer Registers.  It can handle upto 2 *
	 *	simultaneous instructions.											  *
	 *																		  *
	 *	The Fbox interfaces with the Ibox (see above for more information),	  *
	 *	the Ebox (See above for more information) and the Mbox.  The Mbox	  *
	 *	provides data to the Fbox from memory, via that data cache (Dcache).  *
	 **************************************************************************/

	/*
	 * Physical registers.
	 *
	 * There are 72 register file entries for the floating-point registers.
	 * This is the 31 Floating-point registers (F31 is not stored), plus the 41
	 * results for instructions that can potentially have not been retired.
	 *
	 * Since the floating-point execution unit only has 1 cluster, there is 
	 * just 1 set of 72 registers.
	 */
	u64			pf[AXP_MAX_REGISTERS + AXP_RESULTS_REG - 1];

	/**************************************************************************
	 *	Mbox Definitions													  *
	 *																		  *
	 *	The Mbox is responsible for providing data to the Ebox and Fbox.  The *
	 *	Mbox maintins a Load and Stor Queue, as well as a Miss Address File.  *
	 *																		  *
	 *	The Mbox interfaces with with the Cbox, Ebox, and Fbox (see above for *
	 *	more information on the last 2).  The Cbox provides data when a		  *
	 *	Dcache miss occurs.  The Mbox provides data to the Cbox to store in	  *
	 *	memory when a store operation occirs.								  *
	 **************************************************************************/
	u8			lq;
	u8			sq;
	u8			maf;
	u8			dtb[AXP_TB_LEN];

	/*
	 * Mbox IPRs
	 */
	AXP_MBOX_DTB_TAG		dtbTag0;	/* DTB tag array write 0			*/
	AXP_MBOX_DTB_TAG		dtbTag1;	/* DTB tag array write 1			*/
	AXP_MBOX_DTB_PTE		dtbPte0;	/* DTB PTE array write 0			*/
	AXP_MBOX_DTB_PTE		dtbPte1;	/* DTB PTE array write 1			*/
	AXP_MBOX_DTB_ALTMODE	dtbAltMode;	/* DTB alternate processor mode		*/
	AXP_MBOX_DTB_IS			dtbIs0;		/* DTB invalidate single 0			*/
	AXP_MBOX_DTB_IS			dtbIs1;		/* DTB invalidate single 1			*/
	AXP_MBOX_DTB_ASN		dtbAsn0;	/* DTB address space number 0		*/
	AXP_MBOX_DTB_ASN		dtbAsn1;	/* DTN address space number 1		*/
	AXP_MBOX_MM_STAT		mmStat;		/* Memory management status			*/
	AXP_MBOX_M_CTL			mCtl;		/* Mbox control						*/
	AXP_MBOX_DC_CTL			dcCtl;		/* Dcache control					*/
	AXP_MBOX_DC_STAT		dcStat;		/* Dcache status					*/

	/**************************************************************************
	 *	Cbox Definitions													  *
	 *																		  *
	 *	The Cbox is responsible for interfacing with the system.  It		  *
	 *	maintains a Probe Queue, Duplicate Tag Store, I/O Write Buffer		  *
	 *	(IOWB), Victim Buffer, and Arbiter.  It interfaces with the System	  *
	 *	(memory, disk drives, I/O devices, etc.), Ibox and Mbox (see above	  *
	 *	for more information about the last 2 items).						  *
	 *																		  *
	 *	The Cbox is responsible for the interfaces between the system and the *
	 *	CPU.																  *
	 **************************************************************************/
	u8			vaf;
	u8			vdf;
	u8			iowb;
	u8			pq;
	u8			dtag;

	/*
	 * Cbox IPRs
	 */
	AXP_CBOX_C_DATA		cData;		/* Cbox data							*/
	AXP_CBOX_C_SHFT		cShft;		/* Cbox shift control					*/

	/*
	 * Alpha AXP Architectural IPRs
	 */
	AXP_BASE_ASN		asn;		/* Address Space Number					*/
	AXP_BASE_ASTEN		astEn;		/* AST Enable							*/
	AXP_BASE_ASTSR		astSr;		/* AST Summary Register					*/
	AXP_BASE_DATFX		datFx;		/* Data Alignment Trap Fixup			*/
	AXP_BASE_ESP		esp;		/* Executive Stack Pointer				*/
	AXP_BASE_FEN		fen;		/* Floating-point Enable				*/
	AXP_BASE_IPIR		ipIr;		/* Interprocessor Interrupt Request		*/
	AXP_BASE_IPL		ipl;		/* Interrupt Priority Level				*/
	AXP_BASE_KSP		ksp;		/* Kernel Stack Pointer					*/
	AXP_BASE_MCES		mces;		/* Machine Check Error Summary			*/
	AXP_BASE_PCBB		pcbb;		/* Privileged Context Block Base		*/
	AXP_BASE_PRBR		prbr;		/* Processor Base Register				*/
	AXP_BASE_PTBR		ptbr;		/* Page Table Base Register				*/
	AXP_BASE_SCBB		scbb;		/* System Control Block Base			*/
	AXP_BASE_SISR		sisr;		/* Software Interrupt Summary Register	*/
	AXP_BASE_SSP		ssp;		/* Supervisor Stack Pointer				*/
	AXP_BASE_SYSPTBR	sysPtbr;	/* System Page Table Base				*/
	AXP_BASE_TBCHK		tbChk;		/* TB Check								*/
	AXP_BASE_USP		usp;		/* User Stack Pointer					*/
	AXP_BASE_VIRBND		virBnd;		/* Virtual Address Boundary				*/
	AXP_BASE_VPTB		vptb;		/* Virtual Page Table Base				*/
	AXP_BASE_WHAMI		whami;		/* Who-Am-I								*/

} AXP_21264_CPU;
#endif /* _AXP_21264_CPU_DEFS_ */
