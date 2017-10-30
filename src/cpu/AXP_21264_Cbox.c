/*
 * Copyright (C) Jonathan D. Belanger 2017.
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
 *	This source file contains the functions needed to implement the
 *	functionality of the Cbox.
 *
 *	Revision History:
 *
 *	V01.000		26-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		28-Oct-2017	Jonathan D. Belanger
 *	Time to fill out the functionality of the Cbox.  Some of the functions
 *	previously defined, may be going away.  We are going to need functions for
 *	all the requests needed by the CPU of the system (from Mbox, Ibox, and
 *	Cbox).  Additionally, IRQ_H interrupt signals from the System to the Ibox
 *	for processing.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Cbox.h"

/*
 * AXP_VictimBuffer
 *	This function is responsible for managing the Victim Buffer (VB).  The VB
 *	is comprised of the Victim Address File (VAF) and Victim Data File (VDF).
 *	There are a total of eight(8) entries in the VB.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_VictimBuffer()
{
	return;
}

/*
 * AXP_IOWriteBuffer
 *	This function is responsible for managing the I/O Write Buffer (IOWB).
 *	There are a total of four(4) 64-byte entries in the IOWB.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_IOWriteBuffer()
{
	return;
}

/*
 * AXP_ProbeQueue
 *	This function is responsible for managing the Probe Queue (PQ).
 *	There are a total of eight(8) entries in the PQ.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_ProbeQueue()
{
	return;
}

/*
 * AXP_DuplicateDcacheTagArray
 *	This function is responsible for managing the duplicate Dcache Tag (DTAG)
 *	array and is used by the Cbox when processing Dcache fills, Icache fills
 *	and system port probes.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_DuplicateDcacheTagArray()
{
	return;
}

/*
 * AXP_IQArbiter
 *	This function is responsible for the arbitration of instructions pending in
 *	the Integer Issue Queue (IQ).  Architecturally, this is defined in the Cbox
 *	as the "Arbiter".  According to the architecture, there are two(2) arbiters
 *	for the IQ, one for the upper subclusters and one for the lower.  Each will
 *	select two instructions from the 20 possible queued integer instructions.
 *	Priority is given to older requests over newer ones.  If one instruction
 *	requests both lower subclusters and there is no other requesting a lower
 *	subcluster, then L0 is selected.  The same, but for the upperclusters will
 *	select U1.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_IQArbiter()
{
	return;
}

/*
 * AXP_FQArbiter
 *	This function is responsible for the arbitration of instructions pending in
 *	the Floating-point Issue Queue (FQ).  Architecturally, this is defined in
 *	the Cbox as the "Arbiter".  According to the architecture, there are
 *	three(3) arbiters for the FQ, one for each add, multiply, or store
 *	pipelines.  The add and multiply arbiters will pick one request, while the
 *	store arbiter will pick two (one for each store pipeline).  Priority is
 *	given to older requests over newer ones.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_FQArbiter()
{
	return;
}

/*
 * AXP_21264_Cbox_Init
 * 	This function is called to initialize the Cbox.  It will read in the
 * 	initialization file, which contains the settings for the Cbox CSRs.  It
 * 	will then load the SROM data into the Icache and finally set the BiSTState
 * 	if all went well.  Otherwise, it will it will return a false (BiST
 * 	failure).
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 * 	Cbox CSRs will be set and Icache loaded with SROM initialization code.
 *
 * Return Value:
 * 	true:	Failed to perform all initialization functions.
 * 	false:	Succeeded.
 */
bool AXP_21264_Cbox_Init(AXP_21264_CPU *cpu)
{
	bool		retVal = false;
	bool		readResult = true;
	FILE		*fp;
	const char	*configFile = "../dat/AXP_21264_Cbox_CSR.nvp";
	char		name[32];
	u32			value;

	fp = AXP_Open_NVP_File(configFile);
	if (fp != NULL)
	{
		while (readResult == true)
		{
			readResult = AXP_Read_NVP_File(fp, name, &value);
			if (readResult == false)
			{
				retVal = true;
				continue;
			}
		}
	}

	return(retVal);
}

/*
 * AXP_21264_Cbox_Main
 * 	This is the main function for the Cbox.  It looks at each of the queues to
 * 	determine if there is anything that needs to be processed for the various
 * 	queues from other CPU boxes and the System.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_21264_Cbox_Main(AXP_21264_CPU *cpu)
{

	/*
	 * The Cbox is very involved in the initialization of the CPU at power-up,
	 * fault-resetting, and waking up from sleep.  The Cbox CSRs are
	 * initialized at this point and the the SROM is loaded into the Icache.
	 * Once all this has been done, then we put the CPU into a Running state,
	 * at which point all hell can break lose.
	 *
	 * When we first come in the CPU should be in a Cold state.  We will be
	 */
	while (cpu->cpuState != ShuttingDown)
	{

		/*
		 * If we are waiting for BiST (Build-in SelfTest), we need to perform
		 * some initialization on the various parts of the CPU (this is our
		 * BiST processing).
		 */
		if ((cpu->cpuState != WaitBiST) && (cpu->BiSTState == SystemReset))
		{
			bool initFailure = false;
			int component = 0;

			/*
			 * We have a SystemReset, set the BiSTState appropriately.
			 */
			cpu->BiSTState = BiSTRunning;

			/*
			 * The other components (Ibox, Ebox, Fbox, and Mbox) should have
			 * their initialization function called to perform the
			 * initialization they need to do for themselves.  After this,
			 * they'll all be waiting for the CPU to go into Running State,
			 * which is set here after the Cbox finished its own
			 * initialization.  If any of the initialization routines return
			 * an error, an error should have been displayed and we'll change
			 * the CPU state to ShuttingDown (which will cause all the other
			 * components to shutdown as well).
			 */
			while (initFailure == false)
			{
				switch (component)
				{
					case 0:		/* Mbox */
						initFailure = AXP_21264_Mbox_Init(cpu);
						break;

					case 1:		/* Ebox */
						initFailure = AXP_21264_Ebox_Init(cpu);
						break;

					case 2:		/* Fbox */
						initFailure = AXP_21264_Fbox_Init(cpu);
						break;

					case 3:		/* Ibox */
						initFailure = AXP_21264_Ibox_Init(cpu);
						break;

					case 4:		/* Cbox */
						initFailure = AXP_21264_Cbox_Init(cpu);
				}
				if (initFailure == false)
					component++;
			}

			/*
			 * If any of the initializations failed, then the BiST failed.  We
			 * will have to perform a bunch of shutdown logic outside the top
			 * while loop.
			 */
			if (initFailure == true)
				cpu->BiSTState = BiSTFailed;
		}
	}
	return;
}
