/*
 * Copyright (C) Jonathan D. Belanger 2018.
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
 *	This source file contains the code to read in a CSV file and convert it to
 *	a SROM file that can be loaded during initial load of the Digital Alpha AXP
 *	21264 Emulator.
 *
 * Revision History:
 *
 *	V01.000		03-Feb-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_Utility.h"

/*
 * main
 *	This function is called by the image activator.
 *
 * Input Parameters:
 *	argc:
 *		A value indicating the number of entries in the argv parameter.  This
 *		parameter is always one more than the actual arguments provided on the
 *		command line.
 *	argv:
 *		An array, limit argc, of strings representing the image filename being
 *		executed, please each of the arguments provided on the command line.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	0:	Normal Successful Completion.
 *	!0:	An Error occurred that is causing the image to exit.
 */
int main(int argc, char **argv)
{
	char	*inFile;
	char	*outFile;
	FILE	*inFP;
	u32		*SROMbuffer = NULL;
	char	readLine[133];
	u64		baseAddr = 0;
	int		retVal = 0;
	int		ii = 0;
	int		SROMsize;
	bool	baseAddrSet = false;

	printf("\n%%DECAXP-I-START, The Digital Alpha AXP 21264 CPU Emulator is starting.\n");
	if (argc == 3)
	{
		inFile = argv[1];
		outFile = argv[2];

		inFP = fopen(inFile, "r");
		if (inFP != NULL)
		{
			char	*addr = &readLine[0];
			char	*instr = &readLine[9];

			SROMsize = 4;
			SROMbuffer = calloc(SROMsize, sizeof(u32));
			while (feof(inFP) && (SROMbuffer != NULL))
			{
				char *ptr;

				if (fgets(readLine, 132, inFP) != NULL)
				{
					addr[8] = '\0';
					instr[17] = '\0';
					if (baseAddrSet == false)
					{
						u32	binAddr = strtoul(addr, &ptr, 16);

						baseAddr = binAddr;
						baseAddrSet = true;
					}
					if (ii >= SROMsize)
					{
						SROMsize += 4;
						SROMbuffer = realloc(SROMbuffer, SROMsize);
					}
					if (SROMbuffer != NULL)
					{
						SROMbuffer[ii] = stroul(instr, &ptr, 16);
						ii++;
					}
				}
			}
			if (SROMbuffer != NULL)
			{
				AXP_SROM_HANDLE sromHandle;
				u32				fwID;
				int 			jj;
				bool			SROMstatus;

				if (ii < SROMsize)
					for (jj = ii; jj < SROMsize; jj++)
						SROMbuffer[jj] = 0x47ff041f;	/* BIS R31,R31,R31 */
				fwID = time(NULL);
				SROMstatus = AXP_OpenWrite_SROM(
							outFile,
							&sromHandle,
							baseAddr,
							fwID);
				if (SROMstatus == false)
					SROMstatus = AXP_Write_SROM(
										&sromHandle,
										SROMbuffer,
										(SROMsize * 4));
				else
					retVal = -4;
				if ((retVal == 0) && (SROMstatus == false))
				{
					SROMstatus = AXP_Close_SROM(&sromHandle);
					if (SROMstatus == true)
						retVal = -6;
				}
				else
					retVal = -5;

			}
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
	return(retVal);
}

