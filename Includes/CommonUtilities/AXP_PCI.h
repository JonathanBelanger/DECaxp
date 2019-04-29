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
 *	This header file contains the definitions needed to support the PCI
 *	protocol.  The emulation of the PCI bus, will require the following
 *	steps:
 *	    1)	The Pchip thread will be created and initialized.
 *	    2)	The Pchip will initialize all known Configuration Spaces, as
 *		saved from the previous execution.
 *	    3)	The Pchip will create the following PCI targets:
 *		a) Time of Year (TOY) clock
 *		b) The console output
 *		c) The console keyboard
 *	    4)	The Pchip will scan the configuration information and create
 *		targets for each PCI device.
 *	    5)	As each PCI target is created it will be required to do the
 *		following:
 *		a) Initialize any VDP space
 *		b) Register itself as a PCI device, with the information needed
 *		   to be able to address and control the device.
 *	    6)	As each PCI target registers itself:
 *		a) If this device has not been seen before, its Configuration
 *		   Space will be initialized
 *		b) If this device has been seen before, then it's Configuration
 *		   Space will be marked as present.  NOTE: Previous
 *		   Configuration Space definitions will not be deleted.
 *	    7)	Both PCI targets and master should be ready for normal
 *		processing.
 *
 * Revision History:
 *
 *	V01.000		19-May-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_PCI_H_
#define _AXP_PCI_H_

#include "CommonUtilities/AXP_Utility.h"
#include "CommonUtilities/AXP_Configure.h"

/*
 * Let's start with the PCI commands.
 */
typedef enum
{
    InterruptAcknowledge,	/* b'0000' */
    SpecialCycle,		/* b'0001' */
    IORead,			/* b'0010' */
    IOWrite,			/* b'0011' */
    MemoryRead = 0x6,		/* b'0110' */
    MemoryWrite,		/* b'0111' */
    ConfigurationRead = 0xa,	/* b'1010' */
    ConfigurationWrite,		/* b'1011' */
    MemoryReadMultiple,		/* b'1100' */
    DualAddressCycle,		/* b'1101' */
    MemoryReadLine,		/* b'1110' */
    MemoryWriteAndInvalidate	/* b'1111' */
} AXP_PCI_CMD;

typedef struct
{
    AXP_PCI_CMD command;	/* C/BE[3:0]# */
    u64		addr;		/* AD[63:0] */
    u32		data;		/* AD[31:0] */
    bool	bit64;		/* Is addr 32 or 64 bits */
} AXP_PCI_MSG;

/*
 * Value of the Address field for the special cycle command.
 */
#define AXP_PCI_SHUTDOWN	0x0000
#define AXP_PCI_HALT		0x0001

/*
 * The logical block size for PCI is a DWORD, which is a 32-bit block.
 */
typedef union
{
    u32	l;
    u16	w[2];
    u8	b[4];
} AXP_DWORD;

/*
 * Configuration Space
 */
typedef struct
{
    u16 vendorID;
    u16 deviceID;
    u16 devCtrl;
    u16 status;
    u32 revision : 8;
    u32 classCode : 24;
    u8 cacheLineSize;
    u8 latencyTimer;
    u8 headerType;
    u8 bist;
    u32 baseAddrReg[6];		/* bar */
    u32 cardBusCISPtr;
    u16 subsystemVendorID;
    u16 subsystemID;
    u32 expansionROMBaseAddr;
    u8 capabilitiesPtr;
    u8 res_1[3];
    u32 res_2;
    u8 interruptLine;
    u8 interruptPin;
    u8 minGnt;
    u8 maxLat;
} AXP_PCI_CFG;

typedef union
{
    u16 devCtrl;
    struct
    {
	u16 ioSpace : 1;
	u16 memSpace : 1;
	u16 busMaster : 1;
	u16 specialCycles : 1;
	u16 memWriteInv : 1;
	u16 vgaPaletteSnoop : 1;
	u16 parityErrorRsp : 1;
	u16 steppingCtrl : 1;
	u16 serrNumEnable : 1;
	u16 fastB2BEnable : 1;
	u16 res : 6;
    };
} AXP_PCI_DEV_CTRL;

typedef union
{
    u16 status;
    struct
    {
	u16 res_1 : 4;
	u16 capList : 1;
	u16 sixtySixMHzCap : 1;
	u16 res_2 : 1;
	u16 fastB2BEnable : 1;
	u16 masterDataParityErr : 1;
	u16 devSelTiming : 2;
	u16 sentTargetAbt : 1;
	u16 rcvdTargetAbt : 1;
	u16 sentMasterAbt : 1;
	u16 rcvdMasterAbt : 1;
	u16 detectParityErr : 1;
    };
} AXP_PCI_DEV_STATUS;

#define AXP_DEVSEL_FAST	0
#define AXP_DEVSEL_MED	1
#define AXP_DEVSEL_SLOW	2

typedef union
{
    u8 bist;
    struct
    {
	u8 complCode : 4;
	u8 res : 2;
	u8 startBiST : 1;
	u8 bistCap : 1;
    };
} AXP_PCI_BIST;

typedef union
{
    u32 baseAddrReg;
    struct
    {
	u32 memSpaceInd : 1;	/* set to 0 for memory space */
	u32 type : 2;
	u32 prefetchable : 1;
	u32 baseAddr : 28;
    } mem;
    struct
    {
	u32 ioSpaceInd : 1;	/* set to 1 for IO space */
	u32 res : 1;
	u32 baseAddr : 30;
    } io;
} AXP_BAR;

#define AXP_PCI_BAR32	0
#define AXP_PCI_BAR64	2

typedef union
{
    u32 expansionROMBaseAddr;
    struct
    {
	u32 expROMEnable : 1;
	u32 res : 10;
	u32 expROMBAR : 21;
    };
} AXP_ROM_BAR;

typedef struct
{
    u16 romSig;			/* must be set to 0xaa55 */
    u8 procUniqueData[16];	/*  */
    u16 pciDataPtr;
} AXP_PCI_ROM_HDR;

#define AXP_PCI_ROM_SIG	0xaa55;

typedef struct
{
    char signature[4];		/* must be "PCIR" */
    u16 vendorID;
    u16 deviceID;
    u16 res_1;
    u16 length;
    u32 revision : 8;
    u32 classCode : 24;
    u16 imgLen;
    u16 revLvl;
    u8 codeType;
    u8 indicator;
    u16 res_2;
} AXP_PCI_DATA;

#define AXP_CODETYPE_X86	0
#define AXP_CODETYPE_OPEN	1
#define AXP_CODETYPE_HPPA	2
#define AXP_IND_LAST(ind)	(((ind) & 0x80) == 0x80)

/*
 * This macro determines the next base address for the PCI Expansion ROM Image
 * base upon the first byte immediately after the current image.  If the first
 * byte is specified as 0, then we are looking for the first image.  Images
 * must start on a 512-byte boundary.
 */
#define AXP_NEXT_IMAGE_BASE(nextByte)	(((nextByte) + 512) & 0xfffffe00)

typedef struct
{
    u8 id;
    u8 nextCap;
    u8 cap[2];		/* there could be one or more bytes, just just 2 */
} AXP_PCI_CAPLIST;

typedef struct
{
    u8 capID;		/* Set to 0x05 for Message Signaled Interrupts (MSI) */
    u8 nextCap;
    u16 msgCtrl;
    u32 msgAddr;
    u16 msgData;
} AXP_PCI_MSI32;

typedef struct
{
    u8 capID; 		/* Set to 0x05 for Message Signaled Interrupts (MSI) */
    u8 nextCap;
    u16 msgCtrl;
    u32 msgAddrLower;
    u32 msgAddrUpper;
    u16 msgData;
} AXP_PCI_MSI64;

typedef union
{
    u16 msgCtrl;
    struct
    {
	u16 msiEnable : 1;
	u16 multMsgCap : 3;
	u16 multMsgEba : 3;
	u16 addr64 : 1;
	u16 res : 8;
    };
} AXP_MSGCTRL;

#define AXP_MSG_1	0
#define AXP_MSG_2	1
#define AXP_MSG_4	2
#define AXP_MSG_8	3
#define AXP_MSG_16	4
#define AXP_MSG_32	5

#define AXP_MSGADDR(addr)			((addr) & 0xfffffffc)
#define AXP_MSGADDR64(upper, lower)	(((upper) << 32) + (AXP_MSGADDR(lower))

typedef struct
{
    u8 capID;		/* Set to 0x03 for Vital Product Data */
    u8 nextCap;
    u16 vpdAddr : 15;
    u16 flag : 1;
    u32 vpdData;
} AXP_PCI_VPDCAP;

/*
 * VPD Example:
 *
 *	-----------------------------------------------------------------------
 *	Offset	Item						Value
 *	-----------------------------------------------------------------------
 *	0x00	Large Resource Type "ID String Tag (0x02)"	0x82
 *	0x01	Length (2 bytes)				0x0021
 *	0x03	Data						"ABCD
 *								 Super-Fast
 * 								 Widget
 * 								 Controller"
 *	0x24	Large Resource Type "VPD-R Tag (0x10)"		0x90
 *	0x25	Length (2 bytes)				0x0059
 *		-----------------------------------------------------------
 *		Offset	Tag, Length, and Data		Value
 *		-----------------------------------------------------------
 *		0x27	VPD Keyword (2 bytes)		"PN"
 *		0x29	Length (1 byte)			0x08
 *		0x2a	Data				"6181682A"
 *		0x32	VPD Keyword (2 bytes)		"EC"
 *		0x34	Length (1 byte)			0x0a
 *		0x35	Data				"4950262536"
 *		0x3f	VPD Keyword (2 bytes)		"SN"
 *		0x41	Length (1 byte)			0x08
 *		0x42	Data				"00000194"
 *		0x4a	VPD Keyword (2 bytes)		"MN"
 *		0x4c	Length (1 byte)			0x04
 *		0x4d	Data				"1037"
 *		0x50	VPD Keyword (2 bytes)		"RV"
 *		0x52	Length (1 byte)			0x2c
 *		0x53	Data				Checksum
 *		0x54	Data				Reserved (0x00)
 *		-----------------------------------------------------------
 *	0x80	Large Resource Type "VPD-W Tag (0x11)"		0x91
 *	0x81	Length (2 bytes)				0x007e
 *		-----------------------------------------------------------
 *		Offset	Tag, Length, and Data		Value
 *		-----------------------------------------------------------
 *		0x83	VPD Keyword (2 bytes)		"V1"
 *		0x85	Length (1 byte)			0x05
 *		0x86	Data				"65A01"
 *		0x8b	VPD Keyword (2 bytes)		"Y1"
 *		0x8d	Length (1 byte)			0x0d
 *		0x8e	Data				"Error Code 26"
 *		0x9b	VPD Keyword (2 bytes)		"RW"
 *		0x9d	Length (1 byte)			0x61
 *		0x9e	Data				Reserved (0x00)
 *		-----------------------------------------------------------
 *	0xff	Small Resource Type "End of Tag (0xf)"		0x78
 *	-----------------------------------------------------------------------
 */
typedef struct
{
    u16 keyword;
    u8 length;
    u8 data; /* first byte of data */
} AXP_PCI_VPD;

#define AXP_PCI_VPD_PN	0x4E40	/* Little-endian PN */
#define AXP_PCI_VPD_EC	0x4345	/* Little-endian EC */
#define AXP_PCI_VPD_MN	0x4E4D	/* Little-endian MN */
#define AXP_PCI_VPD_SN	0x4E53	/* Little-endian SN */
#define AXP_PCI_VPD_CP	0x5043	/* Little-endian CP */
#define AXP_PCI_VPD_RV	0x5652	/* Little-endian RV */
#define AXP_PCI_VPD_YA	0x4159	/* Little-endian YA */
#define AXP_PCI_VPD_RW	0x5752	/* Little-endian RW */
#define AXP_PCI_VPD_Vx	0x58	/* Vendor-specific V? */
#define AXP_PCI_VPD_Yx	0x59	/* System-specific Y? */

typedef struct
{
    u32 tag : 8;
    u32 length : 16;
    u32 data : 8;
} AXP_PCI_VPD_RES;

#define AXP_PCI_VPD_RES_ID	0x82
#define AXP_PCI_VPD_RES_RO	0x90
#define AXP_PCI_VPD_RES_RW	0x91
#define AXP_PCI_VPD_RES_END	0x78

#endif /* _AXP_PCI_H_ */
