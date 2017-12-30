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
 *	Bcache functionality of the Cbox.
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
 * AXP_21264_Bcache_Evict
 *	This function is called to evict a Bcache block.  If the block is dirty,
 *	then it is written out to memory.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Bcache_Evict(AXP_21264_CPU *cpu, u64 pa)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);
	u8		entry;

	/*
	 * If the block is valid, then if it is dirty, we need to send it to the
	 * System to store back in memory.
	 */
	if (valid == true)
	{
		if (cpu->bTag[index].dirty == true)
			entry = AXP_21264_Add_VDB(
							cpu,
							toMemory,
							pa,
							cpu->bCache[index],
							false,
							true);	/* TODO: mutex setting needs? */
	}

	/*
	 * We always clear the valid bit, because we may be involved in a Bcache
	 * Flush operation.
	 */
	cpu->bTag[index].valid = false;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Bcache_Flush
 *	This function is called to Flush everything from the Bcache.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Bcache_Flush(AXP_21264_CPU *cpu)
{
	int		ii;
	int		bCacheArraySize;

	/*
	 * First we need to determine the size of the Bcache.
	 */
	switch (cpu->csr.BcSize)
	{
		case AXP_BCACHE_1MB:
			bCacheArraySize = AXP_21264_1MB / AXP_BCACHE_BLOCK_SIZE;
			break;

		case AXP_BCACHE_2MB:
			bCacheArraySize = AXP_21264_2MB / AXP_BCACHE_BLOCK_SIZE;
			break;

		case AXP_BCACHE_4MB:
			bCacheArraySize = AXP_21264_4MB / AXP_BCACHE_BLOCK_SIZE;
			break;

		case AXP_BCACHE_8MB:
			bCacheArraySize = AXP_21264_8MB / AXP_BCACHE_BLOCK_SIZE;
			break;

		case AXP_BCACHE_16MB:
			bCacheArraySize = AXP_21264_16MB / AXP_BCACHE_BLOCK_SIZE;
			break;
	}

	/*
	 * NOTE: We only have to mark the array entry invalid in the Bcache Tag
	 * array.  What is in the Bcache Block array is only relevant when the Tag
	 * array indicates that it is valid.  When the valid flag is set, the data
	 * was just written to the block array.
	 */
	for (ii = 0; ii < bCacheArraySize; ii++)
		 AXP_21264_Bcache_Evict(cpu, cpu->bTag[index].pa);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Bcache_Valid
 *	This function is called to determine if a physical address has a valid
 *	location within the Bcache.  We don't actually look in the Bcache, but do
 *	look in the Bcache Tag array.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	false:	The physical address is not in the Bcache.
 *	true:	The physical address is in the Bcache.
 */
bool AXP_21264_Bcache_Valid(AXP_21264_CPU *cpu, u64 pa)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);

	/*
	 * If the entry at the index, based on the physical address, is valid and
	 * the tag associated with that entry matches the tag out of the physical
	 * address, then we have a valid entry.  Otherwise, we do not.
	 */
	return((cpu->bTag[index].valid == true) &&
		   (cpu->bTag[index].tag == AXP_BCACHE_TAG(cpu, pa)));
}

/*
 * AXP_21264_Bcache_Status
 * 	This function is called to return the status of a Bcache entry (if valid).
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Values:
 *	An unsigned 32-bit location to receive a masked value with the following
 *	bits set as appropriate:
 *			AXP_21264_CACHE_MISS
 *			AXP_21264_CACHE_HIT
 *			AXP_21264_CACHE_DIRTY
 *			AXP_21264_CACHE_SHARED
 *
 * NOTE:	This is called from the Mbox and Cbox, which already locked bCache
 * 			mutex.
 */
u32 AXP_21264_Bcache_Status(AXP_21264_CPU *cpu, u64 pa)
{
	u8		retVal = AXP_21264_CACHE_MISS;
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * If there is no valid record, then this is a MISS.  Nothing else we need
	 * to do.
	 */
	if (valid == true)
	{
		u32 index = AXP_BCACHE_INDEX(cpu, pa);

		/*
		 * Set the return value based on the fact that we hit in the Bcache and
		 * the status bits associated with that entry.
		 */
		retVal = AXP_21264_CACHE_HIT;
		if (cpu->bTag[index].dirty == true)
			retVal |= AXP_21264_CACHE_DIRTY;
		if (cpu->bTag[index].shared == true)
			retVal |= AXP_21264_CACHE_SHARED;
	}
	return(retVal);
}

/*
 * AXP_21264_Bcache_Read
 *	This function is called to read the contents of a Bcache location and
 *	return them to the caller.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	data:
 *		A pointer to a properly size buffer (64 bytes) to receive the contents
 *		of the Bcache, if found (valid).
 *
 * Return Values:
 *	false:	The physical address is not in the Bcache.
 *	true:	The physical address is in the Bcache.
 */
bool AXP_21264_Bcache_Read(
				AXP_21264_CPU *cpu,
				u64 pa,
				u8 *data,
				bool *dirty,
				bool *shared)
{
	bool	retVal = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * If the physical address is in the Bcache, then we can copy the data to
	 * the caller's buffer.
	 */
	if (retVal == true)
	{
		u32 index = AXP_BCACHE_INDEX(cpu, pa);

		/*
		 * Copy the data.
		 */
		memcpy(
			data,
			cpu->bCache[index],
			AXP_BCACHE_BLOCK_SIZE);

		/*
		 * If requested, return the dirty and shared bits.
		 */
		if (dirty != NULL)
			*dirty = cpu->bTag[index].dirty;
		if (shared != NULL)
			*shared = cpu->bTag[index].shared;
	}

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Bcache_Write
 *	This function is called to write the contents of a buffer into a Bcache
 *	location.  This function always succeeds.  If a location is already in use
 *	and we are updating it, then we will also write the values to memory.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *	data:
 *		A pointer to a properly size buffer (64 bytes) containing the data to
 *		be written to the Bcache.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Bcache_Write(
				AXP_21264_CPU *cpu,
				u64 pa,
				u8 *data)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * Before we go to far, see if we need to evict the current buffer.
	 */
	if ((valid == false) && (cpu->bTag[index].valid == true))
		AXP_21264_Bcache_Evict(cpu, pa);

	/*
	 * Now copy the buffer into the Bcache, then update the associated tag
	 * with the tag value and setting the valid bit.
	 */
	memcpy(cpu->bCache[index], data, AXP_BCACHE_BLOCK_SIZE);
	cpu->bTag[index].tag = AXP_BCACHE_TAG(cpu, pa);
	cpu->bTag[index].pa = pa;
	cpu->bTag[index].valid = true;

	/*
	 * If the buffer was already valid, then we need to indicate that this
	 * Bcache block is dirty.  This way, when it is evicted, it will get
	 * written out to memory.
	 */
	if (valid == true)
		cpu->bTag[index].dirty = true;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Bcache_SetShared
 *	This function is called to set the shared bit for a Bcache block.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Bcache_SetShared(
				AXP_21264_CPU *cpu,
				u64 pa)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * If the Bcache block is valid, then indicate that it is shared with
	 * another agent.
	 */
	if (valid == true)
		cpu->bTag[index].shared = true;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Bcache_ClearShared
 *	This function is called to clear the shared bit for a Bcache block.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Bcache_ClearShared(
				AXP_21264_CPU *cpu,
				u64 pa)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * If the Bcache block is valid, then indicate that it is shared with
	 * another agent.
	 */
	if (valid == true)
		cpu->bTag[index].shared = false;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Bcache_SetDirty
 *	This function is called to set the dirty bit for a Bcache block.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Bcache_SetDirty(
				AXP_21264_CPU *cpu,
				u64 pa)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * If the Bcache block is valid, then indicate that it is shared with
	 * another agent.
	 */
	if (valid == true)
		cpu->bTag[index].dirty = true;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Bcache_ClearDirty
 *	This function is called to clear the dirty bit for a Bcache block.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Bcache_ClearDirty(
				AXP_21264_CPU *cpu,
				u64 pa)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * If the Bcache block is valid, then indicate that it is shared with
	 * another agent.
	 */
	if (valid == true)
		cpu->bTag[index].dirty = false;

	/*
	 * Return back to the caller.
	 */
	return;
}
