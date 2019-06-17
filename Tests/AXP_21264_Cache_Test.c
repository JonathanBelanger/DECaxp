/*
 * Copyright (C) Jonathan D. Belanger 2017-2019.
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
 *  This source file contains the main function to test both the instruction
 *  and Data cache (Icache & Dcache) code.
 *
 * Revision History:
 *
 *  V01.000 13-Aug-2017 Jonathan D. Belanger
 *  Initially written.
 */
#include "CommonUtilities/AXP_Blocks.h"
#include "CPU/AXP_21264_CPU.h"
#include "CPU/Ibox/AXP_21264_Ibox.h"

#ifndef AXP_TEST_DATA_FILES
#define AXP_TEST_DATA_FILES "."
#endif
#define AXP_MAX_FILENAME_LEN    256

int parseLine(char *line, u32 *oper, u32 *addr, u32 *data)
{
    int retVal = 0;
    int ii = 0;
    int len = strlen(line);
    bool count;

    while ((line[ii] != ' ') &&
           !(((line[ii] >= '0') && (line[ii] <= '9')) ||
             ((line[ii] >= 'a') && (line[ii] <= 'f'))))
    {
        ii++;
    }
    *oper = line[ii] - '0';
    retVal++;
    ii += 2;
    *addr = 0;
    count = false;
    while ((ii < len) &&
           (((line[ii] >= '0') && (line[ii] <= '9')) ||
            ((line[ii] >= 'a') && line[ii] <= 'f')))
    {
        *addr *= 16;
        if ((line[ii] >= '0') && (line[ii] <= '9'))
        {
            *addr += (line[ii] - '0');
        }
        else
        {
            *addr += (10 + line[ii] - 'a');
        }
        ii++;
        count = true;
    }
    if (count == true)
    {
        retVal++;
    }
    ii++;
    *data = 0;
    count = false;
    while ((ii < len) && (line[ii] != 13))
    {
        *data *= 16;
        if ((line[ii] >= '0') && (line[ii] <= '9'))
        {
            *data += (line[ii] - '0');
        }
        else
        {
            *data += (10 + line[ii] - 'a');
        }
        ii++;
        count = true;
    }
    if (count == true)
    {
        retVal++;
    }

    return (retVal);
}

int main()
{
    AXP_21264_CPU *cpu = NULL;
    AXP_21264_TLB *itb;
    AXP_INS_LINE nextIns;
    AXP_PC pc;
    AXP_EXCEPTIONS except;
    u64 zeroPCval = 0;
    AXP_PC zeroPC;
    u32 noOp = 0x47ff041f;
    u32 readIns[AXP_ICACHE_LINE_INS];
    FILE *fp = NULL;
    char *fileNames[] = {"compress.trace", "tex.trace", "cc.trace", NULL};
    char fileName[AXP_MAX_FILENAME_LEN];
    char *line;
    size_t lineLen = 32;
    int ii = 0;
    u32 oper, addr, data, items;
    u64 va, pa;
    AXP_DCACHE_LOC dataLoc;
    u32 fault = 0;
    u32 fetchedData;
    bool _asm;
    u32 readMiss, writeMiss, instrMiss;
    u32 writeWayMiss, instrWayMiss;
    u32 readHit, writeHit, instrHit;
    u32 readData, writeData, readInst;
    u32 totalOper;

    printf("\nAXP 21264 Data and Instruction Cache Tester\n");
    AXP_PUT_PC(zeroPC, zeroPCval);
    line = AXP_Allocate_Block(-lineLen, NULL);
    cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);

    /*
     * We need to set some things in the cpu to set the environment in which
     * the cache processing works.  We set the following items:
     *  - memory access is being done in user more
     *  - the DTE PTR is set to allow user-more reading and writing
     *  - both cache sets in the I-Cache and D-Cache are enabled
     *  - allocate a 1MB B-Cache (accessed when a Dcache block is evicted or
     *    flushed)
     */
    if (cpu != NULL)
    {
        u32 bCacheSize = ONE_M / AXP_BCACHE_BLOCK_SIZE;

        cpu->ierCm.cm = AXP_CM_USER;    /* Run this in user mode            */
        cpu->dtbPte0.ure = 1;           /* Allow for user-mode read access  */
        cpu->dtbPte0.uwe = 1;           /* Allow for user-mode write access */
        cpu->dtbPte1 = cpu->dtbPte0;    /* DTB_PTE0 must equal DTB_PTE1     */
        cpu->iCtl.ic_en = 3;            /* Use both Icache sets             */
        cpu->dcCtl.set_en = 3;          /* Use both Dcache sets             */
        cpu->bCache = AXP_Allocate_Block(-(bCacheSize *
                                           sizeof(AXP_21264_BCACHE_BLK)),
                                         cpu->bCache);
        cpu->bTag = AXP_Allocate_Block(-(bCacheSize *
                                         sizeof(AXP_21264_BCACHE_TAG)),
                                       cpu->bTag);
    }
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
    {
        readIns[ii] = noOp;
    }
    memset(&nextIns, 0, sizeof(nextIns));
    ii = 0;
    while ((fileNames[ii] != NULL ) && (cpu != NULL ))
    {
        printf("\n>>> Processing file: %s\n", fileNames[ii]);
        sprintf(fileName, "%s/%s", AXP_TEST_DATA_FILES, fileNames[ii]);
        if ((fp = fopen(fileName, "r")) != NULL)
        {
            while (getline(&line, &lineLen, fp) != -1)
            {
                totalOper++;
                oper = addr = data = 0;
                items = parseLine(line, &oper, &addr, &data);
                if ((items != 2) && (items != 3))
                {
                    printf("%s\n", line);
                    printf("%u %08x %08x\n", oper, addr, data);
                    printf("parseLine did not return the number of expected items %u\n",
                           items);
                    abort();
                }
                switch (oper)
                {
                    case 0: /* Read from Memory */
                        readData++;

                        /*
                         * The data should be in the Dcache block, so if we get
                         * a Miss, then we have an error (at least for this
                         * test).
                         */
                        va = addr;
                        pa = AXP_va2pa(cpu,
                                       va,
                                       zeroPC,
                                       true,
                                       Read,
                                       &_asm,
                                       &fault,
                                       &except);

                        /*
                         * For all the reads, the call to convert from VA
                         * to PA should not generate a fault.
                         */
                        if ((fault == AXP_DTBM_DOUBLE_3) ||
                            (fault == AXP_DTBM_DOUBLE_4) ||
                            (fault == AXP_DTBM_SINGLE))
                        {
                            readMiss++;

                            /*
                             * First create an Address Translation Buffer.
                             */
                            AXP_addTLBEntry(cpu, va, va, true);

                            /*
                             * Now try and convert the virtual address to a
                             * physical one.
                             */
                            pa = AXP_va2pa(cpu,
                                           va,
                                           zeroPC,
                                           true,
                                           Read,
                                           &_asm,
                                           &fault,
                                           &except);
                            if (fault != 0)
                            {
                                printf("Got a va2pa(Read 2) fault 0x%04x\n",
                                       fault);
                                abort();
                            }
                        }
                        else if (fault != 0)
                        {
                            printf("Got a va2pa(Read 1) fault 0x%04x\n", fault);
                            abort();
                        }
                        else
                        {
                            readHit++;
                        }
                        break;

                    case 1: /* Write to Memory */
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
                        pa = AXP_va2pa(cpu,
                                       va,
                                       zeroPC,
                                       true,
                                       Write,
                                       &_asm,
                                       &fault,
                                       &except);
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
                            pa = AXP_va2pa(cpu,
                                           va,
                                           zeroPC,
                                           true,
                                           Write,
                                           &_asm,
                                           &fault,
                                           &except);
                        }
                        else if (fault != 0)
                        {
                            printf("Got a va2pa(Write 2) fault 0x%04x\n",
                                   fault);
                            abort();
                        }

                        /*
                         * At this point one of the above calls to convert from
                         * a virtual address to a physical one.  If not, then
                         * we have an unexpected error.
                         */
                        if (fault == 0)
                        {
                            u32 dCacheStatus;

                            /*
                             * At this point we don't know if the data to be
                             * saved is already in the Data Cache or not.  Call
                             * to get the status.  Then either write or update
                             * it.
                             */
                            if (AXP_Dcache_Status(cpu,
                                                  va,
                                                  pa,
                                                  sizeof(fetchedData),
                                                  true,
                                                  &dCacheStatus,
                                                  &dataLoc,
                                                  false) == NoException)
                            {
                                if ((dCacheStatus & AXP_21264_CACHE_HIT) ==
                                    AXP_21264_CACHE_HIT)
                                {
                                    writeHit++;
                                    AXP_DcacheWrite(cpu,
                                                    &dataLoc,
                                                    sizeof(data),
                                                    &data,
                                                    0);
                                }
                                else
                                {
                                    writeMiss++;
                                    AXP_DcacheWrite(cpu,
                                                    &dataLoc,
                                                    sizeof(data),
                                                    &data,
                                                    0);
                                }
                            }
                            else
                            {
                                printf("Got an DataAlignment exception (Write "
                                       "1) when we should have!\n");
                                abort();
                            }
                        }
                        else
                        {
                            printf("Got a va2pa(Write 1) fault 0x%04x\n",
                                   fault);
                            abort();
                        }
                        break;

                    case 2: /* Read instruction from Memory */
                        readInst++;

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
                        va = addr; /* We use this when searching for the TLB */
                        AXP_PUT_PC(pc, va);
                        if (AXP_IcacheFetch(cpu, pc, &nextIns) == false)
                        {
                            AXP_PC boundaryPC;
                            u64 tmpAddr = addr;

                            tmpAddr &= 0x7ffffc0; /* Round to 64 byte boundary */
                            AXP_PUT_PC(boundaryPC, tmpAddr);

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
                                AXP_IcacheAdd(cpu, boundaryPC, readIns, itb);
                            }
                            else
                            {
                                instrMiss++;
                                AXP_IcacheAdd(cpu, boundaryPC, readIns, itb);
                            }
                            if (AXP_IcacheFetch(cpu, pc, &nextIns) == false)
                            {
                                printf("AXP_IcacheFetch unexpectedly returned false\n");
                                abort();
                            }
                        }
                        else
                        {
                            instrHit++;
                        }
                        break;
                }
            }
            AXP_DcacheFlush(cpu);
            AXP_IcacheFlush(cpu, false);
        }
        fclose(fp);
        fp = NULL;
        ii++;
    }
    printf("Total operations executed: %u\n\n", totalOper);
    printf("Total reads from Dcache: %u\n", readData);
    printf("Total reads that Hit in the Dcache: %u\n", readHit);
    printf("Total reads that Missed in the Dcache: %u\n\n", readMiss);

    printf("Total writes to Dcache: %u\n", writeData);
    printf("Total writes that Hit the Dcache (updates): %u\n", writeHit);
    printf("Total writes that Missed the Dcache (adds): %u\n", writeMiss);
    printf("Total writes that Missed the Dcache and DTB: %u\n\n", writeWayMiss);

    printf("Total reads from Icache: %u\n", readInst);
    printf("Total reads that Hit in the Icache: %u\n", instrHit);
    printf("Total reads that Missed in the Icache: %u\n", instrMiss);
    printf("Total reads that Missed the Icache and the ITB: %u\n\n",
           instrWayMiss);
    return (0);
}
