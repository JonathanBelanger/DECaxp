#define AXP_CM_KERNEL		0
#define AXP_SPE2_VA_MASK	0x00000fffffffe000ll
#define AXP_SPE1_VA_MASK	0x000001ffffffe000ll
#define AXP_SPE1_PA_MASK	0x00000e0000000000ll
#define AXP_SPE1_VA_40		0x0000010000000000ll
#define AXP_SPE0_VA_MASK	0x000000003fffe000ll

u64 AXP_va2pa(AXP_21264_CPU *cpu, u64 va, AXP_PC pc, bool ins)
{
	u64 pa = 0x0ll;
	u8	spe = (ins ? cpu->iCtl.spe : cpu->mCtl.spe);

	if ((spe != 0) && (cpu->ierCm.cp == AXP_CM_KERNEL))
	{
		if ((spe & 0x4) && (va.spe2 == 2))
		{
			pa = va & AXP_SPE2_VA_MASK;
			/* asm bit = false ? */
			return(pa);
		}
		else if ((spe & 0x2) && (va.spe1 == 0x7e))
		{
			pa = (va & AXP_SPE1_VA_MASK) | (va & AXP_SPE1_VA_40 ? AXP_SPE1_PA_43_41 : 0;
			/* asm bit = false ? */
			return(pa);
		}
		else if ((spe & 0x1) && (va.spe0 == 0x3fffe))
		{
			pa = va & AXP_SPE0_VA_MASK;
			/* asm bit = false ? */
			return(pa);
		}
	}
	
	/* get the TB entry */
	if (tb == NULL)
	{
		if (pc.pal != AXP_PAL_MODE)
		{
			cpu->excAddr = pc;
			/* need to figure out how to recognize a double miss */
			if (single fault)
				cpu->mmStat.? = 1;
			cpu->faultVa = va;
			cpu->excSub.? = 1;
			/* need to tell the Ibox to call one of the TB miss PALcode entries */
		}
		else
		{
			if (double miss)
			{
				/* handle the double miss */
			}
			else if (ins == true)
			{
				/* handle the ITB miss */
			}
			else
			{
				/* handle the DTB miss */
			}
			pa = va2pa(cpu, va, pc, ins);
		}
	}
	else
	{
		if (tb->access != reqAccess)
		{
			cpu->excAddr = pc;
			if (inst == true)
			{
				cpu->excSum = 0;
				if (pc.pal == AXP_PAL_MODE)
					/* handle this IACV in PALmode */
				else
					/* need to tell iBox to call the IACV PALcode */
			}
			else
			{
				cpu->excSum.? = 1;
				cpu->mmStat.? = 1;
				cpu->faultVa = va;
				if (pc.pal == AXP_PAL_MODE)
					/* handle this DFAULT in PALmode */
				else
					/* need to tell iBox to call the DFAULT PALcode */
			}
		}
		else
		{
			pa = tb->physAddr | (va & tb->keepMask)
			/* asmBit? = tb->asm; */
		}
	}
	return(pa);
}
