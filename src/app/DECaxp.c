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
 *	This source file contains the main function for the Digital Alpha AXP 21264
 *	Emulator.
 *
 * Revision History:
 *
 *	V01.000		20-Jan-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_Utility.h"
#include "AXP_Trace.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_CboxDefs.h"
#include "AXP_21264_Cbox.h"

/*
 * reconstituteFilename
 *	This function is called with argc and argv and looks for the parts of a
 *	filename that have spaces and got scattered into the argument vector.
 *
 * Input Parameters:
 *	argc:
 *		A value indicating the number of entries in the argv parameter.  This
 *		parameter is always one more than the actual arguments provided on the
 *		command line.
 *	argv:
 *		An array, limit argc, of strings representing the image filename being
 *		executed, please each of the arguments provided on the command line.
 *
 * Output Parameters:
 *	configFilename:
 *		A pointer to a string to receive the reconstituted filename.
 *
 * Return Values:
 * 	None.
 */
void reconstituteFilename(int argc, char **argv, char *configFilename)
{
	int ii;

	/*
	 * If a configuration filename parameter was supplied on the call, then
	 * loop through each of the arguments append them to the filename string.
	 */
	if (configFilename != NULL)
	{
		configFilename[0] = '\0';
		for (ii = 1; ii < argc; ii++)
		{
			if (argv[ii][strlen(argv[ii])-1] == '\\')
				argv[ii][strlen(argv[ii])-1] = '\0';
			sprintf(&configFilename[strlen(configFilename)], "%s ", argv[ii]);
		}

		/*
		 * The filename string has an extra space on the end.  Remove it.
		 */
		ii = strlen(configFilename);
		if (ii > 0)
			configFilename[ii-1] = '\0';
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * main
 *	This is the main function for the Digital Alpha AXP 21264 Emulator.  It
 *	allocates the things it needs to and then starts the ball rolling around
 *	getting everything initialized.  The other threads will handle the rest of
 *	the work.
 *
 * Input Parameters:
 *	argc:
 *		A value indicating the number of entries in the argv parameter.  This
 *		parameter is always one more than the actual arguments provided on the
 *		command line.
 *	argv:
 *		An array, limit argc, of strings representing the image filename being
 *		executed, please each of the arguments provided on the command line.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	0:	Normal Successful Completion.
 *	!0:	An Error occurred that is causing the image to exit.
 */
int main(int argc, char **argv)
{
	AXP_21264_CPU	*cpu = NULL;
	char			filename[167];
	int				retVal = 0;

	printf("\n%%DECAXP-I-START, The Digital Alpha AXP 21264 CPU Emulator is starting.\n");

	/*
	 * There are 2 arguments on the image activation.  The first is this image
	 * and can be ignore.  The next is the configuration file to be opened.
	 * Because filenames can have spaces and these spaces will utilize
	 * successive argv locations.  Let's call a function and reconstitute them
	 * into a single file specification.
	 */
	if (argc > 1)
	{
		reconstituteFilename(argc, argv, filename);
		if ((AXP_LoadConfig_File(filename) == AXP_S_NORMAL) &&
			(AXP_TraceInit() == true))
			cpu = AXP_21264_AllocateCPU();
		if (cpu != NULL)
		{
			pthread_join(cpu->cBoxThreadID, NULL);

			/*
			 * The following calls are just to keep the linker happy.
			 */
			if (cpu == NULL)
			{
				AXP_21264_Set_IRQ(cpu, 0);
				AXP_21264_Add_PQ(
							cpu,
							AXP_21264_SET_PROBE(AXP_21264_DM_NOP, AXP_21264_NS_NOP),
							NOPsysdc,
							0x0000000000000000ll,
							0,
							NULL,
							false,
							false,
							false,
							false);
			}
			AXP_TraceEnd();
		}
		else
			printf("\n%%DECAXP-F-RUNNING, The Digital Alpha AXP 21264 CPU Emulator failed to successfully start.\n");
	}
	else
	{
		printf("usage: DECaxp <config-file>\n");
	}
	return(retVal);
}

