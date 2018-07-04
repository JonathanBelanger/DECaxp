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
 *  This module contains the code to test disk emulation.
 *
 * Revision History:
 *
 *  V01.000	01-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Trace.h"

int main(void)
{
    char *diskPath = "/cygdrive/g/git/DECaxp/src/tst";
    char *diskName = "RZ1CD-CS.dsk";
#if 0
    char *modelNumber = "ST34501WC";
    char *serialNumber = "LG564729";
    char *revisionNumber = "A02";
#endif

    printf("\nDECaxp Disk Testing...\n");
    printf(
	"\nTest 1: Create a disk in %s with the name of %s of %d bytes "
	"in size...\n",
	diskPath,
	diskName,
	(3 *ONE_M));


    /*
     * Return back to the caller.
     */
    return(0);
}

