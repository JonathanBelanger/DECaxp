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
#include "Devices/VirtualDisks/AXP_VirtualDisk.h""
#include "CommonUtilities/AXP_Utility.h"
#include "CommonUtilities/AXP_Trace.h"
#include "Devices/VirtualDisks/AXP_VHD_Utility.h"

typedef struct
{
	u8	*buf;
	size_t	bufSize;
	u32	value;
	char	*name;
} crc32cTests;

static crc32cTests crc32cTestCases[] =
{
    /* Test 1 */
    {
	.buf = (u8 *) "a",
	.bufSize = 1,
	.value = 0xc1d04330,
	.name = "Just a lowercase \'a\'"
    },
    /* Test 2 */
    {
	.buf = (u8 *) "foo",
	.bufSize = 3,
	.value = 0xcfc4ae1d,
	.name = "Lowercase \'foo\'"
    },
    /* Test 3 */
    {
	.buf = (u8 *) "hello world",
	.bufSize = 11,
	.value = 0xc99465aa,
	.name = "Lowercase \'hello world\'"
    },
    /* Test 4 */
    {
	.buf = (u8 *) "hello ",
	.bufSize = 6,
	.value = 0x7e627e58,
	.name = "Lowercase \'hello \' (with a space at the end)"
    },
    /* Test 5 */
    {
	.buf = (u8 *) "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	       "\0\0\0\0\0",
	.bufSize = 32,
	.value = 0x8a9136aa,
	.name = "Null string of 32 bytes"
    },
    /* Test 6 */
    {
	.buf = (u8 *) "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	       "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	       "\xff\xff\xff\xff",
	.bufSize = 32,
	.value = 0x62a8ab43,
	.name = "32 bytes of 0xff"
    },
    /* Test 7 */
    {
	.buf = (u8 *) "\x1f\x1e\x1d\x1c\x1b\x1a\x19\x18\x17\x16\x15\x14\x13"
	       "\x12\x11\x10\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04"
	       "\x03\x02\x01\x00",
	.bufSize = 32,
	.value = 0x113fdb5c,
	.name = "Nonprintable characters from 0x1f down to 0x00"
    },
    /* Test 8 */
    {
	.buf = (u8 *) "\x01\xc0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	       "\x00\x00\x00\x14\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x14"
	       "\x00\x00\x00\x18\x28\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00"
	       "\x00\x00\x00\x00\x00",
	.bufSize = 48,
	.value = 0xd9963a56,
	.name = "Various nonprintable characters"
    },
    /* Test 9 */
    {
	.buf = (u8 *) "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c"
	       "\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b"
	       "\x1c\x1d\x1e\x1f",
	.bufSize = 32,
	.value = 0x46dd794e,
	.name = "Nonprintable characters from 0x00 to 0x1f"
    },
    /* Test 10 */
    {
	.buf = (u8 *) "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d"
	       "\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c"
	       "\x1d\x1e\x1f !\"#$\%&\'(",
	.bufSize = 40,
	.value = 0x0e2c157f,
	.name = "Nonprintable and Printable from \' \' to \'(\' characters"
    },
    /* Test 11 */
    {
	.buf = (u8 *) ")*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOP",
	.bufSize = 40,
	.value = 0xe980ebf6,
	.name = "Printable from \')\' to \'P\'"
    },
    /* Test 12 */
    {
	.buf = (u8 *) "QRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwx",
	.bufSize = 40,
	.value = 0xde74bded,
	.name = "Printable from \'Q\' to \'x\'"
    },
    /* Test 13 */
    {
	.buf = (u8 *) "yz{|}~\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a"
	       "\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99"
	       "\x9a\x9b\x9c\x9d\x9e\x9f\xa0",
	.bufSize = 40,
	.value = 0xd579c862,
	.name = "Printable from \'y\' to \'~\' and Nonprintable characters"
    },
    /* Test 14 */
    {
	.buf = (u8 *) "\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad"
	       "\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc"
	       "\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8",
	.bufSize = 40,
	.value = 0xba979ad0,
	.name = "Nonprintable characters from 0xa1 to 0xc8"
    },
    /* Test 15 */
    {
	.buf = (u8 *) "\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5"
	       "\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4"
	       "\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0",
	.bufSize = 40,
	.value = 0x2b29d913,
	.name = "Nonprintable characters from 0xc9 to 0xf0"
    },
    /* Test 16 */
    {
	.buf = (u8 *) "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d"
	       "\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c"
	       "\x1d\x1e\x1f !\"#$\%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJ"
	       "KLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f\x80"
	       "\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
	       "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e"
	       "\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad"
	       "\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc"
	       "\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb"
	       "\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda"
	       "\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9"
	       "\xea\xeb\xec\xed\xee\xef\xf0",
	.bufSize = 240,
	.value = 0x24c5d375,
	.name = "Nonprintable and all the printable characters"
    },
    /* Test 17 */
    {
	.buf = (u8 *) "123456789",
	.bufSize = 9,
	.value = 0xe3069283,
	.name = "The string \'123456789\'"
    },
    {
	.bufSize = 0
    }
};

int main(void)
{
    AXP_VHD_CREATE_PARAM 	createParam;
    AXP_VHD_STORAGE_TYPE	storageType;
    AXP_VHD_HANDLE		handle;
    char			*diskPath = "/cygdrive/g/git/DECaxp/src/tst/VHDTests";
    char			*diskName = "RZ1CD-CS.vhdx";
    char			fullPath[256];
#if 0
    char			*modelNumber = "ST34501WC";
    char			*serialNumber = "LG564729";
    char			*revisionNumber = "A02";
#endif
    i32				retVal = AXP_VHD_SUCCESS;
    u32				crcCalc;
    int				ii = 0;

    printf("\nDECaxp Disk Testing...\n\n");

    while (crc32cTestCases[ii].bufSize != 0)
    {
	crcCalc = AXP_Crc32(
			crc32cTestCases[ii].buf,
			crc32cTestCases[ii].bufSize,
			false,
			0);
	printf("Test %d: %s: [actual=%08x, expected=%08x] - %s\n",
	    (ii + 1),
	    crc32cTestCases[ii].name,
	    crcCalc,
	    crc32cTestCases[ii].value,
	    (crcCalc == crc32cTestCases[ii].value ? "Passed" : "Failed"));
	ii++;
    }

    createParam.ver = CREATE_VER_1;
    uuid_clear(createParam.ver_1.GUID.uuid);
    createParam.ver_1.maxSize = 3 * ONE_M;
    createParam.ver_1.blkSize = AXP_VHD_DEF_BLK;
    createParam.ver_1.sectorSize = AXP_VHD_DEF_SEC;
    createParam.ver_1.parentPath = NULL;
    createParam.ver_1.srcPath = NULL;

    storageType.deviceID = STORAGE_TYPE_DEV_VHDX;
    AXP_VHD_KnownGUIDMemory(AXP_Vendor_Microsoft, &storageType.vendorID);

    printf(
	"Test %d: Create a VHDX(v%d) disk in %s with the name of %s of %llu "
	"bytes in size...\n",
	ii++,
	createParam.ver,
	diskPath,
	diskName,
	createParam.ver_1.maxSize);
    sprintf(fullPath, "%s/%s", diskPath, diskName);

    retVal = AXP_VHD_Create(
			&storageType,
			fullPath,
			ACCESS_NONE,
			NULL,
			CREATE_NONE,
			0,
			&createParam,
			NULL,
			&handle);
    if (retVal == AXP_VHD_SUCCESS)
    {
	printf("\t...Succeeded...\n");
	if (AXP_TraceInit() == true)
	{
	    AXP_Dump_VHD_Info(handle);
	}
	AXP_VHD_CloseHandle(handle);
    }
    else
	printf("\t...Failed...\n");

    /*
     * Return back to the caller.
     */
    printf("...Done.\n");
    return(0);
}

