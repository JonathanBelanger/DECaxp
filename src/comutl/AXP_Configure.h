/*
 * Copyright (C) Jonathan D. Belanger 2017-2018.
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
 *	This header file contains useful definitions for compiling various portions
 *	of the code in or out.  Particularly unit testing code.  This header file
 *	is only included in .c files and should not be included in an include file.
 *
 * Revision History:
 *
 *	V01.000		14-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		22-May-2017	Jonathan D. Belanger
 *	Restructured the code a bit, so the definition that was in here is no
 *	longer needed.
 *
 *	V01.002		29-Jan-2018	Jonathan D. Belanger
 *	Started coding for reading, parsing and making the configuration
 *	information for the Digital Alpha AXP 21264 CPU Emulator code.
 */
#ifndef _AXP_CONFIGURE_DEFS_
#define _AXP_CONFIGURE_DEFS_
/*
 * The name space for the emulator is as follows:
 *
 *	DECaxp
 *		Owner
 *			Name
 *				First				string
 *				MI					string
 *				Last				string
 *				Suffix				string
 *			Creation Date			DD-MMM-YYYY
 *			Modify Date				DD-MMM-YYYY
 *		System
 *			Model
 *				Name				string
 *				Model				string
 *			SROM
 *				InitFile			file-specification
 *				PALImage			file-specification
 *				ROMImage			file-specification
 *				NVRamFile			file-specification
 *			CPUS
 *				*CPU (number)
 *					Generation		number
 *					Pass			number
 *					Name			string
 *			DIMMS
 *				*DIMM (number)
 *					Size			decimal(MB, GB)
 *			Disks
 *				*Disk (number)
 *					Type			Disk, CDROM, RWCDROM
 *					Name			string
 *					Size			decimal(MB, GB)
 *					File			file-specification
 *			Console
 *				Port				number
 *			Network
 *				Name				string
 *				MAC					##-##-##-##-##-##
 *			Printers
 *				*Printer (number)
 *				<TBD>				ignored
 *			Tapes
 *				*Table (number)
 *				<TBD>				ignored
 */
typedef enum
{
	Owner,
	System
} AXP_21264_CONFIG_NODES;

typedef enum
{
	Name,
	CreationDate,
	ModifyDate
} AXP_21264_CONFIG_OWNER;

typedef enum
{
	Model,
	SROM,
	CPUS,
	DIMMS,
	Disks,
	Console,
	Network,
	Printers,
	Tapes
} AXP_21264_CONFIG_SYSTEM;

typedef struct
{
	char *first;
	char *mi;
	char *last;
	char *suffix;
	struct tm create;
	struct tm modify;
} AXP_21264_OWNER_INFO;

/* TODO:	Continue to work on these */

/*
 * Function Prototypes
 */
int AXP_LoadConfig_File(char *);

#endif /* _AXP_CONFIGURE_DEFS_ */

