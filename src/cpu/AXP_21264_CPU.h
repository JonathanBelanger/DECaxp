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
 *
 *	V01.002		11-Jun-2017	Jonathan D. Belanger
 *	Got rid of the architectural registers (R0-R31 and F0-F31) and moved some
 *	items around so that memory copying is minimized.
 *
 *	V01.003		12-Jun-2017	Jonathan D. Belanger
 *	Forgot the Unique ID counter to be assigned to each instruction.  Also, I'm
 *	rethinking the use of queues for the IQ and FQ.  I'm thinking a wrap-around
 *	list would be just as good and also avoid the need to allocate and
 *	initialize memory every time we have to decode an instruction.
 *
 *	V01.004		28-Jun-2017	Jonathan D. Belanger
 *	Added an include for the Mbox Load/Store Queue structure definitions.
 *	Changed the physical registers (pr and pf) back to a u64 type.  Because of
 *	the out-of-order instruction execution, register values are copied out of
 *	the register into a location where it is utilized, and interpreted beyond
 *	an unsigned 64-bit integer, and then stored back into the physical register
 *	when the instruction is retired, the physical registers are just an place
 *	to hold the value for the next instruction to copy.
 *
 *	V01.005		19-Jul-2017	Jonathan D. Belanger
 *	Added the VAXintrFlag for support of the RC and RC VAX Compatibility
 *	instructions.
 *
 *	V01.006		20-Jul-2017	Jonathan D. Belanger
 *	Added locations for the Processor Cycle Counter, AMASK and IMPLVER
 *	registers.
 */
#ifndef _AXP_21264_CPU_DEFS_
#define _AXP_21264_CPU_DEFS_

#include "AXP_Utility.h"
#include "AXP_Blocks.h"
#include "AXP_Base_CPU.h"
#include "AXP_21264_Instructions.h"
#include "AXP_21264_Predictions.h"
#include "AXP_21264_IPRs.h"
#include "AXP_21264_CacheDefs.h"
#include "AXP_21264_RegisterRenaming.h"
#include "AXP_21264_Mbox_Queues.h"
#include "AXP_Exceptions.h"

#define AXP_RESULTS_REG		41
#define AXP_NUM_FETCH_INS	4
#define AXP_IQ_LEN			20
#define AXP_FQ_LEN			15
#define AXP_SHADOW_REG		8
#define AXP_R04_SHADOW		(AXP_MAX_REGISTERS + 0)		/* R32 */
#define AXP_R05_SHADOW		(AXP_MAX_REGISTERS + 1)		/* R33 */
#define AXP_R06_SHADOW		(AXP_MAX_REGISTERS + 2)		/* R34 */
#define AXP_R07_SHADOW		(AXP_MAX_REGISTERS + 3)		/* R35 */
#define AXP_R20_SHADOW		(AXP_MAX_REGISTERS + 4)		/* R36 */
#define AXP_R21_SHADOW		(AXP_MAX_REGISTERS + 5)		/* R37 */
#define AXP_R22_SHADOW		(AXP_MAX_REGISTERS + 6)		/* R38 */
#define AXP_R23_SHADOW		(AXP_MAX_REGISTERS + 7)		/* R39 */
#define AXP_REG(regNum, palMode)											\
		((palMode) ?														\
		 (((cpu->iCtl.sde & AXP_I_CTL_SDE_ENABLE) != 0) ?					\
		  ((((regNum) >= 4) && ((regNum) <= 7)) ?							\
		   ((regNum) + 28) : ((((regNum) >= 20) && ((regNum) <= 23)) ?		\
			((regNum) + 16) : (regNum))) : (regNum)) : (regNum))
#define AXP_TB_LEN			128
#define AXP_ICB_INS_CNT		16
#define AXP_21264_PAGE_SIZE	8192	/* 8KB page size */
#define AXP_21264_MEM_BITS	44
#define AXP_INT_PHYS_REG	AXP_MAX_REGISTERS + AXP_SHADOW_REG + AXP_RESULTS_REG - 1
#define AXP_FP_PHYS_REG		AXP_MAX_REGISTERS + AXP_RESULTS_REG - 1
#define AXP_UNMAPPED_REG	31	/* R31 and F31 are never renamed/mapped */
#define AXP_I_FREELIST_SIZE	AXP_INT_PHYS_REG - AXP_MAX_REGISTERS - 1
#define AXP_F_FREELIST_SIZE	AXP_FP_PHYS_REG - AXP_MAX_REGISTERS - 1
#define AXP_INFLIGHT_FETCHES 20
#define AXP_INFLIGHT_MAX	80
#define AXP_MBOX_QUEUE_LEN	32
#define AXP_CACHE_ENTRIES	512
#define AXP_2_WAY_CACHE	2

/*
 * PALcode Exception Entry Points
 * 	When hardware encounters an exception, Ibox execution jumps to a PALcode
 * 	entry point at a PC determined by the type of exception.  The return PC of
 * 	the instruction the triggered the exception is placed in the EXC_ADDR
 * 	register and onto the return prediction stack.  The entry points are listed
 * 	in decreasing order of priority.
 *
 *		Entry Name		Type		Offset	Description
 *		DTBM_DOUBLE_3	Fault		0x100	Dstream TB miss on virtual page table
 *											entry fetch.  Use three-level flow.
 *		DTBM_DOUBLE_4	Fault		0x180	Dstream TB miss on virtual page table
 *											entry fetch.  Use four-level flow.
 *		FEN				Fault		0x200	Floating point disabled.
 *		UNALIGNED		Fault		0x280	Unaligned Dstream reference.
 *		DTBM_SINGLE		Fault		0x300	Dstream TB miss.
 *		DFAULT			Fault		0x380	Dstream fault or virtual address sign
 *											check error.
 *		OPCDEC			Fault		0x400	Illegal opcode or function field:
 *											- Opcode 1, 2, 3, 4, 5, 6 or 7
 *											- Opcode 0x19, 0x1b, 0x1d, 0x1e or
 *											  0x1f not PALmode or not
 *											  I_CTL[HWE]
 *											- Extended precision IEEE format
 *											- Unimplemented function field of
 *											  opcodes 0x14 or 0x1c.
 *		IACV			Fault		0x480	Istream access violation or virtual
 *											address sign check error.
 *		MCHK			Interrupt	0x500	Machine check.
 *		ITB_MISS		Fault		0x580	Istream TB miss.
 *		ARITH			Synch. Trap	0x600	Arithmetic exception or update to
 *											FPCR.
 *		INTERRUPT		Interrupt	0x680	Interrupt: hardware, software, and
 *											AST.
 *		MT_FPCR			Synch. Trap	0x700	Invoked when an MT_FPCR instruction
 *											is issued.
 *		RESET/WAKEUP	Interrupt	0x780	Chip reset or wake-up from sleep
 *											mode.
 */
#define AXP_DTBM_DOUBLE_3	0x0100
#define AXP_DTBM_DOUBLE_4	0x0180
#define AXP_FEN				0x0200
#define AXP_UNALIGNED		0x0280
#define AXP_DTBM_SINGLE		0x0300
#define AXP_DFAULT			0x0380
#define AXP_OPCDEC			0x0400
#define AXP_IACV			0x0480
#define AXP_MCHK			0x0500
#define AXP_ITB_MISS		0x0580
#define AXP_ARITH			0x0600
#define AXP_INTERRUPT		0x0680
#define AXP_MT_FPCR_TRAP	0x0700
#define AXP_RESET_WAKEUP	0x0780

/*
 * This structure is a buffer to contain the next set of instructions to get
 * queued up for execution.
 */
typedef struct
{
	bool			branch2bTaken;
	u32				linePrediction;
	u32				setPrediction;
	u64				retPredStack;
	AXP_INS_FMT		instructions[AXP_NUM_FETCH_INS];
	AXP_INS_TYPE	instrType[AXP_NUM_FETCH_INS];
	AXP_PC			instrPC[AXP_NUM_FETCH_INS];
} AXP_INS_LINE;

typedef struct
{
	AXP_CQUE_ENTRY		header;
	AXP_INSTRUCTION		*ins;
	u32					index;
} AXP_QUEUE_ENTRY;

/*
 * The following states are used during CPU execution.  The state transitions
 * are as follows:
 *
 * 		Current State	Event							Next State
 * 		-------------	----------------------------	------------
 * 		Cold 			Complete CPU initialization		WaitBiST
 * 		WaitBiST 		Complete BiST					Run
 * 		Run 			Fault Reset_L 					FaultReset
 * 		Run 			SleepMode						Sleep
 * 		Run 			Shutdown 						ShuttingDown
 * 		FaultReset 		Complete CPU Reset				Run
 * 		Sleep 			Wake 							WaitBiSI
 * 		WaitBiSI 		Complete self initialization 	Run
 * 		ShuttingDown	shutdown complete				EXIT(1)
 * 		(1) EXIT = Delete CPU main thread (shutdown already deleted the others)
 */
typedef enum
{
	Cold,
	WaitBiST,
	WaitBiSI,
	Run,
	FaultReset,
	Sleep,
	ShutingDown
} AXP_21264_STATES;
typedef enum
{
	InitializationComplete,
	BiSTComplete,
	FaultResetL,
	SleepMode,
	Wake,
	BiSIComplete,
	Shutdown
} AXP_21264_EVENTS;;

/*
 * The following enumerations should probably go someplace else.  See the
 * Miscellaneous section in the SPU structure, that should probably also be
 * relocated.
 * TODO: Find the real home for these definitions.
 */
typedef enum
{
	EV3 = 1,
	EV4,					/* 21064 */
	Simulation,
	LCAFamily,				/* 21066, 21068, 20166A, 20168A */
	EV5,					/* 21164 */
	EV45,					/* 21064A */
	EV56,					/* 21164A */
	EV6,					/* 21264 */
	PCA56,					/* 21164PC */
	PCA57,
	EV67,					/* 21264 */
	EV68CB_DC,				/* 21264 */
	EV68A,					/* 21264 */
	EV68CX,					/* 21264 */
	EV7,					/* 21364 */
	EV79,					/* 21364 */
	EV69A					/* 21264 */
} AXP_PROC_MAJ_TYPE;
#define	AXP_PASS_2_21_EV4		0	/* EV4 */
#define AXP_PASS_3_EV4			1
#define AXP_RESERVED			0	/* LCA Family */
#define AXP_PASS_1_11_66		1
#define AXP_PASS_2_66			2
#define AXP_PASS_1_11_68		3
#define AXP_PASS_2_68			4
#define AXP_PASS_1_66A			5
#define AXP_PASS_1_68A			6
#define AXP_PASS_2_22			1	/* EV5 */
#define AXP_PASS_23_EV5			2
#define AXP_PASS_3_EV5			3
#define AXP_PASS_32				4
#define AXP_PASS_4_EV5			5
#define AXP_PASS_1				1	/* EV45 */
#define AXP_PASS_11				2
#define AXP_PASS_1_11			6
#define AXP_PASS_2_EV45			3
#define AXP_PASS_2_EV56			2
#define	AXP_PASS_2_21			2	/* EV6 */
#define AXP_PASS_22_EV6			3
#define AXP_PASS_23_EV6			4
#define AXP_PASS_3_EV6			5
#define AXP_PASS_24_EV6			6
#define AXP_PASS_25_EV6			7
#define AXP_PASS_21				2	/* EV67 */
#define AXP_PASS_211			4
#define AXP_PASS_221			5
#define AXP_PASS_23_24			6
#define AXP_PASS_212			7
#define AXP_PASS_222			8
#define AXP_PASS_223_225		9
#define AXP_PASS_224			10
#define AXP_PASS_25_EV67		11
#define AXP_PASS_241			12
#define AXP_PASS_251			13
#define AXP_PASS_26				14
#define AXP_PASS_22_23			3	/* EV68CB */
#define AXP_PASS_3_31			4
#define AXP_PASS_24				5
#define AXP_PASS_4				6
#define AXP_PASS_2_EV68DC		2	/* EV68DC */
#define AXP_PASS_231			3
#define AXP_PASS_214_EV68DC		4
#define AXP_PASS_2_EV68A		2	/* EV68A */
#define AXP_PASS_21_21A_3		3
#define AXP_PASS_22_EV68A		4

/*
 * This structure contains all the fields required to emulate an Alpha AXP
 * 21264 CPU.
 */
typedef struct
{
	/*
	 * This field needs to be at the top of all data blocks/structures
	 * that need to be specifically allocated by the Blocks module.
	 */
	AXP_BLOCK_DSC header;

	/*
	 * CPU state.
	 */
	AXP_21264_STATES cpuState;

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

	u8			instrCounter;		/* Unique ID for each instruction		*/
	u8			res_1;				/* Align to 32-bit boundary				*/

	/*
	 * This is equivalent to the VPC
	 */
	AXP_PC				vpc[AXP_INFLIGHT_MAX];
	u32					vpcStart;
	u32					vpcEnd;

	/*
	 * Reorder Buffer
	 */
	AXP_INSTRUCTION 	rob[AXP_INFLIGHT_MAX];
	u32					robStart;
	u32					robEnd;

	/*
	 * Instruction Queues (Integer and Floating-Point).
	 */
	AXP_COUNTED_QUEUE	iq;
	AXP_COUNTED_QUEUE	fq;

	/*
	 * Instruction Queue Pre-allocated Cache.
	 */
	AXP_QUEUE_ENTRY		iqEntries[AXP_IQ_LEN];
	u32					iqEFreelist[AXP_IQ_LEN];
	u32					iqEFlStart;
	u32					iqEFlEnd;
	AXP_QUEUE_ENTRY		fqEntries[AXP_FQ_LEN];
	u32					fqEFreelist[AXP_IQ_LEN];
	u32					fqEFlStart;
	u32					fqEFlEnd;

	/*
	 * Ibox Internal Processor Registers (IPRs)
	 */
	AXP_IBOX_ITB_TAG	itbTag;		/* ITB tag array write					*/
	AXP_IBOX_ITB_PTE	itbPte;		/* ITB Page Table Entry					*/
	AXP_IBOX_ITB_IS		itbIs;		/* ITB invalidate single				*/
	AXP_IBOX_EXC_ADDR	excAddr;	/* Exception address					*/
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
	 * This is the Instruction Cache.  It is 64K bytes in size (just counting
	 * the actual instructions (16 of them) in the cache block and not the bits
	 * needed to support it.  Each cache block contains 16 instructions, each 4
	 * bytes long, for a total of 64 bytes.  There are 2 sets of cache, which
	 * leaves us with 64KB / (16 * 4B) / 2 sets = 512 rows for each set.
	 */
	AXP_ICACHE_BLK		iCache[AXP_CACHE_ENTRIES][AXP_2_WAY_CACHE];

	/*
	 * This is the Instruction Address Translation (Look-aside) Table (ITB).
	 * It is 128 entries in size, and is allocated in a round-robin scheme.
	 * We, therefore, maintain a start and end index (both start at 0).
	 */
	AXP_21264_TLB		itb[AXP_TB_LEN];
	u32					nextITB;

	/**************************************************************************
	 *	Ebox Definitions													  *
	 *																		  *
	 *	The Ebox is responsible for processing instructions from the IQ.  It  *
	 *	maintains 2 sets of Physical Integer Registers, which are copies of	  *
	 *	one another.  It can handle up to 4 simultaneous instructions.		  *
	 *																		  *
	 *	The Ebox interfaces with the Ibox (see above for more information),	  *
	 *	the Fbox and the Mbox.  The Fbox and Ebox are allowed to move values  *
	 *	from a register in one to the other.  This is does for Integer/FP to  *
	 *	FP/Integer conversion and FP branch operations.  The Mbox provides	  *
	 *	data to the Ebox from memory, via that data cache (Dcache).			  *
	 **************************************************************************/

	/*
	 * VAX Compatibility Interrupt Flag.  This flag is intended to be utilized
	 * for VAX Compatibility.  It is intended to be utilized to determine if a
	 * sequence of Alpha instructions between RS and RC, corresponding to a
	 * single VAX instruction, were executed without interruption or exception.
	 */
	bool				VAXintrFlag;

	/*
	 * Load Lock/Store Conditional.
	 *
	 * The following fields are used for handling the LDx_L/STx_C instructions.
	 * The first field is a true/false indicator (locked/not locked,
	 * respectively).  The second field is the Locked Physical Address location
	 * to receive the address locked by the LDx_L instruction.  The STx_C
	 * instruction must reference the same 16-byte naturally aligned block as
	 * the LDx_L instruction.
	 */
	bool				lockFlag;
	u64					lockedPhysicalAddress;
	u64					lockedVirtualAddress;

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
	 *
	 * NOTE: There are no architectural registers (ARs).  These registers are
	 * virtual.  The virtual ARs are assigned at the time an instruction is
	 * decoded.  The register mapping is used too determine which physical
	 * register (PR) is defined to which AR.
	 */
	u64					pr[AXP_INT_PHYS_REG];
	u32					prFreeList[AXP_I_FREELIST_SIZE];
	u32					prFlStart;
	u32					prFlEnd;
	AXP_21264_REG_MAP	prMap[AXP_MAX_REGISTERS];
	AXP_21264_REG_STATE	prState[AXP_INT_PHYS_REG];

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
	u64					pf[AXP_FP_PHYS_REG];
	u32					pfFreeList[AXP_F_FREELIST_SIZE];
	u32					pfFlStart;
	u32					pfFlEnd;
	AXP_21264_REG_MAP	pfMap[AXP_MAX_REGISTERS];
	AXP_21264_REG_STATE	pfState[AXP_FP_PHYS_REG];

	/*
	 * Fbox IPRs
	 */
	AXP_FBOX_FPCR		fpcr;

	/**************************************************************************
	 *	Mbox Definitions													  *
	 *																		  *
	 *	The Mbox is responsible for providing data to the Ebox and Fbox.  The *
	 *	Mbox maintains a Load and Store Queue, as well as a Miss Address	  *
	 *	File.  																  *
	 *																		  *
	 *	The Mbox interfaces with with the Cbox, Ebox, and Fbox (see above for *
	 *	more information on the last 2).  The Cbox provides data when a		  *
	 *	Dcache miss occurs.  The Mbox provides data to the Cbox to store in	  *
	 *	memory when a store operation occurs.								  *
	 **************************************************************************/

	/*
	 * This is the Data Cache.  It is 64K bytes in size (just counting the
	 * actual data in the cache block and not the bits needed to support it.
	 * Each cache block contains 64 bytes of data.  There are 2 sets of cache,
	 * which leaves us with 64KB / 64B / 2 sets = 512 rows for each set.
	 */
	AXP_DCACHE_BLK			dCache[AXP_CACHE_ENTRIES][AXP_2_WAY_CACHE];

	/*
	 * The next 2 "queues" are actually going to be handled as a FIFO stack.
	 * These queues handle the outstanding load and store operations into the
	 * Dcache (and ultimately memory), and need to be completed in order, to
	 * ensure correct Alpha memory reference behavior.
	 */
	AXP_MBOX_QUEUE			lq[AXP_MBOX_QUEUE_LEN];
	u32						lqNext;
	AXP_MBOX_QUEUE			sq[AXP_MBOX_QUEUE_LEN];
	u32						sqNext;
	u8						maf;
	AXP_21264_TLB			dtb[AXP_TB_LEN];
	u32						nextDTB;

	/*
	 * The following is used to detect when we have a TB Miss while processing
	 * a previous TB Miss (a Double TB Miss).
	 */
	bool					tbMissOutstanding;

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
	 *
	 * NOTE:	Some or all of these may be deleted, as they are emulated in
	 *			IPRs specific to the 21264 Alpha AXP CPU.
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

	/*
	 * Miscellaneous stuff the should probably go someplace else.
	 * TODO: Find the real home for these values.
	 */
	AXP_PROC_MAJ_TYPE	majorType;	/* Processor Major Type					*/
	u32					minorType;	/* Processor Minor Type					*/
	AXP_BASE_AMASK		amask;		/* Architectural Extension Support Mask	*/
	u64					implVer;	/* Implementation Version				*/
} AXP_21264_CPU;

#endif /* _AXP_21264_CPU_DEFS_ */
