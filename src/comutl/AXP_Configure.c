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
 * Module level globals used for searching through the tree.
 */

static void stripXmlString(xmlChar *value)
{
	xmlChar *start, *begin, *end;
	bool	done = false;
	int		len;

	len = xmlStrlen(value) - 1;
	begin = start = value;
	end = value + len;
	while ((start <= end) && (done == false))
	{
		if (isspace(*start))
			start++;
		else
			done = true;
	}
	done = false;
	while ((end >= start) && (done == false))
	{
		if (isspace(*end))
			end--;
		else
			done = true;
	}
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
	else if (start > end)
		*value = '\0';
	return;
}

static void print_element_names(xmlDocPtr doc, xmlNode *a_node)
{
	xmlNode	*cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			xmlAttr *attr = cur_node->properties;

			printf("node type: Element, name: %s\n", cur_node->name);

			while (attr != NULL)
			{
				xmlChar *attrVal = xmlNodeListGetString(doc, attr->children, 1);

				stripXmlString(attrVal);
				if (xmlStrlen(attrVal) > 0)
					printf(
						"    attribute name: \'%s\'; value: \'%s\'\n",
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
			stripXmlString(key);
			if (xmlStrlen(key) > 0)
				printf("text node: \'%s\'\n", key);
			xmlFree(key);
		}

		print_element_names(doc, cur_node->children);
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
			 */
			print_element_names(axpCfgDoc, axpCfgElement);
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
