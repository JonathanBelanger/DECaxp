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
 *	This source file contains the main function to test both the instruction
 *	and Data cache (Icache & Dcache) code.
 *
 * Revision History:
 *
 *	V01.000		13-Aug-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_Ibox.h"

int main()
{
	AXP_21264_CPU	*cpu = NULL;
	AXP_21264_TLB	*dtb;
	FILE			*fp = NULL;
	char			*fileNames[] =
	{
		"../tst/compress.trace",
		"../tst/tex.trace",
		"../tst/cc.trace",
		NULL
	};
	char			*line = NULL;
	int				lineLen = 0;
	int				ii = 0;
	int				oper, addr, data, items;
	u64				va, pa;

	printf("\nAXP 21264 IEEE Floating Point Tester\n");
	cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);
	cpu->iCtl.ic_en = 3;	/* Use both Icache sets */
	cpu->dcCtl.set_en = 3;	/* Use both Dcache sets */

	while ((fileNames[ii] != NULL) && (cpu != NULL))
	{
		if ((fp = fopen(fileNames[ii], "r")) != NULL)
		{
			while (getline(&line, &lineLen, fp) != -1)
			{
				items = sscanf(line, "%d %06x %08x", &oper, &addr, &data);
				switch(oper)
				{
					case 0:		/* Read from Memory */

						/*
						 * The data should be in the Dcache block, so if we get
						 * a Miss, then we have an error (at least for this
						 * test).
						 */
						va = addr;
						break;

					case 1:		/* Write to Memory */

						/*
						 * First check that the memory item is not already in
						 * the Dcache.  If it is, then update the cache record
						 * (using a bit of slight of hand because we are not
						 * using instructions to manipulate the cache).  If the
						 * memory item is not already in the cache, then  add
						 * it.  Note, we will have to handle the Data
						 * Translation Look-aside Buffer (DTB) because it is
						 * used to locate and store items in the cache.  We may
						 * also have to perform a virtual to physical address
						 * translation as well.  BTW: The DTB is going to be
						 * setup to have virtual and physical addresses be the
						 * same (at least for the first attempt at this test
						 * code).
						 */
						va = pa = addr;
						break;

					case 2:		/* Read instruction from Memory */

						/*
						 * This is just an emulation.  There is no attempt to
						 * actually interpret what is returned.  So, what we
						 * will do is, when an instruction that is not in the
						 * Icache, we will create one.  Remember, each Icache
						 * block contains 16 32-bit instructions, which are
						 * aligned on a 64 byte boundary.  We will need to make
						 * sure that when creating an Icache block, that we do
						 * so on the 64 byte boundary, less than or equal to
						 * the virtual address of the instruction being read by
						 * clearing the low 5 bits of virtual address.
						 */
						va = addr;
						break;
				}
			}
		}
		fclose(fp);
		fp = NULL;
		ii++;
	}
	return(0);
}
