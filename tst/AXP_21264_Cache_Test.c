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
	AXP_21264_TLB	*dtb, *itb;
	AXP_INS_LINE	nextIns;
	AXP_PC			pc;
	u32				noOp = 0x47ff041f;
	u32				readIns[AXP_ICACHE_LINE_INS];
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
	u32				oper, addr, data, items;
	u64				va, pa, dataLoc;
	u32				fault = 0;
	u32				fetchedData;
	bool			_asm;
	u32				readMiss, writeMiss, instrMiss;
	u32				writeWayMiss, instrWayMiss;
	u32				readHit, writeHit, instrHit;
	u32				readData, writeData, readInst;
	u32				totalOper;

	printf("\nAXP 21264 IEEE Floating Point Tester\n");
	cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);
	cpu->iCtl.ic_en = 3;	/* Use both Icache sets */
	cpu->dcCtl.set_en = 3;	/* Use both Dcache sets */
	readMiss = 0;
	writeMiss = 0;
	instrMiss = 0;
	writeWayMiss = 0;
	instrWayMiss = 0;
	readHit = 0;
	writeHit = 0;
	instrHit = 0;
	readData = 0;
	writeData = 0;
	readInst = 0;
	totalOper = 0;

	/*
	 * We don't actually read in any real instructions or store any specific
	 * ones.  We are just going to fill in the 16 instructions "read from
	 * memory" as NoOps.  When we get a Miss, this is what is filled in.
	 */
	for (ii = 0; ii < AXP_ICACHE_LINE_INS; ii++)
		readIns[ii] = noOp;

	while ((fileNames[ii] != NULL) && (cpu != NULL))
	{
		if ((fp = fopen(fileNames[ii], "r")) != NULL)
		{
			while (getline(&line, &lineLen, fp) != -1)
			{
				totalOper++;
				items = sscanf(line, "%d %06x %08x", &oper, &addr, &data);
				switch (oper)
				{
					case 0:		/* Read from Memory */
						readData++;

						/*
						 * The data should be in the Dcache block, so if we get
						 * a Miss, then we have an error (at least for this
						 * test).
						 */
						va = addr;
						pa = AXP_va2pa(cpu, va, 0, true, Read, &_asm, &fault);

						/*
						 * For all the reads, the call to convert from VA
						 * to PA should not generate a fault.
						 */
						if (fault != 0)
						{
							readMiss++;
							/* printf("Got a va2pa(Read) fault 0x%04x\n", fault);
							abort();	*/
						}
						else
						{
							readHit++;
						}
					break;

				case 1:		/* Write to Memory */
					writeData++;

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
					va = addr;
					pa = AXP_va2pa(cpu, va, 0, true, Write, &_asm, &fault);
					if ((fault == AXP_DTBM_DOUBLE_3) ||
						(fault == AXP_DTBM_DOUBLE_4) ||
						(fault == AXP_DTBM_SINGLE))
					{
						writeWayMiss++;

						/*
						 * First create an Address Translation Buffer.
						 */
						AXP_addTLBEntry(cpu, va, va, true);

						/*
						 * Now try and convert the virtual address to a
						 * physical one.
						 */
						pa = AXP_va2pa(cpu, va, 0, true, Write, &_asm, &fault);
					}
					else if (fault != 0)
					{
						printf("Got a va2pa(Read) fault 0x%04x\n", fault);
						abort();
					}

					/*
					 * At this point one of the above calls to convert from
					 * a virtual address to a physic al one.  If not, then
					 * we have an unexpected error.
					 */
					if (fault == 0)
					{

						/*
						 * At this point we don't knoe if the data to be
						 * saved is already in the Data Cache or not.  We
						 * first need to fetch it.  If that fails, then we
						 * either add it or update it.
						 */
						if (AXP_DcacheFetch(
									cpu,
									va,
									pa,
									sizeof(fetchedData),
									&fetchedData,
									&dataLoc) == false)
						{
							writeHit++;
							AXP_DcacheUpdate(
									cpu,
									va,
									pa,
									sizeof(data),
									data,
									dataLoc);
						}
						else
						{
							writeMiss++;
							AXP_DcacheAdd(cpu, va, pa, sizeof(data), data);
						}
					}
					else
					{
						printf("Got a va2pa(Read) fault 0x%04x\n", fault);
						abort();
					}
					break;

				case 2:		/* Read instruction from Memory */
					readInstr++;

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
					 *
					 * The input to this test code indicates where an
					 * instruction should be fetched from, but does not
					 * contain any instructions.  When we are told to
					 * fetch an instruction and it is not in the cache,
					 * then we'll actually store 16 no-op instructions,
					 * one a 64 byte boundary.
					 */
					va = addr;
					if (AXP_IcacheFetch(cpu, va, &nextIns) == false)
					{
						itb = AXP_findTLBEntry(cpu, va, false);
						if (itb == NULL)
						{
							instrWayMiss++;

							/*
							 * First create an Address Translation Buffer.
							 */
							AXP_addTLBEntry(cpu, va, va, false);
							itb = AXP_findTLBEntry(cpu, va, false);
							if (itb == NULL)
							{
								printf("AXP_findTLBNEntry(Icache) unexpectedly returned NULL\n");
								abort();
							}
							va &= 0x7ffffc0;	/* Round to currrent/previous 64 byte boundary */
							AXP_IcacheAdd(
									cpu,
									*((AXP_PC *) &va),
									nextIns,
									itb);
						}
						else
							instrMiss++;
						if (AXP_IcacheFetch(cpu, va, &nextIns) == false)
						{
							printf("AXP_IcacheFetch unexpectedly returned false\n");
							abort();
						}
					}
					else
						instrHit++:
					break;
				}
			}
		}
		fclose(fp);
		fp = NULL;
		ii++;
	}
	printf("Total operations executed: %d\n\n", totalOper);
	printf("Total reads from Dcache: %d\n", readData);
	printf("Total reads that Hit in the Dcache: %d\n", readHit);
	printf("Total reads that Missed in the Dcache: %d\n\n", readMiss);

	printf("Total writes to Dcache: %d\n", writeData);
	printf("Total writes that Hit the Dcache (updates): %d\n", writeHit);
	printf("Total writes that Missed the Dcache (adds): %d\n", writeMiss);
	printf("Total writes that Missed the Dcache and DLB: %d\n\n", writeWayMiss);

	printf("Total reads from Icache: %d\n", readInst);
	printf("Total reads that Hit in the Icache: %d\n", instrHit);
	printf("Total reads that Missed in the Icache: %d\n", instrMiss);
	printf("Total reads that Missed the Icache and the ITB: %d\n\n", instrWayMiss);
	return(0);
}
