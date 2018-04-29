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
 *	This header file contains the definitions to assist in address mapping that
 *	occurs in the Cchip, but may be used in other areas.
 *
 * Revision History:
 *
 *	V01.000		28-Apr-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP21274_ADDRESSMAPPING_H_
#define _AXP21274_ADDRESSMAPPING_H_

#include "AXP_Utility.h"
#include "AXP_Configure.h"

/*
 * Table 10–1 System Address Map
 *	Space			Size	System Address <43:0>		Comments
 *	System memory	4GB		000.0000.0000–000.FFFF.FFFF	Cacheable and
 *														prefetchable.
 *	Reserved		8188GB	001.0000.0000–7FF.FFFF.FFFF	—
 *	Pchip0 PCI		4GB		800.0000.0000–800.FFFF.FFFF Linear addressing.
 *	memory
 *	TIGbus			1GB		801.0000.0000–801.3FFF.FFFF	addr<5:0> = 0.
 *														Single byte valid in
 *														quadword access.
 *														16MB accessible.
 *	Reserved		1GB		801.4000.0000–801.7FFF.FFFF	—
 *	Pchip0 CSRs		256MB	801.8000.0000–801.8FFF.FFFF	addr<5:0> = 0.
 *														Quadword access.
 *	Reserved		256MB	801.9000.0000–801.9FFF.FFFF	—
 *	Cchip CSRs		256MB	801.A000.0000–801.AFFF.FFFF	addr<5:0> = 0.
 *														Quadword access.
 *	Dchip CSRs		256MB	801.B000.0000–801.BFFF.FFFF	addr<5:0> = 0.
 *														All eight bytes in
 *														quadword access must be
 *														identical.
 *	Reserved		768MB	801.C000.0000–801.EFFF.FFFF	—
 *	Reserved		128MB	801.F000.0000–801.F7FF.FFFF	—
 *	PCI IACK/		64MB	801.F800.0000–801.FBFF.FFFF	Linear addressing.
 *	special Pchip0
 *	Pchip0 PCI I/O	32MB	801.FC00.0000–801.FDFF.FFFF	Linear addressing.
 *	Pchip0 PCI		16MB	801.FE00.0000–801.FEFF.FFFF	Linear addressing.
 *	configuration
 *	Reserved		16MB	801.FF00.0000–801.FFFF.FFFF	—
 *	Pchip1 PCI		4GB		802.0000.0000–802.FFFF.FFFF	Linear addressing.
 *	memory
 *	Reserved		2GB		803.0000.0000–803.7FFF.FFFF	—
 *	Pchip1 CSRs		256MB	803.8000.0000–803.8FFF.FFFF	addr<5:0> = 0,
 *														quadword access.
 *	Reserved		1536MB	803.9000.0000–803.EFFF.FFFF	—
 *	Reserved		128MB	803.F000.0000–803.F7FF.FFFF	—
 *	PCI IACK/		64MB	803.F800.0000–803.FBFF.FFFF	Linear addressing.
 *	special Pchip1
 *	Pchip1 PCI I/O	32MB	803.FC00.0000–803.FDFF.FFFF	Linear addressing.
 *	Pchip1 PCI		16MB	803.FE00.0000–803.FEFF.FFFF	Linear addressing.
 *	configuration
 *	Reserved		16MB	803.FF00.0000–803.FFFF.FFFF	—
 *	Reserved		8172GB	804.0000.0000–FFF.FFFF.FFFF	Bits <42:35> are don’t
 *														cares if bit <43> is
 *														asserted.
 *
 * Source Address Space Mapping definitions.  All definitions are 64-bits wide,
 * even though the address space is only 43-bits.  These are used to convert
 * the physical address supplied by the CPU into an address on the system.
 * These are specific to PIO addresses (not addresses to physical memory).
 */

/*
 * PCI Memory: 0x00000800xxxxxxxx
 */
typedef union
{
	u64		addr;
	struct
	{
		u64	idx : 3;
		u64	linear : 29;
		u64 cpuAddr : 32;
	};
} AXP_21274_LINEAR_MEMADDR;

/*
 * PCI I/O: 0x00000801fcxxxxxx and 0x00000801fdxxxxxx
 */
typedef union
{
	u64		addr;
	struct
	{
		u64	idx : 3;
		u64	linear : 22;
		u64	cpuAddr : 39;	/* always 0x801f */
	};
} AXP_21274_LINEAR_IOADDR;

/*
 * PCI Configuration: 0x00000801fexxxxxx
 */
typedef union
{
	u64		addr;
	struct
	{
		u64	idx : 3;
		u64	reg : 5;
		u64	func : 3;
		u64	dev : 5;
		u64	bus : 8;
		u64	cpuAddr : 40;
	};
}  AXP_21274_LINEAR_CFGADDR;

/*
 * CSR: 0x000008018xxxxxxx, 0x00000801axxxxxxx, 0x00000801bxxxxxxx, and
 *		0x000008038xxxxxxx
 */
typedef union
{
	u64		addr;
	struct
	{
		u64	res_1 : 6;
		u64	csr : 13;
		u64	res_2 : 9;
		u64	chip : 2;
		u64 cpuAddr : 34;
	};
} AXP_21274_CSR_ADDR;

#define AXP_21274_PCHIP0	0
#define AXP_21274_PCHIP1	1
#define AXP_21274_CCHIP		2
#define AXP_21274_DCHIP		3

/*
 * TIGbus: 0x000008010xxxxxxx, 0x000008011xxxxxxx, 0x000008012xxxxxxx, and
 * 		   0x000008013xxxxxxx
 */
typedef union
{
	u64		addr;
	struct
	{
		u64	res_1 : 6;
		u64	tigAddr : 24;
		u64 cpuAddr : 34;
	};
} AXP_21274_TIGBUS_ADDR;

/*
 * The following definitions are used to determine what is being addressed in
 * the system.
 *
 *	What			Valid Physical Addresses
 *	--------------	----------------------------------------------------------
 *	Linear Memory:	0x0800xx, 0x0802xx
 *	Linear I/O:		0x0801fc, 0x0801fd, 0x0803fc, 0x0803fd
 *	Linear Config:	0x0801fe, 0x0803fe
 *	CSR:			0x080180, 0x080182, 0x080183, 0x080380, 0x080382, 0x080383
 *	TigBus:			0x08010x, 0x08011x, 0x08012x, 0x08013x
 */
#define AXP_21274_LINEAR_MEMORY(addr)	\
		((((addr) & 0x00000fff00000000) == 0x0000080000000000) ||	\
		 (((addr) & 0x00000fff00000000) == 0x0000080200000000))
#define AXP_21274_LINEAR_IO(addr)		\
		((((addr) & 0x00000fffff000000) == 0x00000801fc000000) ||	\
		 (((addr) & 0x00000fffff000000) == 0x00000803fc000000))
#define AXP_21274_LINEAR_CFG(addr)		\
		((((addr) & 0x00000fffff000000) == 0x00000801fe000000) ||	\
		 (((addr) & 0x00000fffff000000) == 0x00000803fe000000))
#define AXP_21274_LINEAR_IACK(addr)		\
		((((addr) & 0x00000ffffc000000) == 0x00000801f8000000) ||	\
		 (((addr) & 0x00000ffffc000000) == 0x00000803f8000000))
#define AXP_21264_CSR_ADDR(addr)		\
		(((addr) & 0x00000001c0000000) == 0x0000000180000000)
#define AXP_21274_TIGBUS_ADDR(addr)		\
		((((addr) & 0x00000ffff0000000) == 0x0000080100000000) ||	\
		 (((addr) & 0x00000ffff0000000) == 0x0000080110000000) ||	\
		 (((addr) & 0x00000ffff0000000) == 0x0000080120000000) ||	\
		 (((addr) & 0x00000ffff0000000) == 0x0000080130000000))

/*
 * Target Address Space Mapping Addresses.
 */
typedef union
{
	u32		addr;
	struct
	{
		u32	idx : 3;
		u32	pciAddr : 29;
	};
} AXP_21274_PCI_MEMADDR;

typedef union
{
	u32		addr;
	struct
	{
		u32	idx : 3;
		u32	pciAddr : 22;
		u32 mbz : 7;
	};
} AXP_21274_PCI_IOADDR;

typedef union
{
	u32		addr;
	struct
	{
		u32	type : 2;	/* Type 0 = b'00' */
		u32	regLsb : 1;
		u32	reg : 5;
		u32	func : 3;
		u32 idsel : 21;
	} type0;
	struct
	{
		u32	type : 2;	/* Type 1 = b'01' */
		u32	regLsb : 1;
		u32	reg : 5;
		u32	func : 3;
		u32	dev : 5;
		u32 bus : 8;
		u32 mbz : 8;
	} type1;
} AXP_21274_PCI_CFGADDR;

#define AXP_21274_PIO_IDX	0x0000000700000000
#define AXP_21274_PIO_0		0x0000000000000000	/* Pchip0 PCI memory */
#define AXP_21274_PIO_1		0x0000000100000000
#define AXP_21274_PIO_2		0x0000000200000000	/* Pchip1 PCI memory */
#define AXP_21274_PIO_3		0x0000000300000000
#define AXP_21274_PIO_4		0x0000000400000000	/* Reserved - unused memory */

#endif /* _AXP21274_ADDRESSMAPPING_H_ */
