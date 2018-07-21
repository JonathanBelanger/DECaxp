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
 * Revision History:
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
 *
 *	V01.007		23-Oct-2017	Jonathan D. Belanger
 *	Redefined some Cbox definitions, added a CTAG definition, and added a
 *	definition to be able to determine if a physical address is for I/O
 *	Address Space or Memory Address Space.
 *
 *	V01.008		19-Nov-2017	Jonathan D. Belanger
 *	Started adding pthread definitions.  Also moved some definitions from this
 *	header file to be Base CPU header file.
 *
 *	V01.009		13-Jan-2018	Jonathan D. Belanger
 *	The ReOrder Buffer is accessed by the Ibox (when decoding and queuing the
 *	instruction to the appropriate integer/floating-point queue, and by the
 *	Ebox and Fbox when an instruction has been set to retire.  In order to be
 *	able to prevent a conflict, we need a mutex to protect the ROB.
 *
 *	V01.010		24-Feb-2018	Jonathan D. Belanger
 *	Added some comments about what variables do what for the architectural to
 *	physical register mapping.  This is not overly clear within the code.
 *
 *	V01.011		04-Mar-2018	Jonathan D. Belanger
 *	Register renaming needs to account for the condition where an architectural
 *	register can be mapped to both a source and destination register in the
 *	same instruction.  Therefore, the mapping registers cannot be a simple one,
 *	but needs to account for this condition.
 *
 *	V01.012		01-Apr-2018	Jonathan D. Belanger
 *	Moved the function prototype for the call to allocate a CPU structure to
 *	the system/AXP_21274_21264_Common.h file.  When the System structure is
 *	allocated, it will call the CPU allocation function for each of the CPUs
 *	configured on the system.
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
#include "AXP_21264_CboxDefs.h"
#include "AXP_21264_MboxDefs.h"
#include "AXP_21264_RegisterRenaming.h"
#include "AXP_Exceptions.h"
#include "AXP_21264_21274_Common.h"

#define AXP_RESULTS_REG		41
#define AXP_NUM_FETCH_INS	4
#define AXP_IQ_LEN		20
#define AXP_FQ_LEN		15
#define AXP_SHADOW_REG		8
#define AXP_R04_SHADOW		(AXP_MAX_REGISTERS + 0)		/* R32 */
#define AXP_R05_SHADOW		(AXP_MAX_REGISTERS + 1)		/* R33 */
#define AXP_R06_SHADOW		(AXP_MAX_REGISTERS + 2)		/* R34 */
#define AXP_R07_SHADOW		(AXP_MAX_REGISTERS + 3)		/* R35 */
#define AXP_R20_SHADOW		(AXP_MAX_REGISTERS + 4)		/* R36 */
#define AXP_R21_SHADOW		(AXP_MAX_REGISTERS + 5)		/* R37 */
#define AXP_R22_SHADOW		(AXP_MAX_REGISTERS + 6)		/* R38 */
#define AXP_R23_SHADOW		(AXP_MAX_REGISTERS + 7)		/* R39 */
#define AXP_REG(regNum, palMode)					\
		((palMode) ?						\
		 (((cpu->iCtl.sde & AXP_I_CTL_SDE_ENABLE) != 0) ?	\
		  ((((regNum) >= 4) && ((regNum) <= 7)) ?		\
		   ((regNum) + 28) : ((((regNum) >= 20) && ((regNum) <= 23)) ? \
			((regNum) + 16) : (regNum))) : (regNum)) : (regNum))
#define AXP_TB_LEN		128
#define AXP_ICB_INS_CNT		16
#define AXP_21264_PAGE_SIZE	8192	/* 8KB page size */
#define AXP_21264_MEM_BITS	44
#define AXP_MAX_INT_REGISTERS	(AXP_MAX_REGISTERS + AXP_SHADOW_REG)
#define AXP_MAX_FP_REGISTERS	AXP_MAX_REGISTERS
#define AXP_INT_PHYS_REG	80
#define AXP_FP_PHYS_REG		72
#define AXP_UNMAPPED_REG	31	/* R31 and F31 are never renamed/mapped */
#define AXP_I_FREELIST_SIZE	(AXP_INT_PHYS_REG - AXP_MAX_INT_REGISTERS + 1)
#define AXP_F_FREELIST_SIZE	(AXP_FP_PHYS_REG - AXP_MAX_FP_REGISTERS + 1)
#define AXP_INFLIGHT_FETCHES	20
#define AXP_INFLIGHT_MAX	80
#define AXP_MBOX_QUEUE_LEN	32
#define AXP_CACHE_ENTRIES	512
#define AXP_2_WAY_CACHE		2
#define AXP_21264_IOWB_LEN	4
#define AXP_21264_VDB_LEN	8
#define AXP_21264_MAF_LEN	8
#define AXP_21264_EBOX_L0	0
#define AXP_21264_EBOX_L1	1
#define AXP_21264_EBOX_U0	2
#define AXP_21264_EBOX_U1	3
#define AXP_21264_EBOX_CLUSTERS	4
#define AXP_21264_FBOX_MULTIPLY	0
#define AXP_21264_FBOX_OTHER	1
#define AXP_21264_FBOX_CLUSTERS	2

/*
 * Prediction stack macros.
 */
#define AXP_PUSH(pc)							\
	if (cpu->predStackIdx > 0)					\
		cpu->predictionStack[--cpu->predStackIdx] = (pc)
#define AXP_POP(pc)							\
	if (cpu->predStackIdx < AXP_INFLIGHT_MAX)			\
		pc = cpu->predictionStack[cpu->predStackIdx++]
#define AXP_SWAP(pc)							\
	if (cpu->predStackIdx < AXP_INFLIGHT_MAX)			\
	{								\
		AXP_PC tmpPC;						\
		tmpPC = cpu->predictionStack[cpu->predStackIdx];	\
		cpu->predictionStack[cpu->predStackIdx] = (pc);		\
		pc = tmpPC;						\
	}

/*
 * PALcode Exception Entry Points
 *  When hardware encounters an exception, Ibox execution jumps to a PALcode
 *  entry point at a PC determined by the type of exception.  The return PC of
 *  the instruction the triggered the exception is placed in the EXC_ADDR
 *  register and onto the return prediction stack.  The entry points are listed
 *  in decreasing order of priority.
 *
 *	-----------------------------------------------------------------------
 *	Entry Name	Type		Offset	Description
 *	-----------------------------------------------------------------------
 *	DTBM_DOUBLE_3	Fault		0x100	Dstream TB miss on virtual page
 *						table entry fetch.  Use
 *						three-level flow.
 *	DTBM_DOUBLE_4	Fault		0x180	Dstream TB miss on virtual page
 *						table entry fetch.  Use
 *						four-level flow.
 *	FEN		Fault		0x200	Floating point disabled.
 *	UNALIGNED	Fault		0x280	Unaligned Dstream reference.
 *	DTBM_SINGLE	Fault		0x300	Dstream TB miss.
 *	DFAULT		Fault		0x380	Dstream fault or virtual
 *						address sign check error.
 *	OPCDEC		Fault		0x400	Illegal opcode or function
 *						field:
 *						- Opcode 1, 2, 3, 4, 5, 6 or 7
 *						- Opcode 0x19, 0x1b, 0x1d, 0x1e
 *						  or 0x1f not PALmode or not
 *						  I_CTL[HWE]
 *						- Extended precision IEEE
 *						  format
 *						- Unimplemented function field
 *						  of opcodes 0x14 or 0x1c.
 *	IACV		Fault		0x480	Istream access violation or
 *						virtual address sign check
 *						error.
 *	MCHK		Interrupt	0x500	Machine check.
 *	ITB_MISS	Fault		0x580	Istream TB miss.
 *	ARITH		Synch. Trap	0x600	Arithmetic exception or update
 *						to FPCR.
 *	INTERRUPT	Interrupt	0x680	Interrupt: hardware, software,
 *						and AST.
 *	MT_FPCR		Synch. Trap	0x700	Invoked when an MT_FPCR
 *						instruction is issued.
 *	RESET/WAKEUP	Interrupt	0x780	Chip reset or wake-up from
 *						sleep mode.
 *	-----------------------------------------------------------------------
 * 	NOTE: These are CPU generation specific.  The ones below are for the
 * 	      21264 generation of the Digital Alpha AXP CPU.
 */
#define AXP_NO_FAULTS		0x0000
#define AXP_DTBM_DOUBLE_3	0x0100
#define AXP_DTBM_DOUBLE_4	0x0180
#define AXP_FEN			0x0200
#define AXP_UNALIGNED		0x0280
#define AXP_DTBM_SINGLE		0x0300
#define AXP_DFAULT		0x0380
#define AXP_OPCDEC		0x0400
#define AXP_IACV		0x0480
#define AXP_MCHK		0x0500
#define AXP_ITB_MISS		0x0580
#define AXP_ARITH		0x0600
#define AXP_INTERRUPT		0x0680
#define AXP_MT_FPCR_TRAP	0x0700
#define AXP_RESET_WAKEUP	0x0780

/*
 * This structure is a buffer to contain the next set of instructions to get
 * queued up for execution.
 */
typedef struct
{
    bool branch2bTaken;
    u32 linePrediction;
    u32 setPrediction;
    u64 retPredStack;
    AXP_INS_FMT instructions[AXP_NUM_FETCH_INS];
    AXP_INS_TYPE instrType[AXP_NUM_FETCH_INS];
    AXP_PC instrPC[AXP_NUM_FETCH_INS];
} AXP_INS_LINE;

typedef struct
{
    AXP_CQUE_ENTRY header;
    AXP_INSTRUCTION *ins;
    AXP_PIPELINE pipeline;
    u32 index;
    bool processing;
} AXP_QUEUE_ENTRY;

/*
 * The following states are used during CPU execution.  The state transitions
 * are as follows:
 *
 *	-----------------------------------------------------------------------
 * 	Current State	Event				Next State
 *	-----------------------------------------------------------------------
 * 	Cold 		Complete CPU initialization	WaitBiST
 * 	WaitBiST 	Complete BiST			Run
 * 	Run 		Fault Reset_L 			FaultReset
 * 	Run 		SleepMode			Sleep
 * 	Run 		Shutdown 			ShuttingDown
 * 	FaultReset 	Complete CPU Reset		Run
 * 	Sleep 		Wake 				WaitBiSI
 * 	WaitBiSI 	Complete self initialization 	Run
 * 	ShuttingDown	shutdown complete		EXIT(1)
 *	-----------------------------------------------------------------------
 * 	(1) EXIT = Delete CPU main thread (shutdown already deleted the others)
 */
typedef enum
{
    Cold,
    WaitBiST,
    WaitBiSI,
    Run,
    FaultReset,
    Sleep,
    ShuttingDown
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
} AXP_21264_EVENTS;
;

typedef enum
{
    SystemReset,
    BiSTRunning,
    BiSTFailed,
    BiSTSucceeded
} AXP_21264_BIST_STATES;

#define AXP_SCBD_BIT_0		0x01
#define AXP_SCBD_BIT_1		0x02
#define AXP_SCBD_BIT_2		0x04
#define AXP_SCBD_BIT_3		0x08
#define AXP_SCBD_BITS_0_3	0x0f
#define AXP_SCBD_BIT_4		0x10
#define AXP_SCBD_BIT_5		0x20
#define AXP_SCBD_BIT_6		0x40
#define AXP_SCBD_BIT_7		0x80
#define AXP_SCBD_BITS_4_7	0xf0

/*
 * Structure to hold information about the System we are connected to so that
 * we can send and receive data between us.
 */
typedef struct
{
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    AXP_QUEUE_HDR *rq;
} AXP_21264_SYSTEM;

/*
 * This structure contains all the fields required to emulate an Alpha AXP
 * 21264 CPU.
 */
typedef struct
{

    /*
     * CPU state.
     */
    pthread_mutex_t cpuMutex;
    pthread_cond_t cpuCond;
    AXP_21264_STATES cpuState;
    AXP_21264_BIST_STATES BiSTState;

    /**************************************************************************
     *	Ibox Definitions						      *
     *									      *
     *	The Ibox is responsible for instruction processing.  It maintains the *
     *	VPC Queue, ITB, Branch Prediction, Instruction Predecode, Instruction *
     *	decode and register renaming, Instruction Cache, Instruction	      *
     *	Retirement, and the Integer and Floating-Point Instruction Queues.    *
     *									      *
     *	The Ibox interfaces with with the Cbox, Ebox, and Fbox.  The Cbox     *
     *	provides the next set of instructions when an Icache miss occurs.     *
     *	The set of instructions are provided to the Ibox for predecoding and  *
     *	entry into the Icache.  The Ebox reads instructions of the Integer    *
     *	Issue Queue (IQ) into upto 4 integer processors.  The Fbox reads      *
     *	instructions from the FP Issue Queue (FQ) into upto 2 FP processors.  *
     **************************************************************************/

    /*
     * The following definitions are used to support utilizing pthreads for the
     * Ibox.
     */
    pthread_t iBoxThreadID;
    pthread_mutex_t iBoxMutex;
    pthread_cond_t iBoxCondition;
    bool excPend;
    AXP_PC excPC;

    /*
     * The following definitions are used by the branch prediction code.
     */
    LHT localHistoryTable;
    LPT localPredictor;
    GPT globalPredictor;
    CPT choicePredictor;
    u16 globalPathHistory;
    u8 instrCounter; /* Unique ID for each instruction */
    AXP_PC predictionStack[AXP_INFLIGHT_MAX];
    u8 predStackIdx;

    /*
     * This is equivalent to the VPC
     */
    AXP_PC vpc[AXP_INFLIGHT_MAX];
    u32 vpcStart;
    u32 vpcEnd;

    /*
     * Reorder Buffer
     */
    pthread_mutex_t robMutex;
    AXP_INSTRUCTION rob[AXP_INFLIGHT_MAX];
    u32 robStart;
    u32 robEnd;

    /*
     * Instruction Queues (Integer and Floating-Point), as well as the IQ
     * scoreboard bits.
     */
    u8 scoreboard;
    AXP_COUNTED_QUEUE iq;
    AXP_COUNTED_QUEUE fq;

    /*
     * Instruction Queue Pre-allocated Cache.
     */
    AXP_QUEUE_ENTRY iqEntries[AXP_IQ_LEN];
    u32 iqEFreelist[AXP_IQ_LEN];
    u32 iqEFlStart;
    u32 iqEFlEnd;
    AXP_QUEUE_ENTRY fqEntries[AXP_FQ_LEN];
    u32 fqEFreelist[AXP_IQ_LEN];
    u32 fqEFlStart;
    u32 fqEFlEnd;

    /*
     * Ibox Internal Processor Registers (IPRs)
     */
    pthread_mutex_t iBoxIPRMutex;
    AXP_IBOX_ITB_TAG itbTag;	/* ITB tag array write			*/
    AXP_IBOX_ITB_PTE itbPte;	/* ITB Page Table Entry			*/
    AXP_IBOX_ITB_IS itbIs;	/* ITB invalidate single		*/
    AXP_IBOX_EXC_ADDR excAddr;	/* Exception address			*/
    AXP_IBOX_IVA_FORM ivaForm;	/* Instruction VA format		*/
    AXP_IBOX_IER_CM ierCm;	/* Interrupt enable and current mode	*/
    AXP_IBOX_SIRR sirr;		/* Software interrupt request		*/
    AXP_IBOX_ISUM iSum;		/* Interrupt summary			*/
    AXP_IBOX_HW_INT_CLR hwIntClr; /* Hardware interrupt clear		*/
    AXP_IBOX_EXC_SUM excSum;	/* Exception summary			*/
    AXP_IBOX_PAL_BASE palBase;	/* PAL base address			*/
    AXP_IBOX_I_CTL iCtl;	/* Ibox control				*/
    AXP_IBOX_I_STAT iStat;	/* Ibox status				*/
    AXP_IBOX_PCTX pCtx;		/* Process context register		*/
    AXP_IBOX_PCTR_CTL pCtrCtl;	/* Performance counter control		*/

    /*
     * This is the Instruction Cache.  It is 64K bytes in size (just counting
     * the actual instructions (16 of them) in the cache block and not the bits
     * needed to support it.  Each cache block contains 16 instructions, each 4
     * bytes long, for a total of 64 bytes.  There are 2 sets of cache, which
     * leaves us with 64KB / (16 * 4B) / 2 sets = 512 rows for each set.
     */
    pthread_mutex_t iCacheMutex;
    AXP_ICACHE_BLK iCache[AXP_CACHE_ENTRIES][AXP_2_WAY_CACHE];
    bool iCacheFlushPending;
    bool stallWaitingRetirement;

    /*
     * This is the Instruction Address Translation (Look-aside) Table (ITB).
     * It is 128 entries in size, and is allocated in a round-robin scheme.
     * We, therefore, maintain a start and end index (both start at 0).
     */
    pthread_mutex_t itbMutex;
    AXP_21264_TLB itb[AXP_TB_LEN];
    u32 nextITB;

    /**************************************************************************
     *	Ebox Definitions						      *
     *									      *
     *	The Ebox is responsible for processing instructions from the IQ.  It  *
     *	maintains 2 sets of Physical Integer Registers, which are copies of   *
     *	one another.  It can handle up to 4 simultaneous instructions.	      *
     *									      *
     *	The Ebox interfaces with the Ibox (see above for more information),   *
     *	the Fbox and the Mbox.  The Fbox and Ebox are allowed to move values  *
     *	from a register in one to the other.  This is does for Integer/FP to  *
     *	FP/Integer conversion and FP branch operations.  The Mbox provides    *
     *	data to the Ebox from memory, via that data cache (Dcache).	      *
     **************************************************************************/

    /*
     * The following definitions are used to support utilizing pthreads for the
     * Ebox.
     */
    pthread_t eBoxU0ThreadID;
    pthread_t eBoxU1ThreadID;
    pthread_t eBoxL0ThreadID;
    pthread_t eBoxL1ThreadID;
    pthread_mutex_t eBoxMutex;
    pthread_cond_t eBoxCondition;

    /*
     * An optimization for the Ebox is a counter that indicates the number of
     * instructions in the IQ that could be executed by a particular cluster.
     * If the counter is zero, there is nothing for that cluster.  Otherwise,
     * there is at least one instruction that could be executed by the cluster.
     * These counters are incremented in the Ibox and decremented in the Ebox.
     */
    u16 eBoxClusterCounter[AXP_21264_EBOX_CLUSTERS];

    /*
     * When the Mbox completes an instruction, it sets this flag and then
     * signal the Ebox conditional variable.
     */
    bool eBoxWaitingRetirement;

    /*
     * VAX Compatibility Interrupt Flag.  This flag is intended to be utilized
     * for VAX Compatibility.  It is intended to be utilized to determine if a
     * sequence of Alpha instructions between RS and RC, corresponding to a
     * single VAX instruction, were executed without interruption or exception.
     */
    bool VAXintrFlag;

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
     *
     * Design Information:
     *	pr: 		These are the actual physical registers.  For the integer
     *			pipeline, there are a total of 80 physical registers.
     *	prFreeList:	his is where physical registers that are available for use
     *			as a destination register are maintained.  When renaming
     *			the architectural to physical for a destination register,
     *			a free register is removed from this list and indicated as
     *			Pending-Update (when the instruction is retired and the
     *			value of the instruction is actually stored in the physical
     *			register.  Physical registers are put back on this list
     *			when they are no longer being mapped.  This list is
     *			maintained in a round-robin fashion.  Entries are removed
     *			from the prFlStart entry and returned at the prFlEnd entry.
     *			There are a total of 80 (physical) - 32 (architectural) -
     *			8 (PALshadow) = 40 register entries on the free-list.
     *	prFlStart:	This is a value from 0 to 39, representing the location
     *			within the prFreeList where the first available physical
     *			register number is stored.  When this variable has a value
     *			of 39 and 1 is added, it returns back to a value of 0.
     *	prFlEnd:	This is a value from 0 to 39, representing the location
     *			within the prFreeList where the last+1 available physical
     *			register number is returned.  When this variable has a
     *			value of 39 and 1 is added, it returns back to a value of
     *			0.  When the free-list is completely full or empty, the
     *			prFlStart and prFlEnd are equal.
     *	prMap:		These are the mappings for each of the architectural
     *			registers to physical registers currently being mapped.
     *			This array is indexed by the architectural register number.
     *			R31 is always mapped to entry 31.  Each entry has the
     *			current and previous mapping to a physical register.  The
     *			previous mapping is used when aborting a set of
     *			instructions.  There are 32 (architectural) + 8 (PALshadow)
     *			= 40 register entries.
     *	prState:	These are the states for each of the physical registers.
     *			The states for each physical register are:
     *				- Free:			The register is on the free list
     *							and available for mapping.
     *				- Valid:		The register is currently mapped
     *							and its value is ready to be used
     *							to execute an instruction.
     *				- Pending Update:	The register is currently mapped
     *							but its value is not ready to be
     *							used.  It is mapped as a
     *							destination register for an
     *							instruction that has not been
     *							retired.  One the instruction is
     *							retired, the state will be changed
     *							to Valid.
     *			There are a total of 80 physical register states (one for
     *			each physical register) entries.  This array is index by
     *			the physical register number.
     */
    AXP_REGISTERS pr[AXP_INT_PHYS_REG];
    u16 prMap[AXP_MAX_INT_REGISTERS];
    u16 prFreeList[AXP_I_FREELIST_SIZE];
    u16 prFlStart;
    u16 prFlEnd;

    /*
     * Ebox IPRs
     */
    pthread_mutex_t eBoxIPRMutex;
    AXP_EBOX_CC cc; /* Cycle counter					*/
    AXP_EBOX_CC_CTL ccCtl; /* Cycle counter control			*/
    AXP_EBOX_VA va; /* Virtual address					*/
    AXP_EBOX_VA_CTL vaCtl; /* Virtual address control			*/
    AXP_EBOX_VA_FORM vaForm; /* Virtual address format			*/

    /**************************************************************************
     *	Fbox Definitions						      *
     *									      *
     *	The Fbox is responsible for processing instructions from the FQ.  It  *
     *	maintains a sets of Physical Integer Registers.  It can handle upto 2 *
     *	simultaneous instructions.					      *
     *									      *
     *	The Fbox interfaces with the Ibox (see above for more information),   *
     *	the Ebox (See above for more information) and the Mbox.  The Mbox     *
     *	provides data to the Fbox from memory, via that data cache (Dcache).  *
     **************************************************************************/

    /*
     * The following definitions are used to support utilizing pthreads for the
     * Fbox.
     */
    pthread_t fBoxMulThreadID;
    pthread_t fBoxOthThreadID;
    pthread_mutex_t fBoxMutex;
    pthread_cond_t fBoxCondition;

    /*
     * An optimization for the Fbox is a counter that indicates the number of
     * instructions in the FQ that could be executed by a particular cluster.
     * If the counter is zero, there is nothing for that cluster.  Otherwise,
     * there is at least one instruction that could be executed by the cluster.
     * These counters are incremented in the Ibox and decremented in the Fbox.
     */
    u16 fBoxClusterCounter[AXP_21264_FBOX_CLUSTERS];

    /*
     * When the Mbox completes an instruction, it sets this flag and then
     * signal the Ebox conditional variable.
     */
    bool fBoxWaitingRetirement;

    /*
     * Physical registers.
     *
     * There are 72 register file entries for the floating-point registers.
     * This is the 31 Floating-point registers (F31 is not stored), plus the 41
     * results for instructions that can potentially have not been retired.
     *
     * Since the floating-point execution unit only has 1 cluster, there is
     * just 1 set of 72 registers.
     *
     * Design Information:
     *	pf: 		These are the actual physical registers.  For the floating
     *			point pipeline, there are a total of 72 physical registers.
     *	pfFreeList:	This is where physical registers that are available for use
     *			s a destination register are maintained.  When renaming
     *			the architectural to physical for a destination register,
     *			a free register is removed from this list and indicated as
     *			Pending-Update (when the instruction is retired and the
     *			value of the instruction is actually stored in the physical
     *			register.  Physical registers are put back on this list
     *			when they are no longer being mapped.  This list is
     *			maintained in a round-robin fashion.  Entries are removed
     *			from the pfFlStart entry and returned at the pfFlEnd entry.
     *			There are a total of 72 (physical) - 32 (architectural) =
     *			40 register entries on the free-list.
     *	pfFlStart:	This is a value from 0 to 39, representing the location
     *			within the pfFreeList where the first available physical
     *			register number is stored.  When this variable has a value
     *			of 39 and 1 is added, it returns back to a value of 0.
     *	pfFlEnd:	This is a value from 0 to 39, representing the location
     *			within the pfFreeList where the last+1 available physical
     *			register number is returned.  When this variable has a
     *			value of 39 and 1 is added, it returns back to a value of
     *			0.  When the free-list is completely full or empty, the
     *			pfFlStart and pfFlEnd are equal.
     *	pfMap:		These are the mappings for each of the architectural
     *			registers to physical registers currently being mapped.
     *			This array is indexed by the architectural register number.
     *			R31 is always mapped to entry 31.  Each entry has the
     *			current and previous mapping to a physical register.  The
     *			previous mapping is used when aborting a set of
     *			instructions.  There are 32 (architectural) register
     *			entries.
     *	pfState:	These are the states for each of the physical registers.
     *			The states for each physical register are:
     *				- Free:			The register is on the free list
     *							and available for mapping.
     *				- Valid:		The register is currently mapped
     *							and its value is ready to be used
     *							to execute an instruction.
     *				- Pending Update:	The register is currently mapped
     *							but its value is not ready to be
     *							used.  It is mapped as a
     *							destination register for an
     *							instruction that has not been
     *							retired.  One the instruction is
     *							retired, the state will be changed
     *							to Valid.
     *			There are a total of 72 physical register states (one for
     *			each physical register) entries.  This array is index by
     *			the physical register number.
     */
    AXP_REGISTERS pf[AXP_FP_PHYS_REG];
    u16 pfMap[AXP_MAX_FP_REGISTERS];
    u16 pfFreeList[AXP_F_FREELIST_SIZE];
    u16 pfFlStart;
    u16 pfFlEnd;

    /*
     * Fbox IPRs
     */
    AXP_FBOX_FPCR fpcr;

    /**************************************************************************
     *	Mbox Definitions						      *
     *									      *
     *	The Mbox is responsible for providing data to the Ebox and Fbox.  The *
     *	Mbox maintains a Load and Store Queue, as well as a Miss Address      *
     *	File.  								      *
     *									      *
     *	The Mbox interfaces with with the Cbox, Ebox, and Fbox (see above for *
     *	more information on the last 2).  The Cbox provides data when a	      *
     *	Dcache miss occurs.  The Mbox provides data to the Cbox to store in   *
     *	memory when a store operation occurs.				      *
     **************************************************************************/

    /*
     * The following definitions are used to support utilizing pthreads for the
     * Mbox.
     */
    pthread_t mBoxThreadID;
    pthread_mutex_t mBoxMutex;
    pthread_cond_t mBoxCondition;

    /*
     * This is the Data Cache.  It is 64K bytes in size (just counting the
     * actual data in the cache block and not the bits needed to support it.
     * Each cache block contains 64 bytes of data.  There are 2 sets of cache,
     * which leaves us with 64KB / 64B / 2 sets = 512 rows for each set.
     */
    pthread_mutex_t dCacheMutex;
    AXP_DCACHE_BLK dCache[AXP_CACHE_ENTRIES][AXP_2_WAY_CACHE];
    pthread_mutex_t dtagMutex;
    AXP_DTAG_BLK dtag[AXP_CACHE_ENTRIES][AXP_2_WAY_CACHE];

    /*
     * The next 2 "queues" are actually going to be handled as a FIFO stack.
     * These queues handle the outstanding load and store operations into the
     * Dcache (and ultimately memory), and need to be completed in order, to
     * ensure correct Alpha memory reference behavior.
     *
     * NOTE:	These queue are not mutex or conditional queues.  We do this
     *		because the index into the queue is reserved in the iBox during
     *		instruction decoding.
     */
    pthread_mutex_t lqMutex;
    AXP_MBOX_QUEUE lq[AXP_MBOX_QUEUE_LEN];
    u32 lqNext;
    pthread_mutex_t sqMutex;
    AXP_MBOX_QUEUE sq[AXP_MBOX_QUEUE_LEN];
    u32 sqNext;
    pthread_mutex_t dtbMutex;
    AXP_21264_TLB dtb[AXP_TB_LEN];
    u32 nextDTB;

    /*
     * The following is used to detect when we have a TB Miss while processing
     * a previous TB Miss (a Double TB Miss).
     */
    bool tbMissOutstanding;

    /*
     * Mbox IPRs
     */
    pthread_mutex_t mBoxIPRMutex;
    AXP_MBOX_DTB_TAG dtbTag0;	/* DTB tag array write 0		*/
    AXP_MBOX_DTB_TAG dtbTag1;	/* DTB tag array write 1		*/
    AXP_MBOX_DTB_PTE dtbPte0;	/* DTB PTE array write 0		*/
    AXP_MBOX_DTB_PTE dtbPte1;	/* DTB PTE array write 1		*/
    AXP_MBOX_DTB_ALTMODE dtbAltMode; /* DTB alternate processor mode	*/
    AXP_MBOX_DTB_IS dtbIs0;	/* DTB invalidate single 0		*/
    AXP_MBOX_DTB_IS dtbIs1;	/* DTB invalidate single 1		*/
    AXP_MBOX_DTB_ASN dtbAsn0;	/* DTB address space number 0		*/
    AXP_MBOX_DTB_ASN dtbAsn1;	/* DTN address space number 1		*/
    AXP_MBOX_MM_STAT mmStat;	/* Memory management status		*/
    AXP_MBOX_M_CTL mCtl;	/* Mbox control				*/
    AXP_MBOX_DC_CTL dcCtl;	/* Dcache control			*/
    AXP_MBOX_DC_STAT dcStat;	/* Dcache status			*/

    /**************************************************************************
     *	Cbox Definitions						      *
     *									      *
     *	The Cbox is responsible for interfacing with the system.  It	      *
     *	maintains a Probe Queue, Duplicate Tag Store, I/O Write Buffer	      *
     *	(IOWB), Victim Buffer, and Arbiter.  It interfaces with the System    *
     *	(memory, disk drives, I/O devices, etc.), Ibox and Mbox (see above    *
     *	for more information about the last 2 items).			      *
     *									      *
     *	The Cbox is responsible for the interfaces between the system and the *
     *	CPU.								      *
     **************************************************************************/

    /*
     * The following definitions are used to support utilizing pthreads for the
     * Cbox.
     */
    pthread_t cBoxThreadID;

    /*
     * The following structures are used to control the Cbox and its queues.
     */
    pthread_mutex_t cBoxInterfaceMutex;
    pthread_cond_t cBoxInterfaceCond;
    AXP_21264_CBOX_VIC_BUF vdb[AXP_21264_VDB_LEN];
    AXP_21264_CBOX_IOWB iowb[AXP_21264_IOWB_LEN];
    AXP_21264_CBOX_PQ pq[AXP_21264_PQ_LEN];
    bool noProbeResponses;
    AXP_21264_CBOX_MAF maf[AXP_21264_MAF_LEN];
    u8 irqH;			/* Interrupt bits (IRQH[0:5] set by system */
    u8 vdbTop, vdbBottom;
    u8 iowbTop, iowbBottom;
    u8 pqTop, pqBottom;
    u8 mafTop, mafBottom;
    u8 cmdAck;

    /*
     * Cbox IPRs
     */
    pthread_mutex_t cBoxIPRMutex;
    AXP_21264_CBOX_CTAG ctag[AXP_CACHE_ENTRIES][AXP_2_WAY_CACHE];
    AXP_CBOX_C_DATA cData;	/* Cbox data				*/
    AXP_CBOX_C_SHFT cShft;	/* Cbox shift control			*/
    AXP_21264_CBOX_CSRS csr;	/* Control and Status Registers (CSR)	*/

    /*
     * Bcache data structures.
     */
    pthread_mutex_t bCacheMutex;
    AXP_21264_BCACHE_BLK *bCache;/* An array of 64-byte blocks		*/
    AXP_21264_BCACHE_TAG *bTag;	/* An array of tags for the Bcache	*/

    /*
     * Alpha AXP Architectural IPRs
     *
     * NOTE:	Some or all of these may be deleted, as they are emulated in
     *		IPRs specific to the 21264 Alpha AXP CPU.
     */
    AXP_BASE_ASN asn;		/* Address Space Number			*/
    AXP_BASE_ASTEN astEn;	/* AST Enable				*/
    AXP_BASE_ASTSR astSr;	/* AST Summary Register			*/
    AXP_BASE_DATFX datFx;	/* Data Alignment Trap Fix-up		*/
    AXP_BASE_ESP esp;		/* Executive Stack Pointer		*/
    AXP_BASE_FEN fen;		/* Floating-point Enable		*/
    AXP_BASE_IPIR ipIr;		/* Interprocessor Interrupt Request	*/
    AXP_BASE_IPL ipl;		/* Interrupt Priority Level		*/
    AXP_BASE_KSP ksp;		/* Kernel Stack Pointer			*/
    AXP_BASE_MCES mces;		/* Machine Check Error Summary		*/
    AXP_BASE_PCBB pcbb;		/* Privileged Context Block Base	*/
    AXP_BASE_PRBR prbr;		/* Processor Base Register		*/
    AXP_BASE_PTBR ptbr;		/* Page Table Base Register		*/
    AXP_BASE_SCBB scbb;		/* System Control Block Base		*/
    AXP_BASE_SISR sisr;		/* Software Interrupt Summary Register	*/
    AXP_BASE_SSP ssp;		/* Supervisor Stack Pointer		*/
    AXP_BASE_SYSPTBR sysPtbr;	/* System Page Table Base		*/
    AXP_BASE_TBCHK tbChk;	/* TB Check				*/
    AXP_BASE_USP usp;		/* User Stack Pointer			*/
    AXP_BASE_VIRBND virBnd;	/* Virtual Address Boundary		*/
    AXP_BASE_VPTB vptb;		/* Virtual Page Table Base		*/
    AXP_BASE_WHAMI whami;	/* Who-Am-I				*/

    /*
     * Information about the System, to which we send and receive commands and
     * data.
     */
    AXP_21264_SYSTEM system;

    /*
     * Miscellaneous stuff the should probably go someplace else.
     */
    u32 majorType;		/* Processor Major Type			*/
    u32 minorType;		/* Processor Minor Type			*/
    AXP_BASE_AMASK amask;	/* Architectural Extension Support Mask	*/
    u64 implVer;		/* Implementation Version		*/
} AXP_21264_CPU;

/*
 * The following macros will return true if an address is in the I/O Address
 * Space and false if it is in the Memory Address Space.  The first macro
 * masks out everything but the 44th bit, which if set is for I/O Address
 * Space.
 */
#define AXP_21264_IO_ADDR_SPACE		0x0000080000000000ll
#define AXP_21264_IS_IO_ADDR(addr)	(((addr) & AXP_21264_IO_ADDR_SPACE) != 0)

/*
 * Bits associated with a naturally aligned 64-byte block of memory.
 */
#define AXP_21264_ALIGN_MEM_BLK		0xffffffffffffffc0ll

#endif /* _AXP_21264_CPU_DEFS_ */
