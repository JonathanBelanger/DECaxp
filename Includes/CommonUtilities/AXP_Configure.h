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
 *
 *	V01.003		03-Feb-2018	Jonathan D. Belanger
 *	Continued to work on reading in the configuration file and loading it into
 *	a usable format.
 */
#ifndef _AXP_CONFIGURE_DEFS_
#define _AXP_CONFIGURE_DEFS_

#include "CommonUtilities/AXP_Utility.h"

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
 *				CboxCSRFile			file-specification
 *			CPUS
 *				Count				number
 *				Generation			number
 *				Pass				number
 *				Name				string
 *			DARRAY
 *				Size				decimal
 *				Count				decimal
 *			Disks
 *				*Disk (number)
 *					Type			Disk, CDROM, RWCDROM
 *					Name			string
 *					Size			decimal(MB, GB)
 *					File			file-specification
 *			Console
 *				Port				number
 *			Networks
 *				*Network (number)
 *					Name			string
 *					MAC				##-##-##-##-##-##
 *			Printers
 *				*Printer (number)
 *				<TBD>				ignored
 *			Tapes
 *				*Tape (number)
 *				<TBD>				ignored
 */
typedef enum
{
    NoNodes,
    DECaxp,
    Owner,
    SystemConf
} AXP_21264_CONFIG_NODES;

typedef enum
{
    NoOwner,
    Name,
    CreationDate,
    ModifyDate
} AXP_21264_CONFIG_OWNER;

typedef enum
{
    NoName,
    FirstName,
    MI,
    LastName,
    NameSuffix
} AXP_21264_CONFIG_NAME;

typedef enum
{
    NoSystem,
    Model,
    SROM,
    CPUS,
    DARRAYS,
    Disks,
    Console,
    Networks,
    Printers,
    Tapes
} AXP_21264_CONFIG_SYSTEM;

typedef enum
{
    NoModel,
    ModelName,
    ModelModel
} AXP_21264_CONFIG_MODEL;

typedef enum
{
    NoSROM,
    InitFile,
    PALImage,
    ROMImage,
    NVRamFile,
    CboxCSRs
} AXP_21264_CONFIG_SROM;

typedef enum
{
    NoCPUs,
    CPUCount,
    Generation,
    MfgPass
} AXP_21264_CONFIG_CPUS;

typedef enum
{
    NoDARRAYs,
    DARRAYSize,
    DARRAYCount
} AXP_21264_CONFIG_DARRAYS;

typedef enum
{
    NoDisks,
    DECDisk
} AXP_21264_CONFIG_DISKS;

typedef enum
{
    NoDisk,
    DiskType,
    DiskName,
    DiskSize,
    DiskFile
} AXP_21264_CONFIG_DISK;

typedef enum
{
    NoConsole,
    Port
} AXP_21264_CONFIG_CONSOLE;

typedef enum
{
    NoNetworks,
    TopNetworks
} AXP_21264_CONFIG_NETWORKS;

typedef enum
{
    NoNetwork,
    NetworkName,
    NetworkMAC
} AXP_21264_CONFIG_NETWORK;

typedef enum
{
    NoPrinters,
    TopPrinters
} AXP_21264_CONFIG_PRINTERS;

typedef enum
{
    NoTapes,
    TopTapes
} AXP_21264_CONFIG_TAPES;

/*
 * There can only be one Owner record.  It contains the owner's name and the
 * creation and modify date information for the file itself.
 *
 *		Owner
 *			Name
 *				First				string
 *				MI					string
 *				Last				string
 *				Suffix				string
 *			Creation Date			DD-MMM-YYYY
 *			Modify Date				DD-MMM-YYYY
 */
typedef struct
{
    char *first;
    char *mi;
    char *last;
    char *suffix;
    struct tm create;
    struct tm modify;
} AXP_21264_OWNER_INFO;

/*
 * There can be only one model in a system.  The model information describes
 * the system in human readable format.
 *
 *		System
 *			Model
 *				Name				string
 *				Model				string
 */
typedef struct
{
    char *name;
    char *model;
} AXP_21264_MODEL_INFO;

/*
 * There can only be one SROM in a system.  This contains the locations of the
 * files where things like the initialize file containing the code that
 * generates the PAL image, the PAL image read in after the ROM image has
 * initialized the CPU, and the Non-volatile RAM, where things like timers and
 * the like are store between reboots.
 *
 *		System
 *			SROM
 *				InitFile			file-specification
 *				PALImage			file-specification
 *				ROMImage			file-specification
 *				NVRamFile			file-specification
 *				CboxCSRFile			file-specification
 */
typedef struct
{
    char *initFile;
    char *PALImage;
    char *ROMImage;
    char *NVRamFile;
    char *CboxCSRFile;
} AXP_21264_SROM_INFO;

/*
 *		System
 *			CPUS
 *				Count				number
 *				Generation			enum
 *				Pass				number
 */
#define EV56					7
#define EV6						8
#define EV67					11
#define EV68A					12
#define EV68CB					12
#define EV68DC					12
#define EV68CX					14
#define EV7						15
#define EV79					16
#define EV69A					17

#define	AXP_PASS_2_21_EV4		0	/* EV4 */
#define AXP_PASS_3_EV4			1
#define AXP_RESERVED			0	/* LCA Family */
#define AXP_PASS_1_11_66		1
#define AXP_PASS_2_66			2
#define AXP_PASS_1_11_68		3
#define AXP_PASS_2_68			4
#define AXP_PASS_1_66A			5
#define AXP_PASS_1_68A			6
#define AXP_PASS_2_22			1	/* EV5 */
#define AXP_PASS_23_EV5			2
#define AXP_PASS_3_EV5			3
#define AXP_PASS_32				4
#define AXP_PASS_4_EV5			5
#define AXP_PASS_1				1	/* EV45 */
#define AXP_PASS_11				2
#define AXP_PASS_1_11			6
#define AXP_PASS_2_EV45			3
#define AXP_PASS_2_EV56			2
#define	AXP_PASS_2_21			2	/* EV6 */
#define AXP_PASS_22_EV6			3
#define AXP_PASS_23_EV6			4
#define AXP_PASS_3_EV6			5
#define AXP_PASS_24_EV6			6
#define AXP_PASS_25_EV6			7
#define AXP_PASS_21				2	/* EV67 */
#define AXP_PASS_211			4
#define AXP_PASS_221			5
#define AXP_PASS_23_24			6
#define AXP_PASS_212			7
#define AXP_PASS_222			8
#define AXP_PASS_223_225		9
#define AXP_PASS_224			10
#define AXP_PASS_25_EV67		11
#define AXP_PASS_241			12
#define AXP_PASS_251			13
#define AXP_PASS_26				14
#define AXP_PASS_22_23			3	/* EV68CB */
#define AXP_PASS_3_31			4
#define AXP_PASS_24				5
#define AXP_PASS_4				6
#define AXP_PASS_2_EV68DC		2	/* EV68DC */
#define AXP_PASS_231			3
#define AXP_PASS_214_EV68DC		4
#define AXP_PASS_2_EV68A		2	/* EV68A */
#define AXP_PASS_21_21A_3		3
#define AXP_PASS_22_EV68A		4

typedef struct
{
    char *name;
    char *genStr;
    u32 majorType;
    u32 year;
    u64 dCacheSize;
    u64 iCacheSize;
    u64 sCacheSize;
    u64 bCacheSizeLow;
    u64 bCacheSizeHigh;
    struct
    {
	u32 ieeeRndInf :1; /* h/w support for rounding to +/-Inf */
	u32 bwx :1; /* Byte/Word Extensions */
	u32 mvi :1; /* Multimedia Extensions */
	u32 fix :1; /* Integer to/from FP move and SQRT */
	u32 cix :1; /* Counting and Finding Bits extension */
	u32 pfmi :1; /* Prefetch with modify intent support */
	u32 res :26;
    } isa;
} AXP_CPU_CONFIG;

typedef struct
{
    AXP_CPU_CONFIG *config;
    u32 minorType;
    u32 count;
} AXP_21264_CPU_INFO;

/*
 * There can be one to four Dynamic Memory Arrays (DARRAYs).  Each DARRAY can
 * be either 4x64MB (256MB), 4x128MB (512MB), 4x256MB (1.0GB), 4x512MB (2.0GB),
 * for up to 8GB of memory.
 *
 *		System
 *			DARRAY
 *				Size			decimal(MB,GB)
 *				Count			decimal	[1,2,3,4]
 */
typedef struct
{
    u64 size;
    u32 count;
} AXP_21264_DARRAY_INFO;

/*
 * There can be one or more disk drives.  There has to be at least one for the
 * operating system.  There are up to 3 kinds of disk drive types supported.
 * They are Disk (Read/Write Disk Drive), CDROM (Compact Disk Read-Only
 * Memory), and RWCDROM (Read/Write CDROM).  There may be others added in the
 * future.  For each disk the following information is required.  Type, Name,
 * Size (in megabytes (MB) or gigabytes (GB)), and file-specification where the
 * emulated disk is stored on the host operating system.
 *
 *		System
 *			Disks
 *				*Disk (number)
 *					Type			Disk, CDROM, RWCDROM
 *					Name			string
 *					Size			decimal(MB, GB)
 *					File			file-specification
 */
typedef enum
{
    Diskless,
    Disk,
    CD_ROM,
    RW_CDROM
} AXP_21264_DISK_TYPES;
typedef struct
{
    char *name;
    char *fileSpec;
    u32 unit;
    u64 size;
    AXP_21264_DISK_TYPES type;
} AXP_21264_DISK_INFO;

/*
 * The ES40 has the potential for 2 consoles.  Not exactly sure why.  For now,
 * we are going to only have a single console.  This console will be
 * implemented using a telnet session.  For now, we'll just have the port
 * number.  We may also need to know which network.
 *
 * TODO: Make sure this comment is accurate once fully implemented.
 *
 *		System
 *			Console
 *				Port				number
 */
typedef struct
{
    u32 port;
} AXP_21264_CONSOLE_INFO;

/*
 * There can one or more network controllers defined.  These devices are
 * configured under the System.
 *
 *		System
 *			Networks
 *				Network (number)
 *					Name			string
 *					MAC				##-##-##-##-##-##
 *
 * TODO: Do we also need a device type?
 */
typedef struct
{
    char *name;
    char *mac; /* TODO: This should probably be defined differently */
    u32 unit;
} AXP_21264_NETWORK_INFO;

/*
 * Currently, there is not support for printers or tapes.  When implemented,
 * there can be zero or more of these devices.  These devices are configured
 * under the System.  For now, these are just stubbed out.
 *
 *		System
 *			Printers
 *				*Printer (number)
 *				<TBD>				ignored
 *			Tapes
 *				*Tape (number)
 *				<TBD>				ignored
 */
typedef struct
{
    u32 unit;
} AXP_21264_PRINTER_INFO;

typedef struct
{
    u32 unit;
} AXP_21264_TAPE_INFO;

/*
 * This is the structure to hold the configuration information that has been
 * parsed from the configuration file.
 */
typedef struct
{
    AXP_21264_OWNER_INFO owner;
    struct
    {
	AXP_21264_DISK_INFO *disks;
	AXP_21264_NETWORK_INFO *networks;
	AXP_21264_MODEL_INFO model;
	AXP_21264_SROM_INFO srom;
	AXP_21264_CPU_INFO cpus;
	AXP_21264_DARRAY_INFO darrays;
	AXP_21264_CONSOLE_INFO console;
	u32 diskCount;
	u32 networkCount;
    } system;
} AXP_21264_CONFIG;

/*
 * Exported Global Variables
 */
extern AXP_21264_CONFIG AXP_21264_Config;
extern AXP_CPU_CONFIG AXP_CPU_Configurations[];

/*
 * Function Prototypes
 */
int AXP_LoadConfig_File(char *);
bool AXP_ConfigGet_CPUType(u32 *, u32 *);
u32 AXP_ConfigGet_CPUCount(void);
bool AXP_ConfigGet_InitFile(char *);
bool AXP_ConfigGet_PALFile(char *);
bool AXP_ConfigGet_ROMFile(char *);
bool AXP_ConfigGet_NVRAMFile(char *);
bool AXP_ConfigGet_CboxCSRFile(char *);
void AXP_ConfigGet_DarrayInfo(u32 *, u64 *);
void AXP_TraceConfig(void);

#endif /* _AXP_CONFIGURE_DEFS_ */

