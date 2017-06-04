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
 *	This file contains the utility functions needed throughout the Alpha AXP
 *	emulation code.
 *
 * Revision History:
 *
 *	V01.000		29-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		01-Jun-2017	Jonathan D. Belaanger
 *	Added an LRU function to return the least recently used set with a
 *	particular index.
 */
#include "AXP_Utility.h"

/*
 * AXP_LRUAdd
 * 	This function is called to add a particular index/set pair into a Least
 * 	Recently Used (LRU) list.  This list is maintained with the LRU item at
 * 	the top and the Most Recently Used (MRU) at the bottom.
 *
 * Input Parameters:
 * 	lru:
 * 		A pointer to the list of LRU items.
 *	lruSize:
 *		A value indicating the maximum number of items that can be in the list.
 *	lruMax:
 *		A pointer to the value of the last item in the list.
 *	index:
 *		A value of the cache index, of the index/set pair, to be saved as the
 *		MRU.  Index for the Icache is 33 bits long.
 *	set:
 *		A value of the cache set, of the index/set pair, to be saved as the
 *		MRU.
 *
 * Output Parameters:
 * 	lruMax:
 * 		A pointer to receive the updated maximum number of items on the list.
 * 		This only gets updated when the MRU is not already the last item in the
 * 		list.
 *
 * Return Value:
 * 	true:
 * 		If the item is already on the list or successfully added.
 * 	false:
 * 		If there is no more room on the list for another item.  The caller will
 * 		need to evict something to make room.
 */
bool AXP_LRUAdd(AXP_LRU_LIST *lru, u32 lruSize, i32 *lruMax, u64 index, u32 set)
{
	i32 ii, jj;
	bool insert = true;
	bool retVal = true;

	for(ii = 0; ii <= *lruMax; ii++)
	{
		if ((lru[ii].index == index) &&
			(lru[ii].set == set))
		{

			/*
			 * The item we're looking for is already on the list.  We just need
			 * need to move it to the end.  This will force the list to have
			 * the LRU item at the top of the list.  When a cache item needs to
			 * be evicted, the item at the top will be selected.
			 *
			 * NOTE:	If the item is already the least recently used, then
			 * 			there is nothing more to do.
			 */
			if (ii != *lruMax)
			{
				for(jj = ii; jj < *lruMax; jj++)
				{
					lru[jj].index = lru[jj + 1].index;
					lru[jj].set = lru[jj + 1].set;
				}
				*lruMax = *lruMax - 1;
			}
			else
				insert = false;
			break;
		}
	}

	/*
	 * If we still have to insert the item, because it was not already there or
	 * because it was there and not at the end of the list already, then we
	 * need to add the item to the bottom of the list and increment the maxLRU
	 *
	 * NOTE:	If there is no more room at the inn, then we need to evict
	 * 			someone (probably the top of the list), but that is not our
	 * 			choice.  This is because an entry in this list may also have
	 * 			other representative entries in other lists.  For example, an
	 * 			Icache entry, also has an associated ITB entry, which in turn
	 * 			may have other Icache entries, as well as a PTE entry.
	 */
	if (insert == true)
	{
		if (*lruMax < (i32) lruSize)
		{
			*lruMax = *lruMax + 1;
			lru[*lruMax].index = index;
			lru[*lruMax].set = set;
		}
		else
			retVal = false;
	}
	return(retVal);
}

/*
 * AXP_LRURemove
 * 	This function is called to remove a particular index/set pair off of a
 * 	Least Recently Used (LRU) list.  This list is maintained with the LRU item
 * 	at the top and the Most Recently Used (MRU) at the bottom.
 *
 * Input Parameters:
 * 	lru:
 * 		A pointer to the list of LRU items.
 *	lruMax:
 *		A pointer to the value of the last item in the list.
 *	index:
 *		A value of the cache index, of the index/set pair, to be removed from
 *		the list.  Index for the Icache is 33 bits long.
 *	set:
 *		A value of the cache set, of the index/set pair, to be removed rom the
 *		list.
 *
 * Output Parameters:
 * 	lruMax:
 * 		A pointer to receive the updated maximum number of items on the list.
 * 		This does not get updated if the item is not found.
 *
 * Return Value:
 * 	true:
 * 		If the item was successfully removed.
 * 	false:
 * 		If the item was not found.
 */
bool AXP_LRURemove(AXP_LRU_LIST *lru, i32 *lruMax, u64 index, u32 set)
{
	i32 ii, jj;
	bool retVal = false;

	for(ii = 0; ii <= *lruMax; ii++)
	{
		if ((lru[ii].index == index) &&
			(lru[ii].set == set))
		{

			/*
			 * The item we're looking for is on the list, so all we have to do
			 * now is collapse the list on top of it, and decrement the maximum
			 * number of items on the list.
			 */
			for(jj = ii; jj < *lruMax; jj++)
			{
				lru[jj].index = lru[jj + 1].index;
				lru[jj].set = lru[jj + 1].set;
			}
			(*lruMax)--;
			retVal = true;
			break;
		}
	}
	return(retVal);
}

/*
 * AXP_LRUReturn
 * 	This function is called to return the Least Recently Used (LRU) item off
 * 	the list.  This list is maintained with the LRU item at the top and the
 * 	Most Recently Used (MRU) at the bottom.
 *
 * Input Parameters:
 * 	lru:
 * 		A pointer to the list of LRU items.
 *	lruMax:
 *		A value of the last item in the list.
 *
 * Output Parameters:
 *	index:
 *		A pointer to receive the LRU index of the cache, of the index/set pair,
 *		on the list.  Index for the Icache is 33 bits long.
 *	set:
 *		A pointer to receive the LRU set of the cache, of the index/set pair,
 *		on the list.
 *
 * Return Value:
 * 	true:
 * 		If the item was successfully located (the list has items in it).
 * 	false:
 * 		If there are no items on the list.
 */
bool AXP_LRUReturn(AXP_LRU_LIST *lru, i32 lruMax, u64 *index, u32 *set)
{
	bool retVal = false;

	if (lruMax >= 0)
	{
		*index = lru[0].index;
		*set = lru[0].set;
		retVal = true;
	}
	return(retVal);
}

/*
 * AXP_LRUReturnIdx
 * 	This function is called to return the LRU item for a at a particular index.
 *
 * Input Parameters:
 * 	lru:
 * 		A pointer to the list of LRU items.
 *	lruMax:
 *		A value of the last item in the list.
 *	index:
 *		A value of the cache index, of the index/set pair, to be removed from
 *		the list.  Index for the Icache is 33 bits long.
 *
 * Output Parameters:
 *	set:
 *		A pointer to receive the LRU set of the cache, of the index/set pair,
 *		on the list.
 *
 * Return Value:
 * 	true:
 * 		If the item was successfully removed.
 * 	false:
 * 		If the item was not found.
 */
bool AXP_LRUReturnIdx(AXP_LRU_LIST *lru, i32 lruMax, u64 index, u32 *set)
{
	i32 ii;
	bool retVal = false;

	for(ii = 0; ((ii <= lruMax) && (retVal == false)); ii++)
	{
		if (lru[ii].index == index)
		{
			*set = lru[ii].set;
			retVal = true;
			break;
		}
	}
	return(retVal);
}
