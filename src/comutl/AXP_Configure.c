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
 *	This source file contains the code to read in a configuration file, parse
 *	it, and return requested information from it.
 *
 * Revision History:
 *
 *	V01.000		28-Jan-2018	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		02-Mar-2018	Jonathan D. Belanger
 *	Added functions to return values from the configuration structure.  Also
 *	made the configuration structure static.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

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
 *				Count				number
 *				Generation			string
 *				Pass				number
 *			DIMMS
 *				Count				number
 *				Size				decimal(MB, GB)
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

/*
 * Global variable used throughout the emulator.
 */
static AXP_21264_CONFIG	_axp_21264_config_=
{
	.owner.first = NULL,
	.owner.mi = NULL,
	.owner.last = NULL,
	.system.disks = NULL,
	.system.diskCount = 0,
	.system.networks = NULL,
	.system.networkCount = 0,
	.system.model.name = NULL,
	.system.model.model = NULL,
	.system.srom.initFile = NULL,
	.system.srom.PALImage = NULL,
	.system.srom.ROMImage = NULL,
	.system.srom.NVRamFile = NULL,
	.system.srom.CboxCSRFile = NULL,
	.system.cpus.config = NULL,
	.system.cpus.count = 0,
	.system.cpus.minorType = 0,
	.system.dimms.size = 0,
	.system.dimms.count = 0
};

/*
 * CPU Count
 *	This value is used to identify each CPU as it is created.  This is where
 *	the value for the WHAMI IPR is determined.  This CPU-ID has a mutex to make
 *	sure no more than one CPU gets any particular ID.
 */
static u64 _axp_cpu_id_counter_;
static pthread_mutex_t _axp_config_mutex_;

/*
 * Module local variable used in parsing the XML configuration file.
 */
struct AXP_TopLevel
{
	char					*token;
	AXP_21264_CONFIG_NODES	node;
};
struct AXP_Owner
{
	char					*token;
	AXP_21264_CONFIG_OWNER	node;
};
struct AXP_Name
{
	char					*token;
	AXP_21264_CONFIG_NAME	node;
};
struct AXP_System
{
	char					*token;
	AXP_21264_CONFIG_SYSTEM	node;
};
struct AXP_Model
{
	char					*token;
	AXP_21264_CONFIG_MODEL	node;
};
struct AXP_SROM
{
	char					*token;
	AXP_21264_CONFIG_SROM	node;
};
struct AXP_CPUS
{
	char					*token;
	AXP_21264_CONFIG_CPUS	node;
};
struct AXP_DIMMS
{
	char					*token;
	AXP_21264_CONFIG_DIMMS	node;
};
struct AXP_Disks
{
	char					*token;
	AXP_21264_CONFIG_DISKS	node;
};
struct AXP_Disk
{
	char					*token;
	AXP_21264_CONFIG_DISK	node;
};
struct AXP_Console
{
	char					*token;
	AXP_21264_CONFIG_CONSOLE	node;
};
struct AXP_Networks
{
	char					*token;
	AXP_21264_CONFIG_NETWORKS node;
};
struct AXP_Network
{
	char					*token;
	AXP_21264_CONFIG_NETWORK node;
};
struct AXP_Printers
{
	char					*token;
	AXP_21264_CONFIG_PRINTERS node;
};
struct AXP_Tapes
{
	char					*token;
	AXP_21264_CONFIG_TAPES	node;
};

static struct AXP_TopLevel _top_level_nodes[] =
{
	{"DECaxp", DECaxp},
	{"Owner", Owner},
	{"System", SystemConf},
	{NULL, NoNodes}
};
static struct AXP_Owner _owner_level_nodes[] =
{
	{"Name", Name},
	{"CreationDate", CreationDate},
	{"ModifyDate", ModifyDate},
	{NULL, NoOwner}
};
static struct AXP_Name _name_level_nodes[] =
{
	{"First", FirstName},
	{"MI", MI},
	{"Last", LastName},
	{"Suffix", NameSuffix},
	{NULL, NoName}
};
static struct AXP_System _system_level_nodes[] =
{
	{"Model", Model},
	{"SROM", SROM},
	{"CPUs", CPUS},
	{"DIMMs", DIMMS},
	{"Disks", Disks},
	{"Console", Console},
	{"Networks", Networks},
	{"Printers", Printers},
	{"Tapes", Tapes},
	{NULL, NoSystem}
};
static struct AXP_Model _model_level_nodes[] =
{
	{"Name", ModelName},
	{"Model", ModelModel},
	{NULL, NoModel}
};
static struct AXP_SROM _srom_level_nodes[] =
{
	{"InitFile", InitFile},
	{"PALImage", PALImage},
	{"ROMImage", ROMImage},
	{"NVRamFile", NVRamFile},
	{"CboxCSRFile", CboxCSRs},
	{NULL, NoSROM}
};
static struct AXP_CPUS _cpu_level_nodes[] =
{
	{"Count", CPUCount},
	{"Generation", Generation},
	{"Pass", MfgPass},
	{NULL, NoCPUs}
};
static struct AXP_DIMMS _dimm_level_nodes[] =
{
	{"Count", DIMMCount},
	{"Size", DIMMSize},
	{NULL, NoDIMMs}
};
static struct AXP_Disks _disks_level_nodes[] =
{
	{"Disk", DECDisk},
	{NULL, NoDisk}
};
static struct AXP_Disk _disk_level_nodes[] =
{
	{"Type", DiskType},
	{"Name", DiskName},
	{"Size", DiskSize},
	{"File", DiskFile},
	{NULL, NoDisk}
};
static struct AXP_Console _console_level_nodes[] =
{
	{"Port", Port},
	{NULL, NoConsole}
};
static struct AXP_Networks _networks_level_nodes[] =
{
	{"Network", TopNetworks},
	{NULL, NoNetworks}
};
static struct AXP_Network _network_level_nodes[] =
{
	{"Name", NetworkName},
	{"MAC", NetworkMAC},
	{NULL, NoNetwork}
};
static struct AXP_Printers _printers_level_nodes[] =
{
	{"Printers", TopPrinters},
	{NULL, NoPrinters}
};
static struct AXP_Tapes _tapes_level_nodes[] =
{
	{"Tapes", TopTapes},
	{NULL, NoTapes}
};

static char *months[] =
{
	"",
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC",
	NULL
};

AXP_CPU_CONFIG AXP_CPU_Configurations[] =
{
	{
		.genStr = "Simulation",
		.name = "Simulation",
		.majorType = 3,
		.year = 1990,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 128*ONE_K,
		.bCacheSizeHigh = 16*ONE_M,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV3",
		.name = "Prism",
		.majorType = 1,
		.year = 1991,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 128*ONE_K,
		.bCacheSizeHigh = 16*ONE_M,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV4",
		.name = "21064",
		.majorType = 2,
		.year = 1992,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 128*ONE_K,
		.bCacheSizeHigh = 16*ONE_M,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "LCA4A",
		.name = "21066",
		.majorType = 4,
		.year = 1993,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV4S",
		.name = "21064",
		.majorType = 2,
		.year = 1993,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 128*ONE_K,
		.bCacheSizeHigh = 16*ONE_M,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "LCA45A",
		.name = "21066A",
		.majorType = 4,
		.year = 1994,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "LCA45B",
		.name = "21068A",
		.majorType = 4,
		.year = 1994,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "LCA4B",
		.name = "21068",
		.majorType = 4,
		.year = 1994,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV45",
		.name = "21064A",
		.majorType = 6,
		.year = 1994,
		.dCacheSize = 16*ONE_K,
		.iCacheSize = 16*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 256*ONE_K,
		.bCacheSizeHigh = 16&ONE_M,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV5",
		.name = "21164",
		.majorType = 5,
		.year = 1995,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 96*ONE_K,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 64*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV56",
		.name = "21164A",
		.majorType = 7,
		.year = 1996,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 8*ONE_K,
		.sCacheSize = 96,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 64*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "PCA56",
		.name = "21164PC",
		.majorType = 9,
		.year = 1997,
		.dCacheSize = 8*ONE_K,
		.iCacheSize = 16*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 512*ONE_K,
		.bCacheSizeHigh = 4*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "PCA57",
		.name = "21164PC",
		.majorType = 10,
		.year = 1998,
		.dCacheSize = 16*ONE_K,
		.iCacheSize = 32*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 512*ONE_K,
		.bCacheSizeHigh=4*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV6",
		.name = "21264",
		.majorType = 8,
		.year = 1998,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV67",
		.name = "21264A",
		.majorType = 11,
		.year = 1999,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 0,
		.isa.res = 0
	},
	{
		.genStr = "EV68A",
		.name = "21264B - Samsung",
		.majorType = 12,
		.year = 2001,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV68C",
		.name = "21264B - IBM",
		.majorType = 12,
		.year = 2001,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV68CB",
		.name = "21264C",
		.majorType = 12,
		.year = 2001,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV68DC",
		.name = "21264C",
		.majorType = 12,
		.year = 2001,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV68CD",
		.name = "21264",
		.majorType = 12,
		.year = 2001,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV68AL",
		.name = "21264B",
		.majorType = 13,
		.year = 2001,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV68CX",
		.name = "21264D",
		.majorType = 14,
		.year = 2002,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV68E",
		.name = "21264E",
		.majorType = 14,
		.year = 2002,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV69A",
		.name = "21264",
		.majorType = 17,
		.year = 2002,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 0,
		.bCacheSizeLow = 2*ONE_M,
		.bCacheSizeHigh = 8*ONE_M,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV7",
		.name = "21364",
		.majorType = 15,
		.year = 2003,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 1.75*ONE_M,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV7z",
		.name = "21364",
		.majorType = 15,
		.year = 2004,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 1.75*ONE_M,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV78",
		.name = "21364A",
		.majorType = 15,
		.year = 2004,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 1.75*ONE_M,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV79",
		.name = "21364A",
		.majorType = 16,
		.year = 2004,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 1.75*ONE_M,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = "EV8",
		.name = "21464",
		.majorType = 0,
		.year = 2003,
		.dCacheSize = 64*ONE_K,
		.iCacheSize = 64*ONE_K,
		.sCacheSize = 3*ONE_M,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 1,
		.isa.bwx = 1,
		.isa.mvi = 1,
		.isa.fix = 1,
		.isa.cix = 1,
		.isa.pfmi = 1,
		.isa.res = 0
	},
	{
		.genStr = NULL,
		.name = NULL,
		.majorType = 0,
		.year = 0,
		.dCacheSize = 0,
		.iCacheSize = 0,
		.sCacheSize = 0,
		.bCacheSizeLow = 0,
		.bCacheSizeHigh = 0,
		.isa.ieeeRndInf = 0,
		.isa.bwx = 0,
		.isa.mvi = 0,
		.isa.fix = 0,
		.isa.cix = 0,
		.isa.pfmi = 0,
		.isa.res = 0
	}
};

/*
 * AXP_stripXmlString
 *	This function is called to remove leading and trailing characters from an
 *	xmlChar string.  These are unprintable or control characters and are not
 *	needed.  The updated string is modified in place.
 *
 * Input Parameters:
 *	value:
 *		A pointer to an xmlChar string that needs to be stripped of leading and
 *		trailing information.
 *
 * Output Parameters:
 *	value:
 *		A pointer to the same string, but with the string within updated in
 *		place.
 *
 * Return Values:
 *	None.
 */
static void AXP_stripXmlString(xmlChar *value)
{
	xmlChar *start, *begin, *end;
	bool	done = false;
	int		len;

	/*
	 * First get set up some local variables to assist in stripping the xmlChar
	 * string to one containing just printable characters.
	 */
	len = xmlStrlen(value) - 1;
	begin = start = value;
	end = value + len;

	/*
	 * Set the start pointer to the first character, up until the last
	 * character, that is printable.  If none is found, then the start pointer
	 * equals the end pointer.
	 */
	while ((start <= end) && (done == false))
	{
		if (isspace(*start))
			start++;
		else
			done = true;
	}

	/*
	 * Set the end pointer to the last character, up until the current address
	 * of the first printable character, that is also a printable character.
	 */
	done = false;
	while ((end >= start) && (done == false))
	{
		if (isspace(*end))
			end--;
		else
			done = true;
	}

	/*
	 * If the there are characters at the beginning or end of the original
	 * xmlChar string, and the address of the first printable character is less
	 * than or equal to the last printable character, then we have something to
	 * strip out of the string.
	 */
	if (((start != begin) || (end != (value + len))) && (start <= end))
	{
		while (start <= end)
		{
			*begin = *start;
			begin++;
			start++;
		}
		while (begin <= end)
		{
			*begin = '\0';
			begin++;
		}
	}

	/*
	 * If the start address is greater than the end address, then we have a
	 * zero length string.  Otherwise, there is nothing that needs to be
	 * stripped.
	 */
	else if (start > end)
		*value = '\0';

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_cvtSizeStr
 *	This function is called to convert a string containing the size for various
 *	components of an emulated system.  The string is encoded as the following:
 *
 *		<#>[.<#>][B|KB|MB|GB]
 *
 *	where:
 *		<#>		is an integer greater than or equal to 0.
 *		[.<#>]	is an integer representing the fraction portion.  This is
 *				is optional and assumed to be .0.
 *		B		indicates that the number is a count of bytes.  This and the
 *				following are also optional and is assumed to be B.
 *		KB		indicates to multiply the number by 1024.
 *		MB		indicates to multiply the number by 1024*124.
 *		GB		indicates to multiply the number by 1024*1024*1024.
 *
 *	Though the encoding is a real, the return value is a 32-bit unsigned
 *	integer,as we do not handle fractions of a byte.
 *
 * Input Parameters:
 *	value:
 *		A pointer to the value to be converted.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	An unsigned 32-bit value for the string to be converted.
 */
u64	AXP_cvtSizeStr(char *value)
{
	double	cvtValue;
	double	multiplier = 1.0;
	int		len;
	u64		retVal = 0;

	/*
	 * If a pointer to the string was supplied on the call and there is
	 * something in the string to potentially covert,, then we have something
	 * to do.  Otherwise, we'll just return a value of zero back to the caller.
	 */
	if ((value != NULL) && ((len = strlen(value)) > 0))
	{

		/*
		 * If there is more than one character and the last character is the
		 * letter 'B', then we could have [K|M|G]B or just B.  If the length of
		 * the string is 1, then we either have just a number of just the
		 * letter 'B'.  Convert the former, and ignore the latter.
		 */
		if ((len > 1) && (value[len - 1] == 'B'))
		{
			switch (value[len - 2])
			{
				case 'K':
					multiplier = 1024.0;
					value[len - 2] = '\0';
					break;

				case 'M':
					multiplier = 1024.0 * 1024.0;
					value[len - 2] = '\0';
					break;

				case 'G':
					multiplier = 1024.0 * 1024.0 * 1024.0;
					value[len - 2] = '\0';
					break;

				default:
					value[len - 1] = '\0';
					break;
			}
		}

		/*
		 * Convert the number part of the string to a floating point value and
		 * multiply in the multiplier for the real value.
		 */
		cvtValue = atof(value) * multiplier;

		/*
		 * Finally, convert the floating point value to an unsigned 32-bit
		 * value.
		 */
		retVal = cvtValue;
	}

	/*
	 * Return the result of the conversion back to the caller.
	 */
	return(retVal);
}

/*
 * parse_tapes_names
 *	This function parses the elements within the Tapes Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Tapes node are as follows:
 *		<Tapes>
 *			<Tape number="1" \>
 *		</Tapes>
 *	NOTE:	This node is not fully defined and will be determined, if and when
 *			a tape device is supported.
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *	unit:
 *		A pointer to an unsigned 32-bit integer to receive the unit number for
 *		the device.
 *
 * Return Values:
 *	None.
 */
static void parse_tapes_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_TAPES parent,
					char *value,
					u32 *unit)
{
	xmlNode	*cur_node = NULL;
	char	*ptr;
	char	nodeValue[80];
	u32		nodeUnit;
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value or unit of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length and the local unit is 0.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';
	if (unit == NULL)
		nodeUnit = 0;

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		xmlAttr *attr = cur_node->properties;

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_tapes_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _tapes_level_nodes[ii].token) == 0)
				{
					parent = _tapes_level_nodes[ii].node;
					found = true;
				}
			}

			/*
			 * This element has a unit number as part of its definition.  Get
			 * it and convert it.
			 */
			while (attr != NULL)
			{
				if (strcmp((char *) attr->name, "number") == 0)
				{
					xmlChar *attrVal = xmlNodeListGetString(doc, attr->children, 1);

					AXP_stripXmlString(attrVal);
					if (xmlStrlen(attrVal) > 0)
					{
						nodeUnit = strtoul((char *) attrVal, &ptr, 10);
						if (unit != NULL)
							*unit = nodeUnit;
					}
					xmlFree(attrVal);
				}
				attr = attr->next;
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (value != NULL)
				strcpy(value, (char *) key);
			parent = NoTapes;
			xmlFree(key);
		}
		if (parent != NoTapes)
		{
			parse_tapes_names(
						doc,
						cur_node->children,
						parent,
						nodeValue,
						&nodeUnit);
			parent = NoTapes;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_printers_names
 *	This function parses the elements within the Printers Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Tapes node are as follows:
 *		<Printers>
 *			<Printer number="1" \>
 *		</Printers>
 *	NOTE:	This node is not fully defined and will be determined, if and when
 *			a tape device is supported.
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *	unit:
 *		A pointer to an unsigned 32-bit integer to receive the unit number for
 *		the device.
 *
 * Return Values:
 *	None.
 */
static void parse_printers_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_PRINTERS parent,
					char *value,
					u32 *unit)
{
	xmlNode	*cur_node = NULL;
	char	*ptr;
	char	nodeValue[80];
	u32		nodeUnit;
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value or unit of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length and the local unit is 0.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';
	if (unit == NULL)
		nodeUnit = 0;

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		xmlAttr *attr = cur_node->properties;

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_printers_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _printers_level_nodes[ii].token) == 0)
				{
					parent = _printers_level_nodes[ii].node;
					found = true;
				}
			}

			/*
			 * This element has a unit number as part of its definition.  Get
			 * it and convert it.
			 */
			while (attr != NULL)
			{
				if (strcmp((char *) attr->name, "number") == 0)
				{
					xmlChar *attrVal = xmlNodeListGetString(doc, attr->children, 1);

					AXP_stripXmlString(attrVal);
					if (xmlStrlen(attrVal) > 0)
					{
						nodeUnit = strtoul((char *) attrVal, &ptr, 10);
						if (unit != NULL)
							*unit = nodeUnit;
					}
					xmlFree(attrVal);
				}
				attr = attr->next;
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (value != NULL)
				strcpy(value, (char *) key);
			parent = NoTapes;
			xmlFree(key);
		}
		if (parent != NoPrinters)
		{
			parse_printers_names(
						doc,
						cur_node->children,
						parent,
						nodeValue,
						&nodeUnit);
			parent = NoPrinters;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_network_names
 *	This function parses the elements within the Network Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Network node are as follows:
 *		<Network number="1">
 *			<Name>es40</Name>
 *			<MAC>08-00-20-01-12-45</MAC>
 *		</Network>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *	unit:
 *		A value indicating the unit number associated with the network config
 *		being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_network_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_NETWORK parent,
					u32 unit,
					char *value)
{
	xmlNode	*cur_node = NULL;
	xmlChar	*key = NULL;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_network_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _network_level_nodes[ii].token) == 0)
				{
					parent = _network_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if ((xmlStrlen(key) > 0) && (value != NULL))
				strcpy(value, (char *) key);
			xmlFree(key);
		}

		/*
		 * If the parent is NoNetwork, then we have yet to find anything to
		 * parse out, or we just parsed an element and need to move onto the
		 * next element.
		 */
		if (parent != NoNetwork)
		{

			/*
			 * Utilizing the unit value supplied on the call, find out where we
			 * are to store the information for this network device.
			 */
			ii = 0;
			while ((ii < _axp_21264_config_.system.networkCount) &&
				   (_axp_21264_config_.system.networks[ii].unit != unit))
				ii++;

			/*
			 * All ourselves to parse out the text node for the value of the
			 * configuration item we are parsing.
			 */
			parse_network_names(
					doc,
					cur_node->children,
					parent,
					unit,
					nodeValue);

			/*
			 * Depending upon the element being parsed, initialize the
			 * configuration with the information just parsed out.
			 */
			switch (parent)
			{
				case NetworkName:
					_axp_21264_config_.system.networks[ii].name = realloc(
									_axp_21264_config_.system.networks[ii].name,
									(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.networks[ii].name, nodeValue);
					break;

				case NetworkMAC:
					_axp_21264_config_.system.networks[ii].mac = realloc(
									_axp_21264_config_.system.networks[ii].mac,
									(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.networks[ii].mac, nodeValue);
					break;

				case NoNetwork:
				default:
					break;
			}
			parent = NoNetwork;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_networks_names
 *	This function parses the elements within the Networks Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Networks node are as follows:
 *		<Networks>
 *			<Network number="1">...</Network>
 *		</Networks>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
static void parse_networks_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_NETWORKS parent)
{
	xmlNode	*cur_node = NULL;
	char	*ptr;
	u32		nodeUnit = 0;
	int		ii;
	bool	found;

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			xmlAttr *attr = cur_node->properties;

			found = false;
			for (ii = 0;
				 ((_networks_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _networks_level_nodes[ii].token) == 0)
				{
					parent = _networks_level_nodes[ii].node;
					found = true;
				}
			}

			/*
			 * The Network element has a single attribute called number.  Parse
			 * this out and convert it to a 32-bit integer.  We'll store it in
			 * the allocated configuration item and supply it on the code that
			 * parses out the child configuration data,
			 */
			while (attr != NULL)
			{
				if (strcmp((char *) attr->name, "number") == 0)
				{
					xmlChar *attrVal = xmlNodeListGetString(doc, attr->children, 1);

					AXP_stripXmlString(attrVal);
					if (xmlStrlen(attrVal) > 0)
						nodeUnit = strtoul((char *) attrVal, &ptr, 10);
					xmlFree(attrVal);
				}
				attr = attr->next;
			}
		}

		/*
		 * If we are parsing a Network element, increment the number of
		 * networks being configured, allocate a new entry in the array of
		 * networks, set the unit number for this new entry and call the
		 * parsing code to initialize the remaining fields.
		 */
		if (parent == TopNetworks)
		{

			/*
			 * We have a new network device to be configured.  Increment the
			 * counter of these items.
			 */
			_axp_21264_config_.system.networkCount++;

			/*
			 * Allocate an array large enough to handle an additional entry.
			 */
			_axp_21264_config_.system.networks = realloc(
						_axp_21264_config_.system.networks,
						(_axp_21264_config_.system.networkCount *
							sizeof(AXP_21264_NETWORK_INFO)));

			/*
			 * Set the unit number for this new entry (which should always be
			 * the last one.  Also initialize the other fields so we don't get
			 * a segmentation fault in the function are are about to call;
			 */
			ii = _axp_21264_config_.system.networkCount - 1;
			_axp_21264_config_.system.networks[ii].unit = nodeUnit;
			_axp_21264_config_.system.networks[ii].mac = NULL;
			_axp_21264_config_.system.networks[ii].name = NULL;

			/*
			 * Parse out the other elements for the new network entry.
			 */
			parse_network_names(doc, cur_node->children, NoNetwork, nodeUnit, NULL);
			parent = NoNetworks;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_console_names
 *	This function parses the elements within the Console Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Console node are as follows:
 *		<Console>
 *			<Port>10</Port>
 *		</Console>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
static void parse_console_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_CONSOLE parent,
					char *value)
{
	xmlNode	*cur_node = NULL;
	char	*ptr;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_console_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _console_level_nodes[ii].token) == 0)
				{
					parent = _console_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				strcpy(value, (char *) key);
			xmlFree(key);
		}

		/*
		 * If we are parsing a Port element, then call ourselves back to get
		 * the text associated with this port and convert it to a 32-bit
		 * unsigned integer.
		 */
		if (parent == Port)
		{
			parse_console_names(doc, cur_node->children, parent, nodeValue);
			_axp_21264_config_.system.console.port = strtoul(nodeValue, &ptr, 10);
			parent = NoConsole;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_disk_names
 *	This function parses the elements within the Disk Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Disk node are as follows:
 *			<Disk number="1">
 *				<Type>Disk</Type>
 *				<Name>RZ02</Name>
 *				<Size>100.0GB</Size>
 *				<File>RZ02-1.dsk</File>
 *			</Disk>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *	unit:
 *		A value indicating the unit number associated with the network config
 *		being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_disk_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_DISK parent,
					u32 unit,
					char *value)
{
	xmlNode	*cur_node = NULL;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_disk_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _disk_level_nodes[ii].token) == 0)
				{
					parent = _disk_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				strcpy(value, (char *) key);
			xmlFree(key);
		}

		/*
		 * Depending upon what we have, go parse out the information from the
		 * configuration file.
		 */
		if (parent != NoDisk)
		{

			/*
			 * Utilizing the unit value supplied on the call, find out where we
			 * are to store the information for this network device.
			 */
			ii = 0;
			while ((ii < _axp_21264_config_.system.diskCount) &&
				   (_axp_21264_config_.system.disks[ii].unit != unit))
				ii++;

			parse_disk_names(doc, cur_node->children, parent, unit, nodeValue);
			switch (parent)
			{
				case DiskType:
					if (strcmp(nodeValue, "CDROM") == 0)
						_axp_21264_config_.system.disks[ii].type = CD_ROM;
					else if (strcmp(nodeValue, "RWCDROM") == 0)
						_axp_21264_config_.system.disks[ii].type = RW_CDROM;
					else
						_axp_21264_config_.system.disks[ii].type = Disk;
					break;

				case DiskName:
					_axp_21264_config_.system.disks[ii].name = realloc(
							_axp_21264_config_.system.disks[ii].name,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.disks[ii].name, nodeValue);
					break;

				case DiskSize:
					_axp_21264_config_.system.disks[ii].size =
							AXP_cvtSizeStr(nodeValue);
					break;

				case DiskFile:
					_axp_21264_config_.system.disks[ii].fileSpec = realloc(
							_axp_21264_config_.system.disks[ii].fileSpec,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.disks[ii].fileSpec, nodeValue);
					break;

				case NoDisk:
				default:
					break;
			}
			parent = NoDisk;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_disks_names
 *	This function parses the elements within the Disks Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Disks node are as follows:
 *			<Disks>
 *				<Disk number="1">...</Disk>
 *			</Disks>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *	unit:
 *		A value indicating the unit number associated with the network config
 *		being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_disks_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_DISKS parent)
{
	xmlNode	*cur_node = NULL;
	char	*ptr;
	int		ii;
	u32		nodeUnit;
	bool	found;

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			xmlAttr *attr = cur_node->properties;

			found = false;
			for (ii = 0;
				 ((_disks_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _disks_level_nodes[ii].token) == 0)
				{
					parent = _disks_level_nodes[ii].node;
					found = true;
				}
			}

			/*
			 * The Disk element has a single attribute called number.  Parse
			 * this out and convert it to a 32-bit integer.  We'll store it in
			 * the allocated configuration item and supply it on the code that
			 * parses out the child configuration data,
			 */
			while (attr != NULL)
			{
				if (strcmp((char *) attr->name, "number") == 0)
				{
					xmlChar *attrVal = xmlNodeListGetString(doc, attr->children, 1);

					AXP_stripXmlString(attrVal);
					if (xmlStrlen(attrVal) > 0)
						nodeUnit = strtoul((char *) attrVal, &ptr, 10);
					xmlFree(attrVal);
				}
				attr = attr->next;
			}
		}

		/*
		 * Go get and parse out the information from the configuration.
		 */
		if (parent == DECDisk)
		{

			/*
			 * We have a new disk drive to be configured.  Increment the
			 * counter of these items.
			 */
			_axp_21264_config_.system.diskCount++;

			/*
			 * Allocate an array large enough to handle an additional entry.
			 */
			_axp_21264_config_.system.disks = realloc(
						_axp_21264_config_.system.disks,
						(_axp_21264_config_.system.diskCount *
							sizeof(AXP_21264_DISK_INFO)));

			/*
			 * Set the unit number for this new entry (which should always be
			 * the last one.  Also initialize the other fields so we don't get
			 * a segmentation fault in the function are are about to call;
			 */
			ii = _axp_21264_config_.system.diskCount - 1;
			_axp_21264_config_.system.disks[ii].unit = nodeUnit;
			_axp_21264_config_.system.disks[ii].size = 0;
			_axp_21264_config_.system.disks[ii].type = Diskless;
			_axp_21264_config_.system.disks[ii].name = NULL;
			_axp_21264_config_.system.disks[ii].fileSpec = NULL;

			/*
			 * Parse out the other elements for the new disk entry.
			 */
			parse_disk_names(doc, cur_node->children, NoDisk, nodeUnit, NULL);
			parent = NoDisks;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_dimms_names
 *	This function parses the elements within the DIMMs Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the DIMMs node are as follows:
 *		<DIMMs>
 *			<Count>4</Count>
 *			<Size>4.0GB</Size>
 *		</DIMMs>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_dimms_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_DIMMS parent,
					char *value)
{
	xmlNode	*cur_node = NULL;
	char	*ptr;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_dimm_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _dimm_level_nodes[ii].token) == 0)
				{
					parent = _dimm_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				strcpy(value, (char *) key);
			xmlFree(key);
		}

		/*
		 * Parse out the relevant configuration information from the read in
		 * configuration file.
		 */
		if (parent != NoDIMMs)
		{
			parse_dimms_names(doc, cur_node->children, parent, nodeValue);
			switch (parent)
			{
				case DIMMCount:
					_axp_21264_config_.system.dimms.count =
							strtoul(nodeValue, &ptr, 10);
					break;

				case DIMMSize:
					_axp_21264_config_.system.dimms.size =
							AXP_cvtSizeStr(nodeValue);
					break;

				case NoDIMMs:
				default:
					break;
			}
			parent = NoDIMMs;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_cpus_names
 *	This function parses the elements within the CPUs Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the CPUs node are as follows:
 *		<CPUs>
 *			<Count>1</Count>
 *			<Generation>EV68CB</Generation>
 *			<Pass>5</Pass>
 *		</CPUs>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_cpus_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_CPUS parent,
					char *value)
{
	xmlNode	*cur_node = NULL;
	char	*ptr;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_cpu_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _cpu_level_nodes[ii].token) == 0)
				{
					parent = _cpu_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				strcpy(value, (char *) key);
			xmlFree(key);
		}

		/*
		 * Parse out the relevant configuration information from the read in
		 * configuration file.
		 */
		if (parent != NoCPUs)
		{
			parse_cpus_names(doc, cur_node->children, parent, nodeValue);
			switch (parent)
			{
				case CPUCount:
					_axp_21264_config_.system.cpus.count =
							strtoul(nodeValue, &ptr, 10);
					break;

				case Generation:
					ii = 0;
					while ((AXP_CPU_Configurations[ii].genStr != NULL) &&
						   (_axp_21264_config_.system.cpus.config == NULL))
					{
						if (strcmp(
								nodeValue,
								AXP_CPU_Configurations[ii].genStr) == 0)
						{
							_axp_21264_config_.system.cpus.config =
								&AXP_CPU_Configurations[ii];
						}
						ii++;
					}
					break;

				case MfgPass:
					_axp_21264_config_.system.cpus.minorType =
							strtoul(nodeValue, &ptr, 10);
					break;

				case NoCPUs:
				default:
					break;
			}
			parent = NoCPUs;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_srom_names
 *	This function parses the elements within the SROM Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the SROM node are as follows:
 *		<SROM>
 *			<InitFile>CPU Initialization Load File.dat</InitFile>
 *			<PALImage>cl67srmrom.exe</PALImage>
 *			<ROMImage>DECaxp 21264.rom</ROMImage>
 *			<NVRamFile>DECaxp 21264.nvr</NVRamFile>
 *		</SROM>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_srom_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_SROM parent,
					char *value)
{
	xmlNode	*cur_node = NULL;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_srom_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _srom_level_nodes[ii].token) == 0)
				{
					parent = _srom_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				strcpy(value, (char *)key);
			xmlFree(key);
		}

		/*
		 * Parse out the relevant configuration information from the read in
		 * configuration file.
		 */
		if (parent != NoSROM)
		{
			parse_srom_names(doc, cur_node->children, parent, nodeValue);
			switch (parent)
			{
				case InitFile:
					_axp_21264_config_.system.srom.initFile = realloc(
							_axp_21264_config_.system.srom.initFile,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.srom.initFile, nodeValue);
					break;

				case PALImage:
					_axp_21264_config_.system.srom.PALImage = realloc(
							_axp_21264_config_.system.srom.PALImage,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.srom.PALImage, nodeValue);
					break;

				case ROMImage:
					_axp_21264_config_.system.srom.ROMImage = realloc(
							_axp_21264_config_.system.srom.ROMImage,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.srom.ROMImage, nodeValue);
					break;

				case NVRamFile:
					_axp_21264_config_.system.srom.NVRamFile = realloc(
							_axp_21264_config_.system.srom.NVRamFile,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.srom.NVRamFile, nodeValue);
					break;

				case CboxCSRs:
					_axp_21264_config_.system.srom.CboxCSRFile = realloc(
							_axp_21264_config_.system.srom.CboxCSRFile,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.srom.CboxCSRFile, nodeValue);
					break;

				case NoSROM:
				default:
					break;
			}
			parent = NoSROM;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_model_names
 *	This function parses the elements within the Model Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Model node are as follows:
 *		<Model>
 *			<Name>Compaq ES40</Name>
 *			<Model>ES40</Model>
 *		</Model>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_model_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_MODEL parent,
					char *value)
{
	xmlNode	*cur_node = NULL;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are
	 * called for the first time by the parent parser.  When this happened,
	 * make sure that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_model_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _model_level_nodes[ii].token) == 0)
				{
					parent = _model_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				strcpy(value, (char *) key);
			xmlFree(key);
		}

		/*
		 * Parse out the relevant configuration information from the read in
		 * configuration file.
		 */
		if (parent != NoModel)
		{
			parse_model_names(doc, cur_node->children, parent, nodeValue);
			switch (parent)
			{
				case ModelName:
					_axp_21264_config_.system.model.name = realloc(
							_axp_21264_config_.system.model.name,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.model.name, nodeValue);
					break;

				case ModelModel:
					_axp_21264_config_.system.model.model = realloc(
							_axp_21264_config_.system.model.model,
							(strlen(nodeValue) + 1));
					strcpy(_axp_21264_config_.system.model.model, nodeValue);
					break;

				case NoModel:
				default:
					break;
			}
			parent = NoModel;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_system_names
 *	This function parses the elements within the System Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the System node are as follows:
 *		<System>
 *			<Model>...</Model>
 *			<SROM>...</SROM>
 *			<CPUs>...</CPUs>
 *			<DIMMs>...</DIMMs>
 *			<Disks>...</Disks>
 *			<Console>...</Console>
 *			<Networks>...</Networks>
 *			<Printers>...</Printers>
 *			<Tapes>...</Tapes>
 *		</System>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
static void parse_system_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_SYSTEM parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_system_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _system_level_nodes[ii].token) == 0)
				{
					parent = _system_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * Determine which node we are parsing and call the parsing function
		 * for that particular node.
		 */
		switch (parent)
		{
			case Model:
				parse_model_names(doc, cur_node->children, NoModel, NULL);
				parent = NoSystem;
				break;

			case SROM:
				parse_srom_names(doc, cur_node->children, NoSROM, NULL);
				parent = NoSystem;
				break;

			case CPUS:
				parse_cpus_names(doc, cur_node->children, NoCPUs, NULL);
				parent = NoSystem;
				break;

			case DIMMS:
				parse_dimms_names(doc, cur_node->children, NoDIMMs, NULL);
				parent = NoSystem;
				break;

			case Disks:
				parse_disks_names(doc, cur_node->children, NoDisks);
				parent = NoSystem;
				break;

			case Console:
				parse_console_names(doc, cur_node->children, NoConsole, NULL);
				parent = NoSystem;
				break;

			case Networks:
				parse_networks_names(doc, cur_node->children, NoNetworks);
				parent = NoSystem;
				break;

			case Printers:
				parse_printers_names(
							doc,
							cur_node->children,
							NoPrinters,
							NULL,
							NULL);
				parent = NoSystem;
				break;

			case Tapes:
				parse_tapes_names(doc, cur_node->children, NoTapes, NULL, NULL);
				parent = NoSystem;
				break;

			case NoSystem:
			default:
				break;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_name_names
 *	This function parses the elements within the Name Node in the XML formatted
 *	configuration file.  It extracts the value for each of the components and
 *	stores them in the configuration.  The format for the subnodes in the Name
 *	node are as follows:
 *		<Name>
 *			<First>Jonathan</First>
 *			<MI>D</MI>
 *			<Last>Belanger</Last>
 *			<Suffix />
 *		</Name>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_name_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_NAME parent,
					char *value)
{
	xmlNode	*cur_node = NULL;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are called
	 * for the first time by the parent parser.  When this happened, make sure
	 * that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_name_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _name_level_nodes[ii].token) == 0)
				{
					parent = _name_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (value != NULL)
				strcpy(value, (char *) key);
			parent = NoName;
			xmlFree(key);
		}

		/*
		 * We either have an element we are parsing, or need to continue
		 * parsing the nodes at the current level.
		 */
		switch (parent)
		{
			case FirstName:
				parse_name_names(doc, cur_node->children, parent, nodeValue);
				parent = NoName;
				_axp_21264_config_.owner.first = realloc(
												_axp_21264_config_.owner.first,
												strlen(nodeValue)+1);
				if (_axp_21264_config_.owner.first != NULL)
					strcpy(_axp_21264_config_.owner.first, nodeValue);
				nodeValue[0] = '\0';
				break;

			case MI:
				parse_name_names(doc, cur_node->children, parent, nodeValue);
				parent = NoName;
				_axp_21264_config_.owner.mi = realloc(
												_axp_21264_config_.owner.mi,
												strlen(nodeValue)+1);
				if (_axp_21264_config_.owner.mi != NULL)
					strcpy(_axp_21264_config_.owner.mi, nodeValue);
				nodeValue[0] = '\0';
				break;

			case LastName:
				parse_name_names(doc, cur_node->children, parent, nodeValue);
				parent = NoName;
				_axp_21264_config_.owner.last = realloc(
												_axp_21264_config_.owner.last,
												strlen(nodeValue)+1);
				if (_axp_21264_config_.owner.last != NULL)
					strcpy(_axp_21264_config_.owner.last, nodeValue);
				nodeValue[0] = '\0';
				break;

			case NameSuffix:
				parse_name_names(doc, cur_node->children, parent, nodeValue);
				parent = NoName;
				_axp_21264_config_.owner.suffix = realloc(
												_axp_21264_config_.owner.suffix,
												strlen(nodeValue)+1);
				if (_axp_21264_config_.owner.suffix != NULL)
					strcpy(_axp_21264_config_.owner.suffix, nodeValue);
				nodeValue[0] = '\0';
				break;

			case NoName:
			default:
				break;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * cvt_date_to_tm
 *	This function is called to convert a date string, in the form of
 *	DD-MMM-YYYY into the struct tm format.
 *
 * Input Parameters:
 *	date:
 *		A pointer to a string of the proper format.
 *
 * Output Parameter:
 *	time:
 *		A pointer to a struct tm structure to be initialized.
 *
 * Return Values:
 *	None.
 */
static void cvt_date_to_rm(char *date, struct tm *time)
{
	char	*month = NULL, *year = NULL;
	char	*ptr;
	int		ii;

	/*
	 * Go through each character of the time and upcase and alpha, then locate
	 * the start of the month and year portions.
	 */
	for (ii = 0; ii < strlen(date); ii++)
	{
		date[ii] = toupper(date[ii]);
		if (date[ii] == '-')
		{
			if (month != NULL)
				year = &date[ii+1];
			else
				month = &date[ii+1];
		}
	}

	/*
	 * Null terminate the day and month portions so that the later code is
	 * simpler.
	 */
	*(month-1) = '\0';
	*(year-1) = '\0';
	ii = 1;
	while (strcmp(month, months[ii]) != 0)
		ii++;

	/*
	 * Initialize the output parameter with the information from the input
	 * parameter.
	 */
	time->tm_hour = 0;
	time->tm_isdst = 0;
	time->tm_mday = strtol(date, &ptr, 10);
	time->tm_min = 0;
	time->tm_mon = ii;
	time->tm_sec = 0;
	time->tm_wday = 0;
	time->tm_yday = 0;
	time->tm_year = strtol(year, &ptr, 10) - 1900;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_owner_names
 *	This function parses the elements within the Owner Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the Owner node are as follows:
 *		<Owner>
 *			<Name>...</Name>
 *			<CreationDate>27-Jun-1987</CreationDate>
 *			<ModifyDate>05-Jan-2018</ModifyDate>
 *		</Owner>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	value:
 *		A pointer to a location to receive the value when the node parsed is a
 *		text node.  This parameter may be NULL, when we want to ignore the
 *		results.
 *
 * Return Values:
 *	None.
 */
static void parse_owner_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_OWNER parent,
					char *value)
{
	xmlNode	*cur_node = NULL;
	char	nodeValue[80];
	int		ii;
	bool	found;

	/*
	 * If we are called with an address to value of NULL, then we are called
	 * for the first time by the parent parser.  When this happend, make sure
	 * that the local string is zero length.
	 */
	if (value == NULL)
		nodeValue[0] = '\0';

	/*
	 * We recursively look through the node from the current one and look for
	 * either an Element Node or a Text Node.  If an Element node, there is
	 * something more to parse (handled below).  If it is a text node, then we
	 * are returning a value associated with an Element node.
	 */
	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{

		/*
		 * We have an element node.  See that is one that we care about and
		 * we'll parse it further.  Extra nodes will be ignored and duplicates
		 * will overwrite the previous value.
		 */
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_owner_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _owner_level_nodes[ii].token) == 0)
				{
					parent = _owner_level_nodes[ii].node;
					found = true;
				}
			}
		}

		/*
		 * We have a text node.  This is a value that is to be associated with
		 * an Element node.
		 */
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (value != NULL)
				strcpy(value, (char *) key);
			parent = NoName;
			xmlFree(key);
		}

		/*
		 * We either have an element we are parsing, or need to continue
		 * parsing the nodes at the current level.
		 */
		switch (parent)
		{
			case Name:
				parse_name_names(doc, cur_node->children, NoName, NULL);
				parent = NoOwner;
				break;

			case CreationDate:
				parse_owner_names(doc, cur_node->children, parent, nodeValue);
				cvt_date_to_rm(nodeValue, &_axp_21264_config_.owner.create);
				parent = NoOwner;
				nodeValue[0] = '\0';
				break;

			case ModifyDate:
				parse_owner_names(doc, cur_node->children, parent, nodeValue);
				cvt_date_to_rm(nodeValue, &_axp_21264_config_.owner.modify);
				parent = NoOwner;
				nodeValue[0] = '\0';
				break;

			case NoName:
			default:
				break;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * parse_parent_names
 *	This function parses the elements within the DECaxp Node in the XML
 *	formatted configuration file.  It extracts the value for each of the
 *	components and stores them in the configuration.  The format for the
 *	subnodes in the DECaxp node are as follows:
 *		<DECaxp>
 *			<Owner>...</Owner>
 *			<System>...</System>
 *		</DECaxp>
 *
 * Input Parameters:
 *	doc:
 *		A pointer to the XML document node being parsed.
 *	a_node:
 *		A pointer to the current node (element) being parsed.
 *	parent:
 *		A value indicating the parent node being parsed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
static void parse_parent_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_NODES parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			found = false;
			for (ii = 0;
				 ((_top_level_nodes[ii].token != NULL) && (found == false));
				 ii++)
			{
				if (strcmp((char *) cur_node->name, _top_level_nodes[ii].token) == 0)
				{
					parent = _top_level_nodes[ii].node;
					found = true;
				}
			}
		}

		switch (parent)
		{
			case DECaxp:
				parse_parent_names(doc, cur_node->children, parent);
				break;

			case Owner:
				parse_owner_names(doc, cur_node->children, NoOwner, NULL);
				parent = NoNodes;
				break;

			case SystemConf:
				parse_system_names(doc, cur_node->children, NoSystem);
				parent = NoNodes;
				break;

			case NoNodes:
			default:
				break;
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_LoadConfig_File
 *	This function is called to open a specified configuration file and parse it
 *	into memory.
 *
 * Input Parameters:
 *	fileName:
 *		A string containing the file name for the configuration file to be
 *		loaded.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	AXP_S_NORMAL		- Normal successful completion
 *	AXP_E_FNF			- File not found
 *	AXP_E_BUFTOOSMALL	- Buffer to small to load configuration
 *	AXP_E_EOF			- End-of-file reached prematurely
 *	AXP_E_READERR		- Error reading in file
 *	AXP_E_BADCFGFILE	- Invalid configuration file
 */
int AXP_LoadConfig_File(char *fileName)
{
	xmlDoc	*axpCfgDoc;
	xmlNode	*axpCfgElement = NULL;
	int		retVal = AXP_S_NORMAL;

	/*
	 * Since this needs to be called prior to allocating any CPUs, we need to
	 * initialize the CPU-ID counter and associated mutex.
	 */
	pthread_mutex_init(&_axp_config_mutex_, NULL);
	_axp_cpu_id_counter_ = 0;

	/*
	 * First check the version of the API we compiled against matches the
	 * version of the API in the library.
	 */
	LIBXML_TEST_VERSION

	/*
	 * Parse the file and get the Document Object Module (DOM).
	 */
	axpCfgDoc = xmlReadFile(fileName, NULL, 0);
	if (axpCfgDoc != NULL)
	{
		axpCfgElement = xmlDocGetRootElement(axpCfgDoc);
		if (axpCfgElement == NULL)
		{
			xmlFreeDoc(axpCfgDoc);
			xmlCleanupParser();
			retVal = AXP_E_BADCFGFILE;
		}
		else
		{

			/*
			 * For now we are just going to dump out the XML element tree
			 * (recursively).
			 */
			parse_parent_names(axpCfgDoc, axpCfgElement, NoNodes);
			xmlFreeDoc(axpCfgDoc);
			xmlCleanupParser();
		}
	}
	else
		retVal = AXP_E_READERR;

	/*
	 * Return back to the caller the final status of this call.
	 */
	return(retVal);
}

/*
 * AXP_ConfigGet_CPUType
 *	This function is called to return the major and minor values for the type
 *	of Digital Alpha AXP CPU being emulated.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	major:
 *		A pointer to a 32-bit unsigned value to receive the CPU major type ID.
 *	minor:
 *		A pointer to a 32-bit unsigned value to receive the CPU minor type ID.
 *
 * Return Value:
 * 	true:	Both values were returned.
 * 	false:	One of the value was not returned.
 */
bool AXP_ConfigGet_CPUType(u32 *major, u32 *minor)
{
	bool retVal = false;

	/*
	 * If one or the other returned parameters is not specified, then we return
	 * nothing.
	 */
	if ((major != NULL) && (minor != NULL))
	{

		/*
		 * Lock the interface mutex, get the major and minor CPU type values,
		 * then unlock the mutex.
		 */
		pthread_mutex_lock(&_axp_config_mutex_);
		*major = _axp_21264_config_.system.cpus.config->majorType;
		*minor = _axp_21264_config_.system.cpus.minorType;
		pthread_mutex_unlock(&_axp_config_mutex_);

		/*
		 * Indicate that we are returning a value for the two requested peices
		 * of information.
		 */
		retVal = true;
	}
	return(retVal);
}

/*
 * AXP_ConfigGet_UniqueCPUID
 *	This function is called to return a unique CPU-ID (starting with a value of
 *	zero).
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	A unique CPU-ID value, starting with 0.
 */
u64 AXP_ConfigGet_UniqueCPUID(void)
{
	u64	retVal;

	/*
	 * Lock the interface mutex, get the next CPU-ID, then unlock the mutex.
	 */
	pthread_mutex_lock(&_axp_config_mutex_);
	retVal = _axp_cpu_id_counter_++;
	pthread_mutex_unlock(&_axp_config_mutex_);

	/*
	 * Return the unique CPU-ID selected, back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ConfigGet_InitFile
 *	This function is called to return the value of the Initialization filename.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	initFile:
 *		A pointer to a character string to receive the configuration defined
 *		initialization filename.
 *
 * Return Values:
 *	false:	Nothing was returned.
 *	true:	A value was returned.  It could be a zero length string.
 */
bool AXP_ConfigGet_InitFile(char *initFile)
{
	bool	retVal = false;

	/*
	 * If we have a place to copy the value, then do so now.
	 */
	if (initFile != NULL)
	{

		/*
		 * Lock the interface mutex, copy the value into the return variable,
		 * then unlock the mutex.
		 */
		pthread_mutex_lock(&_axp_config_mutex_);
		strcpy(initFile, _axp_21264_config_.system.srom.initFile);
		pthread_mutex_unlock(&_axp_config_mutex_);

		/*
		 * Set the return value to true, indicating that something is being
		 * returned to the caller.
		 */
		retVal = true;
	}

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ConfigGet_PALFile
 *	This function is called to return the value of the Privileged Architecture
 *	Logic filename.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	PALfile:
 *		A pointer to a character string to receive the configuration defined
 *		PAL filename.
 *
 * Return Values:
 *	false:	Nothing was returned.
 *	true:	A value was returned.  It could be a zero length string.
 */
bool AXP_ConfigGet_PALFile(char *PALfile)
{
	bool	retVal = false;

	/*
	 * If we have a place to copy the value, then do so now.
	 */
	if (PALfile != NULL)
	{

		/*
		 * Lock the interface mutex, copy the value into the return variable,
		 * then unlock the mutex.
		 */
		pthread_mutex_lock(&_axp_config_mutex_);
		strcpy(PALfile, _axp_21264_config_.system.srom.PALImage);
		pthread_mutex_unlock(&_axp_config_mutex_);

		/*
		 * Set the return value to true, indicating that something is being
		 * returned to the caller.
		 */
		retVal = true;
	}

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ConfigGet_ROMFile
 *	This function is called to return the value of the Read-Only Memory
 *	filename.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	ROMFile:
 *		A pointer to a character string to receive the configuration defined
 *		ROM filename.
 *
 * Return Values:
 *	false:	Nothing was returned.
 *	true:	A value was returned.  It could be a zero length string.
 */
bool AXP_ConfigGet_ROMFile(char *ROMfile)
{
	bool	retVal = false;

	/*
	 * If we have a place to copy the value, then do so now.
	 */
	if (ROMfile != NULL)
	{

		/*
		 * Lock the interface mutex, copy the value into the return variable,
		 * then unlock the mutex.
		 */
		pthread_mutex_lock(&_axp_config_mutex_);
		strcpy(ROMfile, _axp_21264_config_.system.srom.ROMImage);
		pthread_mutex_unlock(&_axp_config_mutex_);

		/*
		 * Set the return value to true, indicating that something is being
		 * returned to the caller.
		 */
		retVal = true;
	}

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ConfigGet_NVRAMFile
 *	This function is called to return the value of the Non-Volatile Random
 *	Access Memory filename.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	NVRAMfile:
 *		A pointer to a character string to receive the configuration defined
 *		NVRAM filename.
 *
 * Return Values:
 *	false:	Nothing was returned.
 *	true:	A value was returned.  It could be a zero length string.
 */
bool AXP_ConfigGet_NVRAMFile(char *NVRAMfile)
{
	bool	retVal = false;

	/*
	 * If we have a place to copy the value, then do so now.
	 */
	if (NVRAMfile != NULL)
	{

		/*
		 * Lock the interface mutex, copy the value into the return variable,
		 * then unlock the mutex.
		 */
		pthread_mutex_lock(&_axp_config_mutex_);
		strcpy(NVRAMfile, _axp_21264_config_.system.srom.NVRamFile);
		pthread_mutex_unlock(&_axp_config_mutex_);

		/*
		 * Set the return value to true, indicating that something is being
		 * returned to the caller.
		 */
		retVal = true;
	}

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ConfigGet_CboxCSRFile
 *	This function is called to return the value of the Cbox CPU-System Register
 *	filename.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	CSRfile:
 *		A pointer to a character string to receive the configuration defined
 *		Cbox CSR filename.
 *
 * Return Values:
 *	false:	Nothing was returned.
 *	true:	A value was returned.  It could be a zero length string.
 */
bool AXP_ConfigGet_CboxCSRFile(char *CSRfile)
{
	bool	retVal = false;

	/*
	 * If we have a place to copy the value, then do so now.
	 */
	if (CSRfile != NULL)
	{

		/*
		 * Lock the interface mutex, copy the value into the return variable,
		 * then unlock the mutex.
		 */
		pthread_mutex_lock(&_axp_config_mutex_);
		strcpy(CSRfile, _axp_21264_config_.system.srom.CboxCSRFile);
		pthread_mutex_unlock(&_axp_config_mutex_);

		/*
		 * Set the return value to true, indicating that something is being
		 * returned to the caller.
		 */
		retVal = true;
	}

	/*
	 * Return the outcome back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_TraceConfig
 *	This function is called to write out the configuration information to the
 *	trace file.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_TraceConfig(void)
{
	char	buffer[133];
	char	*bytes[] = {"B", "KB", "MB", "GB"};
	u64		cacheSize;
	int		idx = 0;
	int		ii;
	bool	configComplete = false;

	configComplete = ((_axp_21264_config_.owner.first != NULL) &&
					  (_axp_21264_config_.owner.last != NULL) &&
					  (_axp_21264_config_.system.model.name != NULL) &&
					  (_axp_21264_config_.system.model.model != NULL) &&
					  (_axp_21264_config_.system.cpus.count > 0) &&
					  (_axp_21264_config_.system.cpus.config != NULL) &&
					  (_axp_21264_config_.system.dimms.count > 0) &&
					  (_axp_21264_config_.system.srom.initFile != NULL) &&
					  (_axp_21264_config_.system.srom.PALImage != NULL) &&
					  (_axp_21264_config_.system.srom.NVRamFile != NULL) &&
					  (_axp_21264_config_.system.diskCount > 0) &&
					  (_axp_21264_config_.system.disks != NULL) &&
					  (_axp_21264_config_.system.networkCount > 0) &&
					  (_axp_21264_config_.system.networks != NULL));

	if (AXP_UTL_OPT1)
	{
		AXP_TraceWrite("System Configuration:");
		if (configComplete == true)
		{
			AXP_TraceWrite("\tOwner:");
			AXP_TraceWrite("\t\tName:");
			AXP_TraceWrite("\t\t\tFirst:\t\t\t\'%s\'", _axp_21264_config_.owner.first);
			if ((_axp_21264_config_.owner.mi != NULL) && (strlen(_axp_21264_config_.owner.mi) > 0))
				AXP_TraceWrite("\t\t\tMiddle Initial:\t\t\'%s\'", _axp_21264_config_.owner.mi);
			AXP_TraceWrite("\t\t\tLast:\t\t\t\'%s\'", _axp_21264_config_.owner.last);
			if ((_axp_21264_config_.owner.suffix != NULL) && (strlen(_axp_21264_config_.owner.suffix) > 0))
				AXP_TraceWrite("\t\t\tSuffix:\t\t\t\'%s\'", _axp_21264_config_.owner.suffix);
			AXP_TraceWrite(
					"\t\tCreate Date:\t\t\t%02u-%s-%04u",
					_axp_21264_config_.owner.create.tm_mday,
					months[_axp_21264_config_.owner.create.tm_mon],
					(_axp_21264_config_.owner.create.tm_year+1900));
			AXP_TraceWrite(
					"\t\tModify Date:\t\t\t%02u-%s-%04u",
					_axp_21264_config_.owner.modify.tm_mday,
					months[_axp_21264_config_.owner.modify.tm_mon],
					(_axp_21264_config_.owner.modify.tm_year+1900));
			AXP_TraceWrite("\tSystem:");
			AXP_TraceWrite("\t\tModel:");
			AXP_TraceWrite("\t\t\tModel:\t\t\t%s", _axp_21264_config_.system.model.model);
			AXP_TraceWrite("\t\t\tName:\t\t\t%s", _axp_21264_config_.system.model.name);
			AXP_TraceWrite("\t\tConsole:");
			AXP_TraceWrite("\t\t\tPort:\t\t\t%u", _axp_21264_config_.system.console.port);
			AXP_TraceWrite("\t\tSROM:");
			AXP_TraceWrite("\t\t\tInitialization File:\t%s",
					_axp_21264_config_.system.srom.initFile);
			AXP_TraceWrite("\t\t\tPAL Image File:\t\t%s",
					_axp_21264_config_.system.srom.PALImage);
			AXP_TraceWrite("\t\t\tNon-volatile RAM File:\t%s",
					_axp_21264_config_.system.srom.NVRamFile);
			AXP_TraceWrite("\t\t\tROM Image File:\t\t%s",
					_axp_21264_config_.system.srom.ROMImage);
			AXP_TraceWrite("\t\tCPUs:");
			AXP_TraceWrite("\t\t\tNumber:\t\t\t%u",
					_axp_21264_config_.system.cpus.count);
			AXP_TraceWrite("\t\t\tGeneration:\t\t%s",
					_axp_21264_config_.system.cpus.config->genStr);
			AXP_TraceWrite("\t\t\tName:\t\t\t%s",
					_axp_21264_config_.system.cpus.config->name);
			AXP_TraceWrite("\t\t\tIntroduction Year:\t%u",
					_axp_21264_config_.system.cpus.config->year);
			AXP_TraceWrite("\t\t\tMajor Type:\t\t%d",
					_axp_21264_config_.system.cpus.config->majorType);
			AXP_TraceWrite("\t\t\tMinor Type:\t\t%d",
					_axp_21264_config_.system.cpus.minorType);
			cacheSize = _axp_21264_config_.system.cpus.config->iCacheSize;
			while (cacheSize > ONE_K)
			{
				cacheSize /= ONE_K;
				idx++;
			}
			AXP_TraceWrite("\t\t\tI-Cache Size:\t\t%llu%s", cacheSize, bytes[idx]);
			cacheSize = _axp_21264_config_.system.cpus.config->dCacheSize;
			idx = 0;
			while (cacheSize > ONE_K)
			{
				cacheSize /= ONE_K;
				idx++;
			}
			AXP_TraceWrite("\t\t\tD-Cache Size:\t\t%llu%s", cacheSize, bytes[idx]);
			cacheSize = _axp_21264_config_.system.cpus.config->sCacheSize;
			idx = 0;
			while (cacheSize > ONE_K)
			{
				cacheSize /= ONE_K;
				idx++;
			}
			AXP_TraceWrite("\t\t\tS-Cache Size:\t\t%llu%s", cacheSize, bytes[idx]);
			cacheSize = _axp_21264_config_.system.cpus.config->bCacheSizeLow;
			idx = 0;
			while (cacheSize > ONE_K)
			{
				cacheSize /= ONE_K;
				idx++;
			}
			sprintf(buffer, "\t\t\tB-Cache Size:\t\tbetween %llu%s", cacheSize, bytes[idx]);
			cacheSize = _axp_21264_config_.system.cpus.config->bCacheSizeHigh;
			idx = 0;
			while (cacheSize > ONE_K)
			{
				cacheSize /= ONE_K;
				idx++;
			}
			AXP_TraceWrite("%s and %llu%s", buffer, cacheSize, bytes[idx]);
			AXP_TraceWrite("\t\tDIMMs:");
			AXP_TraceWrite("\t\t\tNumber:\t\t\t%u",
					_axp_21264_config_.system.dimms.count);
			cacheSize = _axp_21264_config_.system.dimms.size;
			idx = 0;
			while (cacheSize > ONE_K)
			{
				cacheSize /= ONE_K;
				idx++;
			}
			AXP_TraceWrite("\t\t\tSize:\t\t\t%llu%s", cacheSize, bytes[idx]);
			AXP_TraceWrite("\t\tNetworks:");
			AXP_TraceWrite("\t\t\tNumber:\t\t\t%u",
					_axp_21264_config_.system.networkCount);
			for (ii = 0; ii < _axp_21264_config_.system.networkCount; ii++)
			{
				AXP_TraceWrite("\t\t\t\t[%d] Unit:\t%d", ii,
						_axp_21264_config_.system.networks[ii].unit);
				AXP_TraceWrite("\t\t\t\t    Name:\t%s",
						_axp_21264_config_.system.networks[ii].name);
				AXP_TraceWrite("\t\t\t\t    MAC Addr:\t%s",
						_axp_21264_config_.system.networks[ii].mac);
			}
			AXP_TraceWrite("\t\tDisk Drives:");
			AXP_TraceWrite("\t\t\tNumber:\t\t\t%u",
					_axp_21264_config_.system.diskCount);
			for (ii = 0; ii < _axp_21264_config_.system.diskCount; ii++)
			{
				AXP_TraceWrite("\t\t\t\t[%d] Unit:\t%u", ii,
						_axp_21264_config_.system.disks[ii].unit);
				AXP_TraceWrite("\t\t\t\t   Name:\t%s",
							_axp_21264_config_.system.disks[ii].name);
				AXP_TraceWrite("\t\t\t\t   File:\t%s",
							_axp_21264_config_.system.disks[ii].fileSpec);
				switch(_axp_21264_config_.system.disks[ii].type)
				{
					case Disk:
						strcpy(buffer, "Hard Disk");
						break;

					case CD_ROM:
						strcpy(buffer, "CD-ROM");
						break;

					case RW_CDROM:
						strcpy(buffer, "R/W CD-ROM");
						break;

					case Diskless:
					default:
						strcpy(buffer, "No Disk Defined");
						break;
				}
				AXP_TraceWrite("\t\t\t\t   Type:\t%s", buffer);
				cacheSize = _axp_21264_config_.system.disks[ii].size;
				idx = 0;
				while (cacheSize > ONE_K)
				{
					cacheSize /= ONE_K;
					idx++;
				}
				AXP_TraceWrite("\t\t\t\t   Size:\t%llu%s", cacheSize, bytes[idx]);
			}
		}
		else
		{
			AXP_TraceWrite("\n\t**************** System Configuration Not Initialized ****************\n");
		}
	}

	/*
	 * Return back to the caller:
	 */
	return;
}
