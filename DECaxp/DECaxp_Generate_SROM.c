/*
 * Copyright (C) Jonathan D. Belanger 2018-2019.
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
 *  This source file contains the code to read in a CSV file and convert it to
 *  a SROM file that can be loaded during initial load of the Digital Alpha AXP
 *  21264 Emulator.
 *
 * Revision History:
 *
 *  V01.000 03-Feb-2018 Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001 01-Jun-2019 Jonathan D. Belanger
 *  Reformatted to remove tabs and be consistent with other source files.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Utility.h"

/*
 * main
 *  This function is called by the image activator.
 *
 * Input Parameters:
 *  argc:
 *      A value indicating the number of entries in the argv parameter.  This
 *      parameter is always one more than the actual arguments provided on the
 *      command line.
 *  argv:
 *      An array, limit argc, of strings representing the image filename being
 *      executed, please each of the arguments provided on the command line.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  0:  Normal Successful Completion.
 *  !0: An Error occurred that is causing the image to exit.
 */
int main(int argc, char **argv)
{
    char *inFile;
    char *outFile;
    FILE *inFP;
    AXP_SROM_HANDLE sromHandle;
    char readLine[133];
    u32 instruction;
    u32 noop = 0x47ff041f; /* BIS R31,R31,R31 */
    u32 blkOffset = 0;
    int retVal = 0;
    bool done = false;
    bool opened = false;

    printf("\n%%DECAXP-I-START, The Digital Alpha AXP 21264 CPU SROM Generator"
           " is starting.\n");
    if (argc == 3)
    {
        inFile = argv[1];
        outFile = argv[2];

        /*
         * TODO: Look at using access() to check for read access to the input
         *       file before trying to open it.  This way we can return a
         *       better error message.
         */
        inFP = fopen(inFile, "r");
        if (inFP != NULL)
        {
            char *ptr;
            char *addr = &readLine[0];
            char *instr = &readLine[9];

            if (fgets(readLine, 132, inFP) != NULL)
            {
                u32 binAddr;
                u64 baseAddr = 0;

                addr[8] = '\0';
                instr[8] = '\0';
                binAddr = strtoul(addr, &ptr, 16);
                baseAddr = binAddr; /* convert from 32-bit to 64-bit */
                done = AXP_OpenWrite_SROM(outFile,
                                          &sromHandle,
                                          baseAddr,
                                          AXP_ROM_FWID_SROM);
                if (done == false)
                {
                    opened = true;
                }
                else
                {
                    retVal = -4;
                }
            }
            while ((feof(inFP) == 0) && (done == false))
            {
                instruction = strtoul(instr, &ptr, 16);
                done = AXP_Write_SROM(&sromHandle, &instruction, 1);
                if (done == false)
                {
                    blkOffset = (blkOffset + 1) & 0x03;
                    done = (fgets(readLine, 132, inFP) == NULL);
                    addr[8] = '\0';
                    instr[8] = '\0';
                }
            }
            if (opened == true)
            {
                int ii;

                done = false;
                if (blkOffset != 0)
                {
                    for (ii = blkOffset; ii < 4; ii++)
                    {
                        done = AXP_Write_SROM(&sromHandle, &noop, 1);
                    }
                }
                if (done == false)
                {
                    done = AXP_Close_SROM(&sromHandle);
                    if (done == true)
                    {
                        retVal = -6;
                    }
                }
                else
                {
                    retVal = -5;
                }
            }

            /*
             * Close the input file.
             */
            fclose(inFP);
        }
        else if (inFP == NULL)
        {
            printf("\nUnable to open file %s for reading.\n", inFile);
            retVal = -2;
        }
        else
        {
            printf("\nUnable to open file %s for writing.\n", outFile);
            retVal = -3;
        }
    }
    else
    {
        printf("\nusage: %s <input-file> <output-file>\n", argv[0]);
        retVal = -1;
    }

    /*
     * Return the final status back.
     */
    return (retVal);
}

