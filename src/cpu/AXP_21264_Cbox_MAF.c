
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
 *	Miss Address File (MAF) functionality of the Cbox.
 *
 *	Revision History:
 *
 *	V01.000		29-Dec-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21264_Cbox.h"
#include "AXP_Configure.h"
#include "AXP_21264_CacheDefs.h"
#include "AXP_21264_Mbox.h"
#include "AXP_21264_Ebox.h"
#include "AXP_21264_Fbox.h"
#include "AXP_21264_Ibox.h"

/*
 * AXP_21264_MAF_Empty
 *	This function is called to determine of there is a record in the Missed
 *	Address File (MAF) that needs to be processed.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	-1:		No entries requiring processing.
 *	>=0:	Index of next entry that can be processed.
 */
int AXP_21264_MAF_Empty(AXP_21264_CPU *cpu)
{
	int		retVal = -1;
	int		ii;
	int		end, start1, start2 = -1, end1, end2;

	if (cpu->mafTop > cpu->mafBottom)
	{
		start1 = cpu->mafTop;
		end1 = AXP_21264_MAF_LEN - 1;
		start2 = 0;
		end2 = cpu->mafBottom;
	}
	else
	{
		start1 = cpu->mafTop;
		end1 = cpu->mafBottom;
	}

	/*
	 * Search through the list to find the first entry that can be processed.
	 */
	ii = start1;
	end = end1;
	while ((ii <= end) && (retVal == -1))
	{
		if ((cpu->maf[ii].type != MAFNotInUse) &&
			(cpu->maf[ii].complete == false))
			retVal = ii;
		if ((retVal == -1) && (start2 != -1) && (ii == end))
		{
			ii = start2;
			end = end2;
			start2 = -1;
		}
		else
			ii++;
	}

	/*
	 * Return what we found, if anything.
	 */
	return(retVal);
}

/*
 * AXP_21264_Process_MAF
 *	This function is called to check the first unprocessed entry on the queue
 *	containing the MAF records.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the MAF to be processed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Process_MAF(AXP_21264_CPU *cpu, int entry)
{
	AXP_21264_CBOX_MAF		*maf = &cpu->maf[entry];
	AXP_21264_TO_SYS_CMD	cmd;
	bool					m1 = false;
	bool					m2 = false;
	bool					rv = true;
	bool					ch = false;

	/*
	 * Process the next MAF entry that needs it.
	 *
	 * TODO: Need to look at Speculative Transactions.
	 */
	switch(maf->type)
	{
		case LDx:
			if (maf->ioReq == true)
			{
				switch(maf->dataLen)
				{
					case BYTE_LEN:
						cmd = ReadBytes;
						break;

					case WORD_LEN:
						cmd = ReadBytes;
						break;

					case LONG_LEN:
						cmd = ReadLWs;
						break;

					case QUAD_LEN:
						cmd = ReadQWs;
						break;
				}
			}
			else
				cmd = ReadBlk;
			break;

		case STx:
		case STx_C:
			cmd = ReadBlkMod;
			break;

		case STxChangeToDirty:
			if (maf->shared == true)
				cmd = SharedToDirty;
			else
				cmd = CleanToDirty;
			break;

		case STxCChangeToDirty:
			cmd = STCChangeToDirty;
			break;

		case WH64:
			cmd = InvalToDirty;
			break;

		case ECB:
			cmd = Evict;
			break;

		case Istream:
			cmd = ReadBlkI;
			break;

		case MemoryBarrier:
			cmd = MB;
			break;

		default:
			break;
	}

	/*
	 * Go check the Oldest pending PQ and set the flags for it here and now.
	 */
	AXP_21264_OldestPQFlags(cpu, &m1, &m2, &ch);

	/*
	 * OK, send what we have to the System.
	 */
	AXP_System_CommandSend(cmd, m2, entry, rv, maf->mask, ch, maf->pa, NULL, 0);

	/*
	 * Indicate that the entry is now processed and return back to the caller.
	 */
	maf->complete = true;
	return;
}

/*
 * AXP_21264_Complete_MAF
 *	This function is called when a probe sent by the System indicates one of
 *	the Reads.  This includes both I/O and Memory.  For I/O, the actual length
 *	of the returned data is what was requested in an MAF entry.  Also, because
 *	of the potential for merging MAF entries, we may need to part out the data
 *	as we proceed through the buffer.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *	entry:
 *		A value indicating the ID associated with the MAF entry that sent the
 *		request to the system.
 *	sysDc:
 *		A value indicating the response sent by the system.
 *	sysData:
 *		A pointer to a 64-byte buffer containing the data returned by the
 *		system.
 */
void AXP_21264_Complete_MAF(
					AXP_21264_CPU *cpu,
					int entry,
					AXP_21264_SYSDC_RSP sysDc,
					u8 *sysData)
{
	AXP_21264_CBOX_MAF	*maf = &cpu->maf[entry];
	int					curPtr;
	int					bufIndex;
	int					dataLen;
	int					ii = 0;
	u8					cacheStatus;
	bool				error = (sysDc == ReadDataError);

	AXP_MaskStartGet(&curPtr);

	/*
	 * HRM 4.5.4 Using SysDc Commands
	 *
	 * Note the following:
	 * 		-	The conventional response for RdBlk commands is SysDc ReadData
	 * 			or ReadDataShared.
	 * 		-	The conventional response for a RdBlkMod command is SysDc
	 * 			ReadDataDirty.
	 * 		-	The conventional response for ChangeToDirty commands is
	 * 			ChangeToDirtySuccess or ChangeToDirtyFail.
	 *
	 * However, the system environment is not limited to these responses.
	 * Table 4–5 (documented in the AXP_21264_Cbox_PQ.c module) shows all 21264
	 * commands, system responses, and the 21264 reaction. The 21264 commands
	 * are described in the following list: (case comments below).
	 *
	 * TODO: Need to consider Fetchx and MBDone.
	 */
	switch(maf->type)
	{
		case MAFNotInUse:
			break;

		/*
		 * Rdx:		Commands are generated by load (from memory) or Istream
		 * 			references.
		 * Rdiox:	Commands are non-cached references to I/O address space.
		 */
		case LDx:
		case Istream:

			/*
			 * Rdx
			 */
			if (maf->ioReq == false)
			{

				/*
				 * ReadBlk (data length = 64)
				 * ReadBlkI (data length = 64)
				 */
				dataLen = AXP_DCACHE_DATA_LEN;
				while (curPtr != -1)
				{
					bufIndex = AXP_MaskGet(&curPtr, maf->mask, dataLen);
					switch(sysDc)
					{

						/*
						 * This is a normal fill. The cache block is filled and
						 * marked clean or shared based on SysDc.
						 */
						case ReadData:
							cacheStatus = AXP_21264_CACHE_CLEAN;
							break;

						case ReadDataShared:
							cacheStatus = AXP_21264_CACHE_SHARED;
							break;

						/*
						 * The cache block is filled and marked dirty/shared.
						 * Succeeding store commands cannot update the block
						 * without external reference.
						 */
						case ReadDataSharedDirty:
							cacheStatus = AXP_21264_CACHE_DIRTY_SHARED;
							break;

						/*
						 * The cache block is filled and marked dirty.
						 */
						case ReadDataDirty:
							cacheStatus = AXP_21264_CACHE_DIRTY;
							break;

						/*
						 * The cache block access was to NXM address space. The
						 * 21264 delivers an all-ones pattern to any load
						 * command and evicts the block from the cache (with
						 * associated victim processing). The cache block is
						 * marked invalid.
						 */
						case ReadDataError:
							/*
							 * TODO:	Need to evict the block associated with
							 *			the pa from the Bcache, Dcache and/or
							 *			Icache.
							 */
							break;

						/*
						 * Both SysDc responses are illegal for read commands.
						 */
						case ChangeToDirtySuccess:
						case ChangeToDirtyFail:
						default:
							/* TODO: What should we do in this case */
							break;
					}

					if ((maf->type == Istream) && (error == false))
						AXP_21264_Ibox_UpdateIcache(
										cpu,
										maf->pa,
										&sysData[bufIndex],
										cacheStatus);
					else if (error == false)
						AXP_21264_Mbox_UpdateDcache(
										cpu,
										maf->lqSqEntry[ii],
										&sysData[bufIndex],
										cacheStatus);
					if (maf->type == LDx)
						AXP_21264_Mbox_CboxCompl(
										cpu,
										maf->lqSqEntry[ii],
										NULL,
										0,
										error);
					ii++;
				}
			}

			/*
			 * Rdiox
			 */
			else
			{

				/* ReadBytes (maf->dataLen = 1) */
				/* ReadBytes (maf->dataLen = 2) */
				/* ReadLWs (maf->dataLen = 4) */
				/* ReadQWs (maf->dataLen = 8) */
				dataLen = maf->dataLen;
				while (curPtr != -1)
				{
					bufIndex = AXP_MaskGet(&curPtr, maf->mask, dataLen);
					switch(sysDc)
					{

						/*
						 * The 21264 delivers the data block, independent of
						 * its status, to waiting load instructions and does
						 * not cache the block in the 21264 cache system.
						 */
						case ReadData:
						case ReadDataDirty:
						case ReadDataShared:
						case ReadDataSharedDirty:
							AXP_21264_Mbox_CboxCompl(
											cpu,
											maf->lqSqEntry[ii],
											sysData,
											dataLen,
											error);
							break;

						/*
						 * The cache block access was to NXM address space. The
						 * 21264 delivers an all-ones pattern to any load
						 * command and does not cache the block in the 21264
						 * cache system.
						 */
						case ReadDataError:
							AXP_21264_Mbox_CboxCompl(
											cpu,
											maf->lqSqEntry[ii],
											NULL,
											0,
											error);
							break;

						default:
							break;
					}
					ii++;
				}
			}
			break;

		/*
		 * RdBlkModx
		 */
		case STx:
		case STx_C:
			/* cmd = ReadBlkMod (data length = 64) */
			dataLen = AXP_DCACHE_DATA_LEN;
			while (curPtr != -1)
			{
				bufIndex = AXP_MaskGet(&curPtr, maf->mask, dataLen);
				switch(sysDc)
				{

					/*
					 * The cache block is filled and marked with a non-writable
					 * status. If the store instruction that generated the
					 * RdBlkModx command is still active (not killed), the
					 * 21264 will retry the instruction, generating the
					 * appropriate ChangeToDirty command. Succeeding store
					 * commands cannot update the block without external
					 * reference.
					 */
					case ReadData:
						cacheStatus = AXP_21264_CACHE_CLEAN;
						break;

					case ReadDataShared:
						cacheStatus = AXP_21264_CACHE_SHARED;
						break;

					case ReadDataSharedDirty:
						cacheStatus = AXP_21264_CACHE_DIRTY_SHARED;
						break;

					/*
					 * The 21264 performs a normal fill response, and the cache
					 * block becomes writable.
					 */
					case ReadDataDirty:
						cacheStatus = AXP_21264_CACHE_DIRTY;
						break;

					/*
					 * Both SysDc responses are illegal for read/modify
					 * commands.
					 */
					case ChangeToDirtySuccess:
					case ChangeToDirtyFail:
					default:
						/* TODO: What should we do in this case */
						break;

					/*
					 * The cache block command was to NXM address space. The
					 * 21264 delivers an all-ones pattern to any dependent load
					 * command, forces a fail action on any pending store
					 * commands to this block, and any store to this block is
					 * not retried. The Cbox evicts the cache block from the
					 * cache system (with associated victim processing). The
					 * cache block is marked invalid.
					 */
					case ReadDataError:
						/*
						 * TODO:	Need to evict the block associated with
						 *			the pa from the Bcache, Dcache and/or
						 *			Icache.
						 */
						break;
				}
				if (error == false)
					AXP_21264_Mbox_UpdateDcache(
									cpu,
									maf->lqSqEntry[ii],
									&sysData[bufIndex],
									cacheStatus);
				AXP_21264_Mbox_CboxCompl(
								cpu,
								maf->lqSqEntry[ii],
								NULL,
								0,
								error);
				ii++;
			}
			break;

		/*
		 * ChxToDirty
		 *
		 * NOTE:	STx_C instructions always use the STxCCHangeToDirty
		 * 			command. So, we don't need to handle the STx_C processing
		 * 			described in this particular case.
		 * TODO:	We are not expecting data at this point.  I'm not sure if
		 *			this is even possible.  We need to look into this.
		 */
		case STxChangeToDirty:
			dataLen = AXP_DCACHE_DATA_LEN;
			while (curPtr != -1)
			{
				bufIndex = AXP_MaskGet(&curPtr, maf->mask, dataLen);
				switch(sysDc)
				{

					/*
					 * The original data in the Dcache is replaced with the
					 * filled data. The block is not writable, so the 21264
					 * will retry the store instruction and generate another
					 * ChxToDirty class command. To avoid a potential live-lock
					 * situation, the STC_ENABLE CSR bit must be set. Any STx_C
					 * instruction to this block is forced to fail. In
					 * addition, a Shared/Dirty response causes the 21264 to
					 * generate a victim for this block upon eviction.
					 */
					case ReadData:
						cacheStatus = AXP_21264_CACHE_CLEAN;
						break;

					case ReadDataShared:
						cacheStatus = AXP_21264_CACHE_SHARED;
						break;

					case ReadDataSharedDirty:
						cacheStatus = AXP_21264_CACHE_DIRTY_SHARED;
						break;

					/*
					 * The data in the Dcache is replaced with the filled data.
					 * The block is writable, so the store instruction that
					 * generated the original command can update this block.
					 * Any STx_C instruction to this block is forced to fail.
					 * In addition, the 21264 generates a victim for this block
					 * upon eviction.
					 */
					case ReadDataDirty:
						cacheStatus = AXP_21264_CACHE_DIRTY;
						break;

					/*
					 * Impossible situation. The block must be cached to
					 * generate a ChxToDirty command. Caching the block is not
					 * possible because all NXM fills are filled non-cached.
					 */
					case ReadDataError:
					default:
						/* TODO: What should we do in this case */
						cacheStatus = AXP_21264_CACHE_MISS;
						break;

					/*
					 * Normal response. ChangeToDirtySuccess makes the block
					 * writable. The 21264 retries the store instruction and
					 * updates the Dcache. Any STx_C instruction associated
					 * with this block is allowed to succeed.
					 */
					case ChangeToDirtySuccess:
						cacheStatus = AXP_21264_CACHE_DIRTY;
						break;

					/*
					 * The MAF entry is retired. Any STx_C instruction
					 * associated with the block is forced to fail. If a STx
					 * instruction generated this block, the 21264 retries and
					 * generates either a RdBlkModx (because the reference that
					 * failed the ChangeToDirty also invalidated the cache by
					 * way of an invalidating probe) or another ChxToDirty
					 * command.
					 */
					case ChangeToDirtyFail:
						error = true;
						cacheStatus = AXP_21264_CACHE_MISS;
						break;
				}
				if (cacheStatus != AXP_21264_CACHE_MISS)
					AXP_21264_Mbox_UpdateDcache(
									cpu,
									maf->lqSqEntry[ii],
									&sysData[bufIndex],
									cacheStatus);
				AXP_21264_Mbox_CboxCompl(
								cpu,
								maf->lqSqEntry[ii],
								NULL,
								0,
								error);
				ii++;
			}
			break;

		/*
		 * STCChangeToDirty
		 */
		case STxCChangeToDirty:
			/* cmd = STCChangeToDirty (data length = 0) */
			while (maf->lqSqEntry[ii] != 0)
			{
				switch(sysDc)
				{

					/*
					 * All fill and ChangeToDirtyFail responses will fail the
					 * STx_C requirements.
					 */
					case ReadDataError:
					case ChangeToDirtyFail:
					case ReadData:
					case ReadDataDirty:
					case ReadDataShared:
					case ReadDataSharedDirty:
						error = true;
						break;

					/*
					 * The STx_C instruction succeeds.
					 */
					case ChangeToDirtySuccess:
						AXP_21264_Mbox_UpdateDcache(
										cpu,
										maf->lqSqEntry[ii],
										NULL,
										AXP_21264_CACHE_DIRTY);
						break;

					default:
						break;
				}
				AXP_21264_Mbox_CboxCompl(
								cpu,
								maf->lqSqEntry[ii],
								NULL,
								0,
								error);
				ii++;
			}
			break;

		/*
		 * InvalToDirty
		 */
		case WH64:
			/* cmd = InvalToDirty (data length = 0) */
			while (maf->lqSqEntry[ii] != 0)
			{
				switch(sysDc)
				{

					/*
					 * The block is not writable, so the 21264 will retry the
					 * WH64 instruction and generate a ChxToDirty command.
					 */
					case ReadData:
					case ReadDataShared:
					case ReadDataSharedDirty:
						break;

					/*
					 * The 21264 doesn’t send InvalToDirty commands off-chip
					 * speculatively.  This NXM condition is a hard error.
					 * Systems should perform a machine check.
					 */
					case ReadDataError:
						break;

					/*
					 * The block is writable. Done.
					 *
					 * TODO:	Can data be returned on a WH64 instruction?
					 */
					case ReadDataDirty:
					case ChangeToDirtySuccess:
						AXP_21264_Mbox_UpdateDcache(
										cpu,
										maf->lqSqEntry[ii],
										NULL,
										AXP_21264_CACHE_DIRTY);
						break;

					/*
					 * Illegal. InvalToDirty instructions must provide a cache
					 * block.
					 */
					case ChangeToDirtyFail:
					default:
						break;
				}
				AXP_21264_Mbox_CboxCompl(
								cpu,
								maf->lqSqEntry[ii],
								NULL,
								0,
								error);
				ii++;
			}
			break;

		/*
		 * Evict
		 */
		case ECB:
			/* Evict (data length = 0) */
			/* sysDc = ChangeToDirtyFail */

			/*
			 * TODO:	Need to evict the block associated with the pa from the
			 * 			Bcache, Dcache and/or Icache.
			 */

			/*
			 * Retiring the MAF entry is the only legal response.
			 */
			break;

		/*
		 * MB
		 */
		case MemoryBarrier:
			/* Memory Barrier (data length = 0) */
			/* sysDc = MBDone */
			AXP_21264_Mbox_CboxCompl(
							cpu,
							maf->lqSqEntry[0],
							NULL,
							0,
							false);
			break;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21265_Check_MAFAddrSent
 *	This function is called to determine if there is an MAF entry for the
 *	indicated physical address that is also one of the change to Dirty types.
 *	If so, then the MAF entry number will be sent to the System as part of a
 *	ProbeResponse.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	pa:
 *		A value representing the physical address associated with this record.
 *
 * Output Parameters:
 *	entry:
 *		A pointer to a unsigned char to receive the MAF entry, if matched.
 *
 * Return Values:
 *	true:	A match was found and needs to be sent to the system.
 *	false:	A match was not found.
 */
bool AXP_21265_Check_MAFAddrSent(AXP_21264_CPU *cpu, u64 pa, u8 *entry)
{
	bool	retVal = false;
	int		ii;
	int		end, start1, start2 = -1, end1, end2;

	if (cpu->mafTop > cpu->mafBottom)
	{
		start1 = cpu->mafTop;
		end1 = AXP_21264_MAF_LEN - 1;
		start2 = 0;
		end2 = cpu->mafBottom;
	}
	else
	{
		start1 = cpu->mafTop;
		end1 = cpu->mafBottom;
	}

	/*
	 * Search through the list to find the first entry that is in-use with the
	 * same physical address and one of the change to dirty types.  If so, then
	 * we have what we are looking, so return this entry back to the caller.
	 */
	ii = start1;
	end = end1;
	while ((ii <= end) && (retVal == false))
	{
		if ((cpu->maf[ii].valid == true) &&
			(cpu->maf[ii].pa == pa) &&
			((cpu->maf[ii].type == STxChangeToDirty) ||
			 (cpu->maf[ii].type == STxCChangeToDirty)))
		{
			*entry = ii;
			retVal = true;
		}
		if ((retVal == false) && (start2 != -1) && (ii = end))
		{
			ii = start2;
			end = end2;
			start2 = -1;
		}
		else
			ii++;
	}

	/*
	 * Return what we found back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Add_MAF_Mem
 *	This function is called to add an Miss Address File (MAF) entry for a
 *	memory reference on to the queue for processing.
 *
 *	NOTE:	The Cbox Interface mutex must be locked prior to calling this
 *			function.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	type:
 *		An enumerated value for the type of MAF entry to add.
 *	pa:
 *		A value representing the physical address associated with this record.
 *	lqSqEntry:
 *		A value indicating the entry within the Mbox's LQ or SQ that is
 *		associated with this MAF.  This will be used to inform the Mbox when
 *		the MAF has been processed.
 *	dataLen:
 *		A value indicating the length of data that missed the caches.  This is
 *		used during the merging process.
 *	shared:
 *		A boolean value used to indicate that the MAF entry is part of a store
 *		where the cache entry needs to be changed to dirty and not shared.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	true:	A new MAF needs to be allocated.
 *	false:	The request to add an MAF to the queue has been merged with an
 *			existing one.  Nothing else to do.
 */
bool AXP_21264_Add_MAF_Mem(
		AXP_21264_CPU *cpu,
		AXP_CBOX_MAF_TYPE type,
		u64 pa,
		i8 lqSqEntry,
		int dataLen,
		bool shared)
{
	AXP_21264_CBOX_MAF	*maf = NULL;
	u64					paEnd;
	int					ii;
	int					end, start1, start2 = -1, end1, end2;
	bool				retVal = false;

	/*
	 * HRM 2.9 MAF Memory Address Space Merging Rules
	 *
	 * Because all memory transactions are to 64-byte blocks, efficiency is
	 * improved by merging several small data transactions into a single larger
	 * data transaction.
	 * Table 2–9 lists the rules the 21264 uses when merging memory
	 * transactions into 64-byte naturally aligned data block transactions.
	 * Rows represent the merged instruction in the MAF and columns represent
	 * the new issued transaction.
	 *
	 * Table 2–9 MAF Merging Rules
	 * -------	-----	-----	-----	-----	-----	--------
	 * MAF/New	LDx		STx		STx_C	WH64	ECB		Istream
	 * -------	-----	-----	-----	-----	-----	--------
	 * LDx 		Merge	—		—		—		—		—
	 * STx		Merge	Merge	—		—		—		—
	 * STx_C	—		—		Merge	—		—		—
	 * WH64		—		—		—		Merge	—		—
	 * ECB		—		—		—		—		Merge	—
	 * Istream	—		—		—		—		—		Merge
	 * -------	-----	-----	-----	-----	-----	--------
	 *
	 * In summary, Table 2–9 shows that only like instruction types, with the
	 * exception of load instructions merging with store instructions, are
	 * merged.
	 */
	if (cpu->mafTop > cpu->mafBottom)
	{
		start1 = cpu->mafTop;
		end1 = AXP_21264_MAF_LEN - 1;
		start2 = 0;
		end2 = cpu->mafBottom;
	}
	else
	{
		start1 = cpu->mafTop;
		end1 = cpu->mafBottom;
	}

	/*
	 * Search through the list to find the first entry that can be processed.
	 * We do this test in 3 stages:
	 *
	 *		1)	If the MAF is in-use and not completed
	 *		2)	If the MAF type matches the new type, or we have a store and
	 *			are doing a load
	 *		3)	If the 64-byte block of the physical address includes the all
	 *			the bytes for the data we are reading/writing.
	 */
	ii = start1;
	end = end1;
	while ((ii <= end) && (maf == NULL))
	{
		if ((cpu->maf[ii].type != MAFNotInUse) &&
			(cpu->maf[ii].ioReq == false) &&
			(cpu->maf[ii].complete == false))
		{
			if ((cpu->maf[ii].type == type) ||
				((cpu->maf[ii].type == STx) && (type == LDx)))
			{
				paEnd = cpu->maf[ii].pa + cpu->maf[ii].bufLen;
				if ((paEnd <= pa) &&
					((pa + dataLen) <= (cpu->maf[ii].pa + AXP_21264_SIZE_QUAD)))
					maf = &cpu->maf[ii];
			}
		}
		if ((maf == NULL) && (ii == end) && (start2 != -1))
		{
			ii = start2;
			end = end2;
			start2 = -1;
		}
		else
			ii++;
	}

	if (maf != NULL)
	{
		bool done = false;

		maf->bufLen = (pa + dataLen) - maf->pa;
		AXP_MaskSet(&maf->mask, maf->pa, pa, dataLen);
		for (ii = 0; ((ii < AXP_21264_MBOX_MAX) && (done == false)); ii++)
		{
			if (cpu->maf[cpu->mafBottom].lqSqEntry[ii] == 0)
			{
				cpu->maf[cpu->mafBottom].lqSqEntry[ii] = lqSqEntry;
				done = true;
			}
		}
	}
	else
		retVal = true;
	return(retVal);
}

/*
 * AXP_21264_Add_MAF_IO
 *	This function is called to add an Miss Address File (MAF) entry for an
 *	I/O reference on to the queue for processing.
 *
 *	NOTE:	The Cbox Interface mutex must be locked prior to calling this
 *			function.  Also, this function will not be called for Byte/Word
 *			read requests (these are never merged, so we always allocate a new
 *			entry).
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	type:
 *		An enumerated value for the type of MAF entry to add.
 *	pa:
 *		A value representing the physical address associated with this record.
 *	lqSqEntry:
 *		A value indicating the entry within the Mbox's LQ or SQ that is
 *		associated with this MAF.  This will be used to inform the Mbox when
 *		the MAF has been processed.
 *	dataLen:
 *		A value indicating the length of data that missed the caches.  This is
 *		used during the merging process.
 *	shared:
 *		A boolean value used to indicate that the MAF entry is part of a store
 *		where the cache entry needs to be changed to dirty and not shared.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	true:	A new MAF needs to be allocated.
 *	false:	The request to add an MAF to the queue has been merged with an
 *			existing one.  Nothing else to do.
 */
bool AXP_21264_Add_MAF_IO(
		AXP_21264_CPU *cpu,
		AXP_CBOX_MAF_TYPE type,
		u64 pa,
		i8 lqSqEntry,
		int dataLen,
		bool shared)
{
	AXP_21264_CBOX_MAF	*maf = NULL;
	u64					paEnd;
	int					ii;
	int					end, start1, start2 = -1, end1, end2;
	int					maxLen;
	bool				retVal = false;

	if ((cpu->csr.ThirtyTwoByteIo == 1) && (dataLen == LONG_LEN))
		maxLen = AXP_21264_SIZE_LONG;
	else
		maxLen = AXP_21264_SIZE_QUAD;

	/*
	 * HRM 2.8.2 I/O Address Space Load Instructions
	 *
	 * The Mbox allocates a new MAF entry to an I/O load instruction and
	 * increases I/O bandwidth by attempting to merge I/O load instructions in
	 * a merge register. Table 2–7 shows the rules for merging data. The
	 * columns represent the load instructions replayed to the MAF while the
	 * rows represent the size of the load in the merge register.
	 *
	 * Table 2–7 Rules for I/O Address Space Load Instruction Data Merging
	 * --------------------		--------------	-------------	-------------
	 * Merge Register/
	 * Replayed Instruction		Load Byte/Word	Load Longword	Load Quadword
	 * --------------------		--------------	-------------	-------------
	 * Byte/Word				No merge		No merge		No merge
	 * Longword					No merge		Merge up to		No merge
	 * 											32 bytes
	 * Quadword					No merge		No merge		Merge up to
	 * 															64 bytes
	 * --------------------		--------------	-------------	-------------
	 *
	 * In summary, Table 2–7 shows some of the following rules.
	 *
	 *	- Byte/word load instructions and different size load instructions are
	 *	  not allowed to merge.
	 *	- A stream of ascending non-overlapping, but not necessarily
	 *	  consecutive, longword load instructions are allowed to merge into
	 *	  naturally aligned 32-byte blocks.
	 *	- A stream of ascending non-overlapping, but not necessarily
	 *	  consecutive, quadword load instructions are allowed to merge into
	 *	  naturally aligned 64-byte blocks.
	 *	- Merging of quadwords can be limited to naturally-aligned 32-byte
	 *	  blocks based on the Cbox WRITE_ONCE chain 32_BYTE_IO field.
	 *	- To minimize latency the I/O register merge window is closed when a
	 *	  timer detects no I/O load instruction activity for 14 cycles, or zero
	 *	  cycles if the last QW/LW of the block is addressed.
	 */
	if (cpu->mafTop > cpu->mafBottom)
	{
		start1 = cpu->mafTop;
		end1 = AXP_21264_MAF_LEN - 1;
		start2 = 0;
		end2 = cpu->mafBottom;
	}
	else
	{
		start1 = cpu->mafTop;
		end1 = cpu->mafBottom;
	}

	/*
	 * Search through all the in-use MAF entries looking for one that can be
	 * merged with the current request for one.
	 */
	ii = start1;
	end = end1;
	while ((ii <= end) && (maf == NULL))
	{
		if ((cpu->maf[ii].type != MAFNotInUse) &&
			(cpu->maf[ii].ioReq == true) &&
			(cpu->maf[ii].complete == false))
		{
			if (cpu->maf[ii].type == type)
			{
				paEnd = cpu->maf[ii].pa + cpu->maf[ii].bufLen;
				if ((paEnd <= pa) &&
					((pa + dataLen) <= (cpu->maf[ii].pa + maxLen)))
					maf = &cpu->maf[ii];
			}
		}
		if ((maf == NULL) && (start2 != -1) && (( ii == end)))
		{
			ii = start2;
			end = end2;
			start2 = -1;
		}
		else
			ii++;
	}

	if (maf != NULL)
	{
		bool done = false;

		maf->bufLen = (pa + dataLen) - maf->pa;
		AXP_MaskSet(&maf->mask, maf->pa, pa, dataLen);
		for (ii = 0; ((ii < AXP_21264_MBOX_MAX) && (done == false)); ii++)
		{
			if (maf->lqSqEntry[ii] == 0)
			{
				maf->lqSqEntry[ii] = lqSqEntry;
				done = true;
			}
		}
	}
	else
		retVal = true;
	return(retVal);
}

/*
 * AXP_21264_Add_MAF
 *	This function is called to add an Miss Address File (MAF) entry on to the
 *	queue for processing.
 *
 *	NOTE:	The Ibox and Mbox call this function. They do this to add a fill
 *			request to the for their associated caches.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	type:
 *		An enumerated value for the type of MAF entry to add.
 *	pa:
 *		A value representing the physical address associated with this record.
 *	lqSqEntry:
 *		A value indicating the entry within the Mbox's LQ or SQ that is
 *		associated with this MAF.  This will be used to inform the Mbox when
 *		the MAF has been processed.
 *	dataLen:
 *		A value indicating the length of data that missed the caches.  This is
 *		used during the merging process.
 *	shared:
 *		A boolean value used to indicate that the MAF entry is part of a store
 *		where the cache entry needs to be changed to dirty and not shared.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Add_MAF(
		AXP_21264_CPU *cpu,
		AXP_CBOX_MAF_TYPE type,
		u64 pa,
		i8 lqSqEntry,
		int dataLen,
		bool shared)
{
	AXP_21264_CBOX_MAF	*maf = NULL;
	int					ii;
	bool				ioRq = AXP_21264_IS_IO_ADDR(pa);
	bool				notMerged;

	/*
	 * Before we do anything, lock the interface mutex to prevent multiple
	 * accessors.
	 */
	pthread_mutex_lock(&cpu->cBoxInterfaceMutex);

	/*
	 * The merging rules are different for I/O reads versus memory reads.  Make
	 * sure we follow the right rules.
	 */
	if (ioRq == false)
	{
		if (type == MemoryBarrier)
			notMerged = true;
		else
			notMerged = AXP_21264_Add_MAF_Mem(
									cpu,
									type,
									pa,
									lqSqEntry,
									dataLen,
									shared);
	}
	else
	{

		/*
		 * Byte/Word I/O reads are not merged.  We need to allocate a new
		 * entry.
		 */
		if ((dataLen == BYTE_LEN) || (dataLen == WORD_LEN))
			notMerged = true;
		else
			notMerged = AXP_21264_Add_MAF_IO(
										cpu,
										type,
										pa,
										lqSqEntry,
										dataLen,
										shared);
	}
	if (notMerged == true)
	{

		/*
		 * Add a record to the next available MAF.
		 */
		if (cpu->maf[cpu->mafBottom].valid == true)
			cpu->mafBottom = (cpu->mafBottom + 1) & 0x07;
		maf = &cpu->maf[cpu->mafBottom];
		maf->type = type;
		maf->pa = pa;
		maf->complete = false;
		maf->lqSqEntry[0] = lqSqEntry;
		for (ii = 1; ii < AXP_21264_MBOX_MAX; ii++)
			maf->lqSqEntry[ii] = 0;
		maf->ioReq = ioRq;
		maf->dataLen = maf->bufLen = dataLen;
		AXP_MaskReset(&maf->mask);
		AXP_MaskSet(&maf->mask, maf->pa, pa, dataLen);
		maf->shared = shared;
		maf->valid = true;
	}

	/*
	 * Let the Cbox know there is something for it to process, then unlock the
	 * mutex so it can.
	 */
	pthread_cond_signal(&cpu->cBoxInterfaceCond);
	pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
	return;
}

/*
 * AXP_21264_Free_MAF
 * 	This function is called to return a previously allocated MAF entry.  It
 * 	does this by setting the valid bit to false and adjusting the mafTop
 * 	index, as necessary.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the MAF to be invalidated.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Free_MAF(AXP_21264_CPU *cpu, u8 entry)
{
	AXP_21264_CBOX_MAF	*maf = &cpu->maf[entry];
	int		ii;
	int 	end, start1, end1, start2 = -1, end2;
	bool	done = false;

	/*
	 * First, clear the valid bit.
	 */
	maf->valid = false;

	/*
	 * We now have to see if we can adjust the top of the queue.
	 */
	if (cpu->mafTop > cpu->mafBottom)
	{
		start1 = cpu->mafTop;
		end1 = AXP_21264_MAF_LEN - 1;
		start2 = 0;
		end2 = cpu->mafBottom;
	}
	else
	{
		start1 = cpu->mafTop;
		end1 = cpu->mafBottom;
	}

	/*
	 * Search through the list to find the first entry that is in-use (valid).
	 */
	ii = start1;
	end = end1;
	while ((ii <= end) && (done == false))
	{
		if (cpu->maf[ii].valid == false)
			cpu->mafTop = (cpu->mafTop + 1) & 0x07;
		else
			done = true;
		if ((done == false) && (start2 != -1) && (ii == end))
		{
			ii = start2;
			end = end2;
			start2 = -1;
		}
		else
			ii++;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}
