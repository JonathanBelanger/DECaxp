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
 *	This module contains the code to convert certain addresses where a table
 *	lookup is required.
 *
 * Revision History:
 *
 *	V01.000		29-Apr-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_21274_21264_Common.h"
#include "AXP_21274_AddressMapping.h"

/*
 * The following definitions are used for Linear (Memory and I/O) Address
 * Translations
 */
enum maskType
{
    Nada,
    Byte,
    Word,
    Long,
    Quad
};
typedef struct
{
    enum maskType type; /* xBytes, xLWs, and xQWs, x = Read or Wr */
    u8 mask;
    u8 addr_2_0;
    u8 cbe_64;
    u8 cbe_32;
    u8 regLSB;
} LINEAR_TABLE;

static LINEAR_TABLE linearTable[] =
{
/*	Type	Mask	Addr[2:0]	cbe[7:0]	cpe[3:0]	Reg LSB */
    {	Byte,	0x01, 	0x00,		0xfe, 		0x0e, 		0x00	},
    {	Byte,	0x02,	0x01,		0xfd,		0x0d,		0x00},
    {	Byte,	0x04,	0x02,		0xfb,		0x0b,		0x00},
    {	Byte,	0x08,	0x03,		0xf7,		0x07,		0x00},
    {	Byte,	0x10,	0x04,		0xef,		0x0e,		0x01},
    {	Byte,	0x20,	0x05,		0xdf,		0x0d,		0x01},
    {	Byte,	0x40,	0x06,		0xbf,		0x0b,		0x01},
    {	Byte,	0x80,	0x07,		0x7f,		0x07,		0x01},
    {	Word,	0x03,	0x00,		0xfc,		0x0c,		0x00},
    {	Word,	0x0c,	0x02,		0xf3,		0x03,		0x00},
    {	Word,	0x30,	0x04,		0xcf,		0x0c,		0x01},
    {	Word,	0xc0,	0x06,		0x3f,		0x03,		0x01},
    {	Long,	0x01,	0x00,		0x00,		0x00,		0x00},
    {	Long,	0x02,	0x04,		0x0f,		0x00,		0x01},
    {	Long,	0x04,	0x00,		0x00,		0x00,		0x00},
    {	Long,	0x08,	0x04,		0x0f,		0x00,		0x01},
    {	Long,	0x10,	0x00,		0x00,		0x00,		0x00},
    {	Long,	0x20,	0x04,		0x0f,		0x00,		0x01},
    {	Long,	0x40,	0x00,		0x00,		0x00,		0x00},
    {	Long,	0x80,	0x04,		0x0f,		0x00,		0x01},
    {	Quad,	0x00,	0x00,		0x00,		0x00,		0x00},
    {	Nada,	0x00,	0x00,		0x00,		0x00,		0x00}
};

/*
 * The following definition is used to decode a device number to an IDSEL.
 */
#define AXP_21274_DEV_TO_IDSEL(devNo)	\
	(((0x00000001) << ((devNo) & 0x1f)) & 0x001fffff)

/*
 * AXP_21274_cvtLinearAddr
 *	This function is called with a 64-bit address that will be converted to a
 *	32-bit PCI Memory address.
 *
 * Input Parameters:
 *	addr:
 *		An unsigned 64-bit address value supplied by the CPU to be converted.
 *	cmd:
 *		A value indicating the PIO command supplied by the CPU.  This is used
 *		to determine how the mask parameter needs to be interpreted.
 *	mask:
 *		An unsigned 8-bit value containing the mask supplied by the CPU.
 *	bits32:
 *		A boolean to indicate if 32-bits are being used.
 *
 * Output Parameters:
 * 	pciAddr:
 * 		A pointer to an unsigned 32-bit location to receive the converted
 * 		address.
 * 	pciByteEnable:
 * 		A pointer to an unsigned 8-bit location to receive the PCI Byte enable
 * 		information.
 */
void AXP_21274_cvtLinearAddr(u64 addr, AXP_System_Commands cmd, u8 mask,
        bool bits32, u32 *pciAddr, u8 *pciByteEnable)
{
    enum maskType type;
    AXP_21274_LINEAR_MEMADDR mem;
    AXP_21274_LINEAR_IOADDR io;
    AXP_21274_LINEAR_CFGADDR cfg;
    AXP_21274_PCI_MEMADDR *pciMem;
    AXP_21274_PCI_IOADDR *pciIO;
    AXP_21274_PCI_CFGADDR *pciCfg;
    int ii;
    bool done = false;

    /*
     * First we need to determine some things about the parameters supplied.
     */
    switch (cmd)
    {

	/*
	 * We have either Bytes or Words.  Need to determine from the mask
	 * parameter.
	 */
	case ReadBytes:
	case WrBytes:
	    if ((mask == 0x03) || (mask == 0x0c) || (mask == 0x30)
		    || (mask == 0xc0))
		type = Word;
	    else
		type = Byte;
	    break;

	case ReadLWs:
	case WrLWs:
	    type = Long;
	    break;

	    /*
	     * Assume ReadQWs or WrQWs.
	     */
	default:
	    type = Quad;
	    break;
    }

    /*
     * Determine what kind of conversion we are being requested to perform.
     */
    if (AXP_21274_LINEAR_MEMORY(addr))
    {
	pciMem = (AXP_21274_PCI_MEMADDR *) pciAddr;
	mem.addr = addr;
	pciMem->pciAddr = mem.linear;
	for (ii = 0; ((linearTable[ii].type != Nada) && (done == false)); ii++)
	{
	    if ((linearTable[ii].type == type)
		    && (linearTable[ii].mask == mask))
	    {
		pciMem->idx = linearTable[ii].addr_2_0;
		*pciByteEnable =
		        (bits32 ?
		                linearTable[ii].cbe_32 : linearTable[ii].cbe_64);
		done = true;
	    }
	}
    }
    else if (AXP_21274_LINEAR_IO(addr))
    {
	pciIO = (AXP_21274_PCI_IOADDR *) pciAddr;
	io.addr = addr;
	pciIO->mbz = 0;
	pciIO->pciAddr = io.linear;
	for (ii = 0; ((linearTable[ii].type != Nada) && (done == false)); ii++)
	{
	    if ((linearTable[ii].type == type)
		    && (linearTable[ii].mask == mask))
	    {
		pciIO->idx = linearTable[ii].addr_2_0;
		*pciByteEnable =
		        (bits32 ?
		                linearTable[ii].cbe_32 : linearTable[ii].cbe_64);
		done = true;
	    }
	}
    }
    else if (AXP_21274_LINEAR_CFG(addr))
    {
	pciCfg = (AXP_21274_PCI_CFGADDR *) pciAddr;
	cfg.addr = addr;
	if (cfg.bus == 0)
	{
	    pciCfg->type0.idsel = AXP_21274_DEV_TO_IDSEL(cfg.dev);
	    pciCfg->type0.func = cfg.func;
	    pciCfg->type0.reg = cfg.reg;
	    pciCfg->type0.type = 0;
	}
	else
	{
	    pciCfg->type1.bus = cfg.bus;
	    pciCfg->type1.dev = cfg.dev;
	    pciCfg->type1.func = cfg.func;
	    pciCfg->type1.reg = cfg.reg;
	    pciCfg->type1.type = 1;
	}
	for (ii = 0; ((linearTable[ii].type != Nada) && (done == false)); ii++)
	{
	    if ((linearTable[ii].type == type)
		    && (linearTable[ii].mask == mask))
	    {
		if (cfg.bus == 0)
		    pciCfg->type0.regLsb = linearTable[ii].regLSB;
		else
		    pciCfg->type1.regLsb = linearTable[ii].regLSB;
		*pciByteEnable =
		        (bits32 ?
		                linearTable[ii].cbe_32 : linearTable[ii].cbe_64);
		done = true;
	    }
	}
    }
    else
	; /* TODO: non-existent memory? */

    /*
     * Return the results of the conversion back to the caller.
     */
    return;
}
