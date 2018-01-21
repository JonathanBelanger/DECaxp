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
#include "AXP_21264_CPU.h"
#include "AXP_21264_CboxDefs.h"
#include "AXP_21264_Cbox.h"

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
 */
int main(int argc, char **argv)
{
	int				retVal = 0;
	AXP_21264_CPU	*cpu;

	cpu = AXP_21264_AllocateCPU();
	if (cpu != NULL)
	{
		printf("\n%%DECAXP-I-RUNNING, The Digital Alpha AXP 21264 CPU Emulator has successfully started.\n");
		pthread_join(cpu->cBoxThreadID);

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
	}
	else
		printf("\n%%DECAXP-F-RUNNING, The Digital Alpha AXP 21264 CPU Emulator failed to successfully start.\n");
	return(retVal);
}

