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
 *	This header file contains the definitions to implemented the ITB, DTB,
 *	Icache and Dcache.
 *
 * Revision History:
 *
 *	V01.000		29-Jul-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_CACHE_DEFS_
#define _AXP_21264_CACHE_DEFS_

#include "AXP_Utility.h"
#include "AXP_21264_CPU.h"

/*
 * The following definitions utilize the granularity hint information in the
 */
#define GH_KEEP(gh)		(AXP_21264_PAGE_SIZE < (3 * (gh)) - 1)
#define GH_MATCH(gh)	(((1 << (AXP_21264_MEM_BITS - 1)) - 1) & ~(GH_KEEP(gh)))
#define GH_PHYS(gh)		(((1 << AXP_21264_MEM_BITS) - 1) & ~(GH_KEEP(gh)))

typedef struct
{
	u64		virtAddr;
	u64		physAddr;
	u64		matchMask;
	u64		keepMask;
	u32		kre : 1;
	u32		ere : 1;
	u32		sre : 1;
	u32		ure : 1;
	u32		kwe : 1;
	u32		ewe : 1;
	u32		swe : 1;
	u32		uwe : 1;
	u32		faultOnRead : 1;
	u32		faultOnWrite : 1;
	u32		faultOnExecute : 1;
	u32		res_1 : 21;
	u8		asn;
	bool	_asm;
	bool	valid;
} AXP_21264_TLB;

AXP_21264_TLB *AXP_findTLBEntry(AXP_21264_CPU *cpu, u64 virtAddr, bool dtb)
{
	AXP_21264_TLB	*retVal = NULL;
	AXP_21264_TLB	*tlbArray = (dtb ? cpu->dtb : cpu->itb);
	u8				asn = (dtb ? cpu->dtbAsn0 : cpu->pCtx.asn);
	int				start = (dtb ? cpu->dtbStart : cpu->itbStart);
	int				end = (dtb ? cpu->dtbEnd : cpu->itbEnd);
	int				tmp = 0;
	int				ii;
	bool			done = false;

	while ((done == false) && (retVal == NULL))
	{
		if ((start < end) && (tmp == 0))
			done = true;
		else if (tmp == 0)
		{
			tmp = end;
			end = AXP_TB_LEN;
		}
		else
		{
			start = 0;
			end = tmp;
			done = true;
		}
		for (ii = start; ii < end; ii++)
		{
			if ((tlbArray[ii].valid == true) &&
				(tlbArray[ii].virtAddr ==
				 (virtAddr & tlbArray[ii].matchMask)) &&
				(tlbArray[ii].asn == asn) &&
				(tlbArray[ii]._asm == true))
			{
				retVal = &tlbArray[ii];
				continue;
			}
		}
	}
	return(retVal);
}

void AXP_addTLBEntry(AXP_21264_CPU *cpu, u64 virtAddr, u64 physAddr, bool dtb)
{
	AXP_21264_TLB	*tlbEntry;

	tlbEntry = AXP_findTLBEntry(cpu, virtAddr, dtb);
	if (tlbEntry == NULL)
	{
		if (dtb)
		{
			tlbEntry = &cpu->dtb[cpu->dtbEnd++];
			if (cpu->dtbEnd >= AXP_TB_LEN)
				cpu->dtbEnd = 0;
		}
		else
		{
			tlbEntry = &cpu->itb[cpu->itbEnd++];
			if (cpu->itbEnd >= AXP_TB_LEN)
				cpu->itbEnd = 0;
		}
	}
	tlbEntry->matchMask = GH_MATCH(dtb ? cpu->dtbPte0.gh : cpu->itbPte.gh);
	tlbEntry->keepMask = GH_KEEP(dtb ? cpu->dtbPte0.gh : cpu->itbPte.gh);
	tlbEntry->virtAddr = virtAddr & tlbEntry->matchMask;
	tlbEntry->physAddr = physAddr & GH_PHYS(dtb ? cpu->dtbPte0.gh : cpu->itbPte.gh);
	if (dtb)
	{
		tlbEntry->faultOnRead = cpu->dtbPte0._for;
		tlbEntry->faultOnWrite = cpu->dtbPte0.fow;
		tlbEntry->faultOnExecute = 0;
		// TODO faultOnExecute?

		tlbEntry->kre = cpu->dtbPte0.kre;
		tlbEntry->ere = cpu->dtbPte0.ere;
		tlbEntry->sre = cpu->dtbPte0.sre;
		tlbEntry->ure = cpu->dtbPte0.ure;
		tlbEntry->kwe = cpu->dtbPte0.kwe;
		tlbEntry->ewe = cpu->dtbPte0.ewe;
		tlbEntry->swe = cpu->dtbPte0.swe;
		tlbEntry->uwe = cpu->dtbPte0.uwe;
		tlbEntry->asn = cpu->dtbAsn0.asn;
		tlbEntry->_asm = cpu->dtbPte0._asm;
	}
	else
	{
		// TODO faultOnExecute?

		tlbEntry->kre = cpu->itbPte.kre;
		tlbEntry->ere = cpu->itbPte.ere;
		tlbEntry->sre = cpu->itbPte.sre;
		tlbEntry->ure = cpu->itbPte.ure;
		tlbEntry->kwe = 0;
		tlbEntry->ewe = 0;
		tlbEntry->swe = 0;
		tlbEntry->uwe = 0;
		tlbEntry->asn = cpu->pCtx.asn;	// TODO: Is this right?
		tlbEntry->_asm = cpu->itbPte._asm;
	}
	tlbEntry->valid = true;
	return;
}

#define AXP_CM_KERNEL		0
#define AXP_CM_EXEC			1
#define AXP_CM_SUPER		2
#define AXP_CM_USER			3
#define AXP_SPE2_VA_MASK	0x00000fffffffe000ll
#define AXP_SPE2_VA_VAL		0x2
#define AXP_SPE2_BIT		0x4
#define AXP_SPE1_VA_MASK	0x000001ffffffe000ll
#define AXP_SPE1_PA_43_41	0x00000e0000000000ll
#define AXP_SPE1_BIT		0x2
#define AXP_SPE1_VA_40		0x0000010000000000ll
#define AXP_SPE1_VA_VAL		0x7e
#define AXP_SPE0_VA_MASK	0x000000003fffe000ll
#define AXP_SPE0_VA_VAL		0x3fffe
#define AXP_SPE0_BIT		0x1

typedef struct
{
	u64			res_1 : 46;
	u64			spe2 : 2;
	u64			res_2 : 16;
} AXP_VA_SPE2;
typedef struct
{
	u64			res_1 : 41;
	u64			spe2 : 7;
	u64			res_2 : 16;
} AXP_VA_SPE1;
typedef struct
{
	u64			res_1 : 30;
	u64			spe2 : 18;
	u64			res_2 : 16;
} AXP_VA_SPE0;
typedef union
{
	u64			va;
	AXP_VA_SPE2	spe2;
	AXP_VA_SPE1	spe1;
	AXP_VA_SPE0	spe0;
} AXP_VA_SPE;

u64 AXP_va2pa(AXP_21264_CPU *cpu, u64 va, AXP_PC pc, bool dtb, bool *_asm)
{
	AXP_VA_SPE		vaSpe = {.va = va};
	AXP_21264_TLB	*tlb;
	u64				pa = 0x0ll;
	u8				spe = (dtb ? cpu->mCtl.spe : cpu->iCtl.spe);

	if (pc.pal == AXP_PAL_MODE)
		pa = va;
	else if (spe != 0)
	{
		if (cpu->ierCm.cm != AXP_CM_KERNEL)
		{
			/* access violation */
		}
		else if ((spe & AXP_SPE2_BIT) && (vaSpe.spe2 == AXP_SPE2_VA_VAL))
		{
			pa = va & AXP_SPE2_VA_MASK;
			_asm = false;
			return(pa);
		}
		else if ((spe & AXP_SPE1_BIT) && (vaSpe.spe1 == AXP_SPE1_VA_VAL))
		{
			pa = ((va & AXP_SPE1_VA_MASK) |
				  (va & AXP_SPE1_VA_40 ? AXP_SPE1_PA_43_41 : 0));
			_asm = false;
			return(pa);
		}
		else if ((spe & AXP_SPE0_BIT) && (vaSpe.spe0 == AXP_SPE0_VA_VAL))
		{
			pa = va & AXP_SPE0_VA_MASK;
			_asm = false;
			return(pa);
		}
	}
	// TODO: Need to determine if we need to do the following when in PAL mode
	tlb = AXP_findTLBEntry(cpu, va, dtb);
	if (tlb == NULL)
	{
		if (pc.pal != AXP_PAL_MODE)
		{
			cpu->excAddr = pc;
			/* need to figure out how to recognize a double miss */
			if (single fault)
				cpu->mmStat.? = 1;
			cpu->va = va;
			cpu->excSum.? = 1;
			/* need to tell the Ibox to call one of the TB miss PALcode entries */
		}
		else
		{
			if (double miss)
			{
				if (cpu->iCtrl.va_48 == 0)
					/* handle the double miss (DTBM_DOUBLE_3) */
				else
					/* handle the double miss (DTBM_DOUBLE_4) */
			}
			else if (dtb == true)
			{
				/* handle the DTB miss (DTBM_SINGLE) */
			}
			else
			{
				/* handle the ITB miss (ITB_MISS) */
			}
			pa = AXP_va2pa(cpu, va, pc, dtb);
		}
	}
	else
	{
		// TODO: Look at using some other mechanism to determine memory access.
		if (((cpu->ierCm.cm == AXP_CM_KERNEL) && ((tlb->kre != reqAccess) || (tlb->kwe != reaAccess))) ||
			((cpu->ierCm.cm == AXP_CM_EXEC) && ((tlb->ere != reqAccess) || (tlb->ewe != reaAccess))) ||
			((cpu->ierCm.cm == AXP_CM_SUPER) && ((tlb->sre != reqAccess) || (tlb->swe != reaAccess))) ||
			((cpu->ierCm.cm == AXP_CM_USER) && ((tlb->ure != reqAccess) || (tlb->uwe != reaAccess))))
		{
			cpu->excAddr = pc;
			if (dtb == true)
			{
				cpu->excSum.? = 1;
				cpu->mmStat.? = 1;
				cpu->va = va;
				/* need to tell iBox to call the DFAULT PALcode */
			}
			else
			{
				cpu->excSum = 0;
				/* need to tell iBox to call the IACV PALcode */
			}
		}
		else
		{
			pa = tlb->physAddr | (va & tlb->keepMask)
			_asm = tlb->_asm;
		}
	}
	return(pa);
}
#endif /* _AXP_21264_CACHE_DEFS_ */
