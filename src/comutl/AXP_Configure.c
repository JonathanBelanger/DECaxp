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

/*
 * Global variable used throughout the emulator.
 */
AXP_21264_CONFIG	AXP_21264_Config =
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
	.system.cpus.name = NULL,
	.system.cpus.count = 0,
	.system.cpus.pass = 0,
	.system.cpus.generation = NoGen,
	.system.dimms.size = 0,
	.system.dimms.count = 0
};

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
	{"CPUS", CPUS},
	{"DIMMS", DIMMS},
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
	{"NVRAMFile", NVRamFile},
	{NULL, NoSROM}
};
static struct AXP_CPUS _cpu_level_nodes[] =
{
	{"Count", CPUCount},
	{"Generation", Generation},
	{"Pass", MfgPass},
	{"Name", CPUName},
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
 * print_tapes_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_tapes_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_TAPES parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("Tapes: node type: Element, name: %s\n", cur_node->name);

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
		}
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("Tapes: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoTapes)
		{
			printf("Tapes: Calling Tapes.\n");
			print_tapes_names(doc, cur_node->children, parent);
			parent = NoTapes;
			printf("Tapes: Returning Tapes.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_printers_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_printers_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_PRINTERS parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("Printers: node type: Element, name: %s\n", cur_node->name);

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
		}
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("Printers: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoPrinters)
		{
			printf("Printers: Calling Printers.\n");
			print_printers_names(doc, cur_node->children, parent);
			parent = NoPrinters;
			printf("Printers: Returning Printers.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_network_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_network_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_NETWORK parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			xmlAttr *attr = cur_node->properties;

			printf("Network: node type: Element, name: %s\n", cur_node->name);

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

			while (attr != NULL)
			{
				xmlChar *attrVal = xmlNodeListGetString(doc, attr->children, 1);

				AXP_stripXmlString(attrVal);
				if (xmlStrlen(attrVal) > 0)
					printf(
						"Network:    attribute name: \'%s\'; value: \'%s\'\n",
						attr->name,
						attrVal);
				xmlFree(attrVal);
				attr = attr->next;
			}
		}
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("Network: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoNetwork)
		{
			printf("Network: Calling Network.\n");
			print_network_names(doc, cur_node->children, parent);
			parent = NoNetwork;
			printf("Network: Returning Network.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_networks_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_networks_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_NETWORKS parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("Networks: node type: Element, name: %s\n", cur_node->name);

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
		}
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("Networks: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent == TopNetworks)
		{
			printf("Networks: Calling Network.\n");
			print_network_names(doc, cur_node->children, NoNetwork);
			parent = NoNetworks;
			printf("Networks: Returning Network.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_console_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_console_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_CONSOLE parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("Console: node type: Element, name: %s\n", cur_node->name);

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
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("Console: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoConsole)
		{
			printf("Console: Calling Console.\n");
			print_console_names(doc, cur_node->children, parent);
			parent = NoConsole;
			printf("Console: Returning Console.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_disk_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_disk_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_DISK parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			xmlAttr *attr = cur_node->properties;

			printf("Disk: node type: Element, name: %s\n", cur_node->name);

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

			while (attr != NULL)
			{
				xmlChar *attrVal = xmlNodeListGetString(doc, attr->children, 1);

				AXP_stripXmlString(attrVal);
				if (xmlStrlen(attrVal) > 0)
					printf(
						"Disk:    attribute name: \'%s\'; value: \'%s\'\n",
						attr->name,
						attrVal);
				xmlFree(attrVal);
				attr = attr->next;
			}
		}
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("Disk: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoDisk)
		{
			printf("Disk: Calling Disk.\n");
			print_disk_names(doc, cur_node->children, parent);
			parent = NoDisk;
			printf("Disk: Returning Disk.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_disks_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_disks_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_DISKS parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("Disks: node type: Element, name: %s\n", cur_node->name);

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
		}
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("Disks: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent == DECDisk)
		{
			printf("Disks: Calling Disk.\n");
			print_disk_names(doc, cur_node->children, NoDisk);
			parent = NoDisks;
			printf("Disks: Returning Disk.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_dimms_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_dimms_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_DIMMS parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			xmlAttr *attr = cur_node->properties;

			printf("DIMMs: node type: Element, name: %s\n", cur_node->name);

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

			while (attr != NULL)
			{
				xmlChar *attrVal = xmlNodeListGetString(doc, attr->children, 1);

				AXP_stripXmlString(attrVal);
				if (xmlStrlen(attrVal) > 0)
					printf(
						"DIMMs:    attribute name: \'%s\'; value: \'%s\'\n",
						attr->name,
						attrVal);
				xmlFree(attrVal);
				attr = attr->next;
			}
		}
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("DIMMS: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoDIMMs)
		{
			printf("DIMMs: Calling DIMMs.\n");
			print_dimms_names(doc, cur_node->children, parent);
			parent = NoDIMMs;
			printf("DIMMs: Returning DIMMs.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_cpus_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_cpus_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_CPUS parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("CPUs: node type: Element, name: %s\n", cur_node->name);

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
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("CPUs: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoCPUs)
		{
			printf("CPUs: Calling CPUs.\n");
			print_cpus_names(doc, cur_node->children, parent);
			parent = NoCPUs;
			printf("CPUs: Returning CPUs.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_srom_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_srom_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_SROM parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("SROM: node type: Element, name: %s\n", cur_node->name);

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
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("SROM: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoSROM)
		{
			printf("SROM: Calling SROM.\n");
			print_srom_names(doc, cur_node->children, parent);
			parent = NoSROM;
			printf("SROM: Returning SROM.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_model_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_model_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_MODEL parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("Model: node type: Element, name: %s\n", cur_node->name);

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
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("Model: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		if (parent != NoModel)
		{
			printf("Model: Calling Model.\n");
			print_model_names(doc, cur_node->children, parent);
			parent = NoModel;
			printf("Model: Returning Model.\n");
		}
    }

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * print_system_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_system_names(
					xmlDocPtr doc,
					xmlNode *a_node,
					AXP_21264_CONFIG_SYSTEM parent)
{
	xmlNode	*cur_node = NULL;
	int		ii;
	bool	found;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			printf("System: node type: Element, name: %s\n", cur_node->name);

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
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("System: text node: \'%s\'\n", key);
			xmlFree(key);
		}
		switch (parent)
		{
			case Model:
				printf("System: Calling Model.\n");
				print_model_names(doc, cur_node->children, NoModel);
				parent = NoSystem;
				printf("System: Returning Model.\n");
				break;

			case SROM:
				printf("System: Calling SROM.\n");
				print_srom_names(doc, cur_node->children, NoSROM);
				parent = NoSystem;
				printf("System: Returning SROM.\n");
				break;

			case CPUS:
				printf("System: Calling CPUs.\n");
				print_cpus_names(doc, cur_node->children, NoCPUs);
				parent = NoSystem;
				printf("System: Returning CPUs.\n");
				break;

			case DIMMS:
				printf("System: Calling DIMMs.\n");
				print_dimms_names(doc, cur_node->children, NoDIMMs);
				parent = NoSystem;
				printf("System: Returning DIMMs.\n");
				break;

			case Disks:
				printf("System: Calling Disks.\n");
				print_disks_names(doc, cur_node->children, NoDisks);
				parent = NoSystem;
				printf("System: Returning Disks.\n");
				break;

			case Console:
				printf("System: Calling Console.\n");
				print_console_names(doc, cur_node->children, NoConsole);
				parent = NoSystem;
				printf("System: Returning Console.\n");
				break;

			case Networks:
				printf("System: Calling Networks.\n");
				print_networks_names(doc, cur_node->children, NoNetworks);
				parent = NoSystem;
				printf("System: Returning Networks.\n");
				break;

			case Printers:
				printf("System: Calling Printers.\n");
				print_printers_names(doc, cur_node->children, NoPrinters);
				parent = NoSystem;
				printf("System: Returning Printers.\n");
				break;

			case Tapes:
				printf("System: Calling Tapes.\n");
				print_tapes_names(doc, cur_node->children, NoTapes);
				parent = NoSystem;
				printf("System: Returning Tapes.\n");
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
				AXP_21264_Config.owner.first = realloc(
												AXP_21264_Config.owner.first,
												strlen(nodeValue)+1);
				if (AXP_21264_Config.owner.first != NULL)
					strcpy(AXP_21264_Config.owner.first, nodeValue);
				nodeValue[0] = '\0';
				break;

			case MI:
				parse_name_names(doc, cur_node->children, parent, nodeValue);
				parent = NoName;
				AXP_21264_Config.owner.mi = realloc(
												AXP_21264_Config.owner.mi,
												strlen(nodeValue)+1);
				if (AXP_21264_Config.owner.mi != NULL)
					strcpy(AXP_21264_Config.owner.mi, nodeValue);
				nodeValue[0] = '\0';
				break;

			case LastName:
				parse_name_names(doc, cur_node->children, parent, nodeValue);
				parent = NoName;
				AXP_21264_Config.owner.last = realloc(
												AXP_21264_Config.owner.last,
												strlen(nodeValue)+1);
				if (AXP_21264_Config.owner.last != NULL)
					strcpy(AXP_21264_Config.owner.last, nodeValue);
				nodeValue[0] = '\0';
				break;

			case NameSuffix:
				parse_name_names(doc, cur_node->children, parent, nodeValue);
				parent = NoName;
				AXP_21264_Config.owner.suffix = realloc(
												AXP_21264_Config.owner.suffix,
												strlen(nodeValue)+1);
				if (AXP_21264_Config.owner.suffix != NULL)
					strcpy(AXP_21264_Config.owner.suffix, nodeValue);
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
	time->tm_mday = strtol(month, &ptr, 10);
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
				cvt_date_to_rm(nodeValue, &AXP_21264_Config.owner.create);
				parent = NoOwner;
				nodeValue[0] = '\0';
				break;

			case ModifyDate:
				parse_owner_names(doc, cur_node->children, parent, nodeValue);
				cvt_date_to_rm(nodeValue, &AXP_21264_Config.owner.modify);
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
 * print_parent_names
 *	This function is a precursor to the actual parsing code.  It scans through
 *	the XML document and pulls out the configuration information and displays
 *	it to stdout.
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
static void print_parent_names(
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
			printf("DECaxp: node type: Element, name: %s\n", cur_node->name);

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
		else if (XML_TEXT_NODE == cur_node->type)
		{
			xmlChar	*key;

			key = xmlNodeListGetString(doc, cur_node, 1);
			AXP_stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("DECaxp: text node: \'%s\'\n", key);
			xmlFree(key);
		}

		switch (parent)
		{
			case DECaxp:
				printf("DECaxp: Calling DECaxp.\n");
				print_parent_names(doc, cur_node->children, parent);
				printf("DECaxp: Calling DECaxp.\n");
				break;

			case Owner:
				parse_owner_names(doc, cur_node->children, NoOwner, NULL);
				parent = NoNodes;
				break;

			case SystemConf:
				printf("DECaxp: Calling System.\n");
				print_system_names(doc, cur_node->children, NoSystem);
				parent = NoNodes;
				printf("DECaxp: Returning System.\n");
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
	xmlDoc	*axpCfgDoc = NULL;
	xmlNode	*axpCfgElement = NULL;
	int		retVal = AXP_S_NORMAL;

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
			 *
			 * TODO:	The recursive code has instructions to try and call
			 *			itself with a child and there will never be one.  So
			 *			this code should be removed.
			 */
			print_parent_names(axpCfgDoc, axpCfgElement, NoNodes);
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

