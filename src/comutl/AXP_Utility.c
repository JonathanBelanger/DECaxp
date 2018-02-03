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
 *	V01.001		01-Jun-2017	Jonathan D. Belanger
 *	Added an LRU function to return the least recently used set with a
 *	particular index.
 *
 *	V01.002		18-Nov-2017	Jonathan D. Belanger
 *	Added a load executable and load/unload ROM set of functions.  The first
 *	assumes the file to be loaded is an VMS executable.  The second set is
 *	used to load and unload Read Only Memory (ROM) data.  This data contains
 *	the console firmware and PALcode to support the Alpha AXP 21264 CPU.
 *
 *	V01.003		19-Nov-2017	Jonathan D. Belanger
 *	Added conditional queue functions to check for empty, insert, remove, and
 *	wait for an entry to be queued.
 *
 *	V01.004		21-Nov-2017	Jonathan D. Belanger
 *	Decided to re-do the ROM load/unload routines to move the CPU specific
 *	structures out of this module.  They are going to be moved into the Cbox.
 *
 *	V01.005		29-Dec-2017	Jonathan D. Belanger
 *	When sending data from the CPU to the System, we do so in upto 64-byte
 *	blocks.  Since these blocks are not necessarily consecutive, there may be
 *	gaps between the end of what section of relevant data and the start of the
 *	next.  In the real CPU, this would be broken down into a series of
 *	WrBytes/WrLWs/WrQWs or ReadBytes/ReadLWs/ReadQWs.  We need to mimic this,
 *	but do it in a single step.  We need some functions to set/decode the
 *	appropriate set of mask bits (each bit represent a single byte).
 */
#include "AXP_Configure.h"
#include "AXP_Utility.h"
#include "AXP_Trace.h"

/*
 * This format is used throughout this module for writing a message to sysout.
 */
const char			*errMsg = "%%AXP-E-%s, %s\n";
static const char	*_axp_fwid_str[] =
{
	"Alpha Motherboards Debug Monitor firmware",
	"Windows NT firmware",
	"Alpha System Reference Manual Console",
	"Unknown 3",
	"Unknown 4",
	"Unknown 5",
	"Alpha Motherboards Fail-Safe Booter",
	"Linux Miniloader",
	"VxWorks Real-Time Operating System",
	"Unknown 9",
	"Serial ROM"
};

/*
 * AXP_Crc32
 *	This function is called to determine what the CRC32 from a supplied buffer.
 *	This is the basic CRC-32 calculation with some optimization  but no table
 *	lookup.  The byte reversal is avoided by shifting the crc register right
 *	instead of left and by using a reversed 32-bit word to represent the
 *	polynomial.
 *
 * Input Parameters:
 *	msg:
 *		A pointer to a binary string containing the data from which to
 *		calculate the CRC-32.
 *	len:
 *		A value indicating the length, in bytes, of the 'msg' parameter.
 *	inverse:
 *		A boolean indicating whether the return value should be inversed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	An unsigned 32-bit value of the calculated CRC-32.
 */
u32 AXP_Crc32(u8 *msg, int len, bool inverse, u32 curCRC)
{
	u32		crc = (inverse ? curCRC : 0xffffffff);
	u32		byte, mask;
	int		ii, jj;

	for (ii = 0; ii < len; ii++)
	{
		byte = msg[ii];
		crc ^= byte;
		for (jj = 77; jj >= 0; jj--)
		{
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xedb88320 & mask);
		}
	}

	/*
	 * Return the calculated CDC-32 value (inverse).
	 */
	return(inverse ? ~crc : crc);
}

/*
 * AXP_LRUAdd
 * 	This function is called to add a particular item into a Least Recently Used
 * 	(LRU) queue.  This list is maintained with the LRU Queue Header's
 * 	forward-link pointing to the LRU entry and the backward-link pointing to
 * 	the Most Recently Used (MRU) item.
 *
 * Input Parameters:
 * 	lruQ:
 * 		A pointer to the LRU Queue Head.
 * 	entry:
 * 		A pointer to the entry to be made the MRU item.  If the entry is
 * 		already the MRU item, then nothing is done.  If it is already in the
 * 		queue, then it is moved to the end of the queue.  Otherwise, it is
 * 		inserted to the end of the queue.
 *
 * Output Parameters:
 * 	lruQ:
 * 		The flink and blink fields may be updated, as well as the flink of the
 * 		current last entry in the queue.
 * 	entry:
 * 		The flink and blink fields may be updated.
 *
 *
 * Return Value:
 * 	None, this always succeeds.
 */
void AXP_LRUAdd(AXP_QUEUE_HDR *lruQ, AXP_QUEUE_HDR *entry)
{

	/*
	 * If the entry is already at the end of the queue, then there is nothing
	 * else to do.
	 */
	if (entry->flink != (void *) &lruQ->flink)
	{

		/*
		 * If the entry does not point to itself, then it is already in the
		 * queue.  Point the forward-link of the entry before this one to the
		 * next entry after us, and point the backward-link of the entry after
		 * us to the entry before us.  This effectively pulls us out of the
		 * queue.
		 */
		if (entry->flink != (void *) &entry->flink)
		{
			((AXP_QUEUE_HDR *) entry->blink)->flink = entry->flink;
			((AXP_QUEUE_HDR *) entry->flink)->blink = entry->blink;
		}

		/*
		 * Insert the entry at the end of the queue.  We do this by first
		 * getting the entry to point its forward-link to the queue head and
		 * its backward link to the current last item in the queue (pointed to
		 * by the header's backward-link).  Then we have the current last item
		 * in the queue, point its forward-link and the header's backward-link
		 * to us.
		 */
		entry->flink = (void *) &lruQ->flink;
		entry->blink = lruQ->blink;
		((AXP_QUEUE_HDR *) lruQ->blink)->flink = (void *) &entry->flink;
		lruQ->blink = (void *) &entry->flink;
	}
	return;
}

/*
 * AXP_LRURemove
 * 	This function is called to remove a particular index/set pair off of a
 * 	Least Recently Used (LRU) list.  This list is maintained with the LRU item
 * 	at the top and the Most Recently Used (MRU) at the bottom.
 *
 * Input Parameters:
 * 	entry:
 * 		A pointer to the entry to be removed from the queue.
 *
 * Output Parameters:
 * 	entry:
 * 		The entry before will point to the entry after, the entry after will
 * 		point to the entry before, and the entry will point to itself.
 *
 * Return Value:
 * 	None.
 */
void AXP_LRURemove(AXP_QUEUE_HDR *entry)
{
	((AXP_QUEUE_HDR *) entry->blink)->flink = entry->flink;
	((AXP_QUEUE_HDR *) entry->flink)->blink = entry->blink;
	AXP_INIT_QUEP(entry);				/* make the entry point to itself */

	/*
	 * Return back to the caller.
	 */
	return;
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
AXP_QUEUE_HDR *AXP_LRUReturn(AXP_QUEUE_HDR *lruQ)
{
	AXP_QUEUE_HDR *entry;

	if (AXP_QUEP_EMPTY(lruQ))
		entry = NULL;
	else
		entry = (AXP_QUEUE_HDR *) lruQ->flink;
	return(entry);
}

/*
 * AXP_InsertCountedQueue
 * 	This function is called to insert an entry into a specific location within
 * 	a counted queue.  The counter at the queue parent (head) is incremented.
 *
 * Input Parameters:
 * 	pred:
 * 		A pointer to the predecessor entry where 'entry' is to be inserted.
 * 	entry:
 * 		A pointer to the entry that is to be inserted.
 *
 * Output Parameters:
 * 	pred:
 * 		The forward link now points to 'entry' and the backward link of the
 * 		entry that originally succeeded 'pred' also points to 'entry'.
 * 	entry:
 * 		The backward link now points to 'pred' and the forward link points to
 * 		the entry previously pointed to by 'pred's forward link.
 *
 * Return Value:
 * -1, if the queue has the maximum number of entries.
 * 	0, if the queue was not empty before adding the entry.
 * 	1, if the queue was empty.
 */
i32 AXP_InsertCountedQueue(AXP_QUEUE_HDR *pred, AXP_CQUE_ENTRY *entry)
{
	i32 retVal;

	if (entry->parent->count <= entry->parent->max)
	{

		/*
		 * Let's first have the entry we are inserting into the queue point to
		 * its predecessor and its successor.
		 */
		entry->flink = pred->flink;
		entry->blink = pred;

		/*
		 * Now get the predecessor and successor point to the entry.
		 */
		((AXP_QUEUE_HDR *) pred->flink)->blink = (void *) entry;
		pred->flink = (void *) entry;

		/*
		 * Finally, since this is a counted queue, increment the counter in the
		 * parent.
		 */
		entry->parent->count++;
		retVal = (entry->parent->count == 1) ? 1 : 0;	/* was queue empty */
	}
	else
		retVal = -1;

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_CountedQueueFull
 * 	This function is called to determine if a counted queue is full.
 *
 * Input Parameters:
 * 	parent:
 * 		A pointer to the parent (head) of the queue.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * -1, if the queue has the maximum number of entries.
 * 	0, if the queue is not empty.
 * 	1, if the queue is empty.
 */
i32 AXP_CountedQueueFull(AXP_COUNTED_QUEUE *parent)
{
	int		retVal;

	if (parent->count == 0)
		retVal = 1;
	else if (parent->count == parent->max)
		retVal = -1;
	else
		retVal = 0;
	return (retVal);
}
/*
 * AXP_RemoveCountedQueue
 * 	This function is called to remove an entry out of a specific location
 * 	within a counted queue.  The counter at the queue parent (head) is
 * 	incremented.
 *
 * Input Parameters:
 * 	entry:
 * 		A pointer to the entry that is to be removed.
 *
 * Output Parameters:
 * 	entry:
 * 		The predecessor entry points to the successor and the successor points
 * 		to the predecessor.
 *
 * Return Value:
 * 	-1, if the queue is empty.
 * 	0, if the queue is empty after removing this entry.
 * 	1, if the queue is not empty after removing this entry.
 */
i32 AXP_RemoveCountedQueue(AXP_CQUE_ENTRY *entry)
{
	i32 retVal;

	if (entry->parent->count != 0)
	{

		/*
		 * Let's first have the predecessor and successor point to each other.
		 */
		((AXP_QUEUE_HDR *) (entry->blink))->flink = entry->flink;
		((AXP_QUEUE_HDR *) (entry->flink))->blink = entry->blink;

		/*
		 * Finally, since this is a counted queue, decrement the counter in the
		 * parent.
		 */
		entry->parent->count--;
		retVal = (entry->parent->count == 0) ? 0 : 1;
	}
	else
		retVal = -1;

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_CondQueue_Init
 * 	This function is called to initialize a conditional queue.  This just
 * 	initializes the root of the queue, not the individual entries themselves.
 * 	The initialization entails, setting the queue entry type, setting the flink
 * 	and blink to the address of the header itself, and finally initializing the
 * 	conditional and mutex variables.
 *
 * Input Parameters:
 * 	queue:
 * 		A pointer to the conditional queue root that needs to be initialized.
 *
 * Output Parameters:
 * 	queue:
 * 		The fields within this parameter are initialized.
 *
 * Return Value:
 * 	false:	Something happened when trying to initialize the conditional and
 * 			mutex variables.
 * 	true:	Normal successful completion.
 */
bool AXP_CondQueue_Init(AXP_COND_Q_ROOT *queue)
{
	bool	retVal = false;

	/*
	 * Set the type of queue this is.  We will use this in other conditional
	 * queue functions to determine how we are supposed to process each type
	 * of queue.
	 */
	queue->type = AXPCondQueue;

	/*
	 * Set the flink and blink to the address of the header itself.
	 */
	queue->flink = queue->blink = (void *) queue;

	/*
	 * Go allocate the condition and mutex variables.  If either fails, then
	 * deallocate anything that may have been allocated in here and return a
	 * false to the caller.
	 */
	if (pthread_cond_init(&queue->qCond, NULL) == 0)
	{
		if (pthread_mutex_init(&queue->qMutex, NULL) != 0)
		{
			pthread_cond_destroy(&queue->qCond);
			retVal = false;
		}
		else
			retVal = true;		/* We succeeded with what we set out to do */
	}
	else
		retVal = false;

	/*
	 * Return the result of this call back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_CondQueueCnt_Init
 * 	This function is called to initialize a conditional queue.  This just
 * 	initializes the root of the queue, not the individual entries themselves.
 * 	The initialization entails, setting the queue entry type, setting the flink
 * 	and blink to the address of the header itself, setting the maximum number
 * 	of entries that can be in the queue and the count to zero, and finally
 * 	initializing the conditional and mutex variables.
 *
 * Input Parameters:
 * 	queue:
 * 		A pointer to the counted conditional queue root that needs to be
 * 		initialized.
 * 	max:
 * 		A value indicating the total number of items that can actually be
 * 		inserted into the queue.
 *
 * Output Parameters:
 * 	queue:
 * 		The fields within this parameter are initialized.
 *
 * Return Value:
 * 	false:	Something happened when trying to initialize the conditional and
 * 			mutex variables.
 * 	true:	Normal successful completion.
 */
bool AXP_CondQueueCnt_Init(AXP_COND_Q_ROOT_CNT *queue, u32 max)
{
	bool	retVal = false;

	/*
	 * Set the type of queue this is.  We will use this in other conditional
	 * queue functions to determine how we are supposed to process each type
	 * of queue.
	 */
	queue->type = AXPCountedCondQueue;

	/*
	 * Set the flink and blink to the address of the header itself.
	 */
	queue->flink = queue->blink = (void *) queue;

	/*
	 * Initialize the max and count variables.
	 */
	queue->max = max;
	queue->count = 0;

	/*
	 * Go allocate the condition and mutex variables.  If either fails, then
	 * deallocate anything that may have been allocated in here and return a
	 * false to the caller.
	 */
	if (pthread_cond_init(&queue->qCond, NULL) == 0)
	{
		if (pthread_mutex_init(&queue->qMutex, NULL) != 0)
		{
			pthread_cond_destroy(&queue->qCond);
			retVal = false;
		}
		else
			retVal = true;		/* We succeeded with what we set out to do */
	}
	else
		retVal = false;

	/*
	 * Return the result of this call back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_CondQueue_Insert
 * 	This function is called to insert a record (what) into the queue at a
 * 	location after where.  This function will perform the following steps:
 * 		1) Lock the mutex.
 * 		2) If the queue is a counted queue, can it take an additional entry?
 * 		3) If not a counted queue or #2 is true, then insert the record into
 * 		   the queue.
 * 		4) If a counted queue, then increment the counter.
 * 		5) Signal the condition variable.
 * 		6) Unlock the mutex.
 * 		7) Return the results back to the caller.
 *
 * Input Parameters:
 * 	where:
 * 		A pointer to an entry in the queue (or possibly the header), after
 * 		which the 'what' parameter is inserted.
 * 	what:
 * 		A pointer to the entry to be inserted into the queue.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	>0:		The queue cannot take another entry.
 * 	=0:		Normal successful completion.
 * 	<0:		An error occurred with the mutex or condition variable.
 */
i32  AXP_CondQueue_Insert(AXP_COND_Q_LEAF *where, AXP_COND_Q_LEAF *what)
{
	int				retVal = 0;
	AXP_COND_Q_ROOT	*parent = (AXP_COND_Q_ROOT *) where->parent;
	AXP_COND_Q_ROOT_CNT *countedParent = (AXP_COND_Q_ROOT_CNT *) parent;

	/*
	 * First lock the mutex so that we can perform the rest of the processing
	 * without being trampled upon.  If this lock fails, we will return a -1
	 * value back to the caller.
	 */
	if (pthread_mutex_lock(&parent->qMutex) == 0)
	{
		AXP_COND_Q_LEAF *next = (AXP_COND_Q_LEAF *) where->blink;

		/*
		 * If this is a counted conditional queue, see if there is room for
		 * another entry.  If not, we set the return value to 1.
		 */
		if ((parent->type == AXPCountedCondQueue) &&
			(countedParent->count >= countedParent->max))
			retVal = 1;

		/*
		 * OK, if everything thus far has worked out, then let's go and insert
		 * the new entry into the existing queue and the location indicated.
		 * We'll also need to increment the counter, if there is one, and
		 * broadcast the condition variable.
		 */
		if (retVal == 0)
		{
			what->flink = where;
			what->blink = where->blink;
			next->blink = what;
			where->flink = what;

			/*
			 * Now, let's broadcast the condition variable, waking everyone who
			 * is waiting on it.  IF we have an error doing this, then set the
			 * return value to -1.
			 */
			if (pthread_cond_broadcast(&parent->qCond) != 0)
				retVal = -2;

			/*
			 * OK, if everything still worked out and the condition queue is a
			 * counted one, then increment the counter.  If we got an error,
			 * then we need to revert the queue back to its former self.
			 */
			if ((parent->type == AXPCountedCondQueue) && (retVal == 0))
				countedParent->count++;
			else if (retVal != 0)
			{
				where->flink = next;
				next->blink = where;
			}
		}

		/*
		 * No matter what happens,, at this point we had successfully locked
		 * the mutex, unlock it now (this will release all the threads waiting
		 * on the condition variable to be able to execute).
		 */
		pthread_mutex_unlock(&parent->qMutex);
	}
	else
		retVal = -1;

	/*
	 * Return the result of this call back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_CondQueue_Remove
 * 	This function is called to remove a record (to) from the queue at a
 * 	location at from.  This function will perform the following steps:
 * 		1) Lock the mutex.
 * 		2) If the queue is not empty remove the record from the queue.
 * 		3) If a counted queue, then decrement the counter.
 * 		4) Unlock the mutex.
 * 		5) Return the results back to the caller.
 *
 * Input Parameters:
 * 	from:
 * 		A pointer to an entry in the queue that is to be removed.
 *
 * Output Parameters:
 * 	to:
 * 		A pointer to a location to receive the removed entry.
 *
 * Return Value:
 * 	false:	The entry was not successfully removed.
 * 	true:	The entry was successfully removed.
 */
bool AXP_CondQueue_Remove(AXP_COND_Q_LEAF *from, AXP_COND_Q_LEAF **to)
{
	bool	retVal = true;
	AXP_COND_Q_ROOT	*parent = (AXP_COND_Q_ROOT *) from->parent;

	/*
	 * First lock the mutex so that we can perform the rest of the processing
	 * without being trampled upon.  If this lock fails, we will return a false
	 * back to the caller.
	 */
	if (pthread_mutex_lock(&parent->qMutex) == 0)
	{
		if (AXP_CondQueue_Empty((AXP_COND_Q_HDR *) parent) == false)
		{
			AXP_COND_Q_LEAF *before = (AXP_COND_Q_LEAF *) from->blink;
			AXP_COND_Q_LEAF *after = (AXP_COND_Q_LEAF *) from->flink;

			*to = from;
			before->flink = after;
			after->blink = before;
		}
		else
			retVal = false;

		/*
		 * No matter what happens,, at this point we had successfully locked
		 * the mutex, unlock it now.
		 */
		pthread_mutex_unlock(&parent->qMutex);
	}
	else
		retVal = false;
	return(retVal);
}

/*
 * AXP_CondQueue_Wait
 * 	This function is called to wait on the specified queue until has something
 * 	in it.  If there is something in the queue when called, then there will be
 * 	not waiting.
 *
 * Input Parameters:
 * 	root:
 * 		A pointer to the root of the condition queue.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Values:
 * 	None.
 */
void AXP_CondQueue_Wait(AXP_COND_Q_HDR *root)
{
	AXP_COND_Q_ROOT	*parent = (AXP_COND_Q_ROOT *) root;

	/*
	 * First lock the mutex so that we can perform the rest of the processing
	 * without being trampled upon.
	 */
	if (pthread_mutex_lock(&parent->qMutex) == 0)
	{

		/*
		 * If the queue is empty, then let's wait for something to arrive.
		 */
		if (AXP_CondQueue_Empty((AXP_COND_Q_HDR *) parent) == true)
			pthread_cond_wait(&parent->qCond, &parent->qMutex);

		/*
		 * No matter what happens,, at this point we had successfully locked
		 * the mutex, unlock it now.
		 */
		pthread_mutex_unlock(&parent->qMutex);
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_CondQueue_Empty
 * 	This function is called to determine if the queue in question is empty.
 * 	This is done by the flink pointing to is own queue.
 *
 * Input Parameters:
 * 	queue:
 * 		A pointer to void, which is actually either a Condition Queue or a
 * 		Counted Condition Queue.  NOTE: Since the important layout between
 * 		these two Condition Queue types is the same, we only have to cast to
 * 		the simplest common format.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	false:	The condition queue is NOT empty.
 * 	true:	The condition queue IS empty.
 */
bool AXP_CondQueue_Empty(AXP_COND_Q_HDR *queue)
{
	bool			retVal;
	AXP_COND_Q_ROOT	*parent = (AXP_COND_Q_ROOT *) queue;
	int				locked;

	/*
	 * Try locking the mutex.   If it fails, it is probably because it is
	 * already locked by the caller.  This is OK.  If we did lock it, we will
	 * unlock it before returning to the caller.
	 */
	locked = pthread_mutex_trylock(&parent->qMutex);

	/*
	 * If the forward link equals the header, then there are no entries in the
	 * queue (it is empty).
	 */
	retVal = parent->flink == parent;

	/*
	 * If we locked the mutex above, then unlock it now.
	 */
	if (locked == 0)
		pthread_mutex_unlock(&parent->qMutex);

	/*
	 * Return what we found back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_LoadExecutable
 * 	This function is called to open an executable file and load the contents
 * 	into the specified buffer.  The executable file is assumed to be an OpenVMS
 * 	formatted file, so the first 576 (0x240) bytes are skipped (this is the
 * 	image header and is not our concern).  The remaining file is read byte by
 * 	byte into the supplied buffer.  The function returns the number of bytes
 * 	read.
 *
 *	NOTE:	We only ever read in an executable file and never write one.  So,
 *			there is no companion Unload function.
 *
 * Input Parameters:
 * 	fineName:
 * 		A string containing the name of the file to read.  It includes the file
 * 		path (relative or explicit).
 * 	bufferLen:
 * 		A value indicating the size of the buffer parameter.  This is used to
 * 		prevent us from writing past the end of the buffer.
 *
 * Output Parameters:
 * 	buffer:
 * 		A pointer to an array of unsigned chars in which the contents of the
 * 		executable file are written.
 *
 * Return Values:
 * 	AXP_E_FNF:			File not found.
 * 	AXP_E_BUFTOOSMALL:	Buffer too small to receive contents of file.
 * 	0:					No data in file.
 * 	>0					Number of bytes read.
 */
i32 AXP_LoadExecutable(char *fileName, u8 *buffer, u32 bufferLen)
{
	FILE	*fp;
	int		retVal = 0;
	int		ii, jj;
	bool	eofHit = false;
	u8		scratch[4];

	if (AXP_UTL_BUFF)
	{
		printf(
			"\n\nDECaxp: opened executable file %s for reading\n\n",
			fileName);
		printf("Header bytes:\n");
	}

	/*
	 * First open the file to be loaded.
	 */
	fp = fopen(fileName, "r");
	if (fp != NULL)
	{

		/*
		 * Read past the executable image header.
		 */
		for (ii = 0; ii < 0x240; ii++)
		{
			if (feof(fp))
			{
				eofHit = true;
				break;
			}
			fread(&scratch[ii % 4], 1, 1, fp);
			if (AXP_UTL_BUFF)
			{
				printf("%x", scratch[ii]);
				if (((ii + 1) % 4) == 0)
				{
					printf("\t");
					for (jj = sizeof(u32)-1; jj >= 0; jj--)
						printf("%c", (isprint(scratch[jj]) ? scratch[jj] : '.'));
					printf("\n");
				}
			}
		}

		/*
		 * Continue looking, reading 1 byte at a time, until we either,
		 * reach the end of file (we may already be there), or we run out of
		 * buffer to write into.
		 */
		ii = 0;
		if (AXP_UTL_BUFF)
			printf("\n\nExecutable bytes:\n");
		while ((eofHit == false) && (retVal == 0))
		{
			fread(&buffer[ii], 1, 1, fp);
			if (feof(fp))
				eofHit = true;
			else
			{
				ii++;

				/*
				 * Make sure there is still room for another byte of buffer for
				 * another byte of data.
				 */
				if (ii >= bufferLen)
					retVal = AXP_E_BUFTOOSMALL;
			}
			if (AXP_UTL_BUFF)
			{
				printf("%x", buffer[ii]);
				if (((ii + 1) % 4) == 0)
				{
					printf("\t");
					for (ii = 4; jj > 0; jj--)
						printf(
							"%c",
							(isprint(buffer[ii-jj]) ? buffer[ii-jj] : '.'));
					printf("\n");
				}
			}
		}

		/*
		 * If we did not detect an error, then return the number of bytes
		 * read.
		 */
		if (retVal == 0)
			retVal = ii;

		/*
		 * We opened the file, so now close it.
		 */
		fclose(fp);
	}

	/*
	 * If the file was not found, return an appropriate error value.
	 */
	else
		retVal = AXP_E_FNF;

	/*
	 * Return the result of this function to the caller.
	 */
	return(retVal);
}

/*
 * AXP_OpenRead_SROM
 *	This function is called to open a Serial ROM file for reading and return
 *	various header information from the file.  If the header information does
 *	not look correct, the file will be closed and a failure indication
 *	returned.
 *
 * Input Parameters:
 *	fileName:
 *		A null-terminated string containing the name of the file to open.
 *
 * Output Parameters:
 *	sromHandle:
 *		A pointer to a location to receive the header information and hold the
 *		file pointer, which is utilized in the read and close functions.
 *
 * Return Values:
 *	false:	Normal successful completion.
 *	true:	An error occurred.
 */
bool AXP_OpenRead_SROM(char *fileName, AXP_SROM_HANDLE *sromHandle)
{
	const char	*errMsg = "%%AXP-E-%s, %s\n";
	char		*localFileName = "";
	int			ii = 0;
	bool		retVal = false;

	if (fileName != NULL)
		localFileName = fileName;
	if (AXP_UTL_BUFF)
		printf(
			"\n\nDECaxp: opened SROM file %s for reading\n\n",
			localFileName);

	/*
	 * Initialize the handle structure with all zeros.
	 */
	memset(sromHandle, 0, sizeof(AXP_SROM_HANDLE));

	/*
	 * Copy the filename into the handle, then try opening the file.
	 */
	strcpy(sromHandle->fileName, localFileName);
	sromHandle->fp = fopen(fileName, "r");

	/*
	 * If teh file was successfully open, then let's try reading in the header.
	 */
	if (sromHandle->fp != NULL)
	{
		sromHandle->openForWrite = false;

		/*
		 * Loop through each of the fields and read in the appropriate size for
		 * each field.
		 */
		while ((ii < AXP_ROM_HDR_CNT) && (retVal == false))
		{
			switch (ii)
			{
				case 0:
					fread(
						&sromHandle->validPat,
						sizeof(u32),
						1,
						sromHandle->fp);
					if (sromHandle->validPat != AXP_ROM_VAL_PAT)
					{
						printf(errMsg, "VPB", "Validity pattern bad.");
						retVal = true;
					}
					break;

				case 1:
					fread(
						&sromHandle->inverseVP,
						sizeof(u32),
						1,
						sromHandle->fp);
					if (sromHandle->validPat != AXP_ROM_INV_VP_PAT)
					{
						printf(
							errMsg,
							"IVPB",
							"Inverse validity pattern bad.");
						retVal = true;
					}
					break;

				case 2:
					fread(&sromHandle->hdrSize, sizeof(u32), 1, sromHandle->fp);
					if (sromHandle->hdrSize != AXP_ROM_HDR_LEN)
					{
						printf(errMsg, "HDRIVP", "Header size invalid.");
						retVal = true;
					}
					break;

				case 3:
					fread(
						&sromHandle->imgChecksum,
						sizeof(u32),
						1,
						sromHandle->fp);
					break;

				case 4:
					fread(&sromHandle->imgSize, sizeof(u32), 1, sromHandle->fp);
					break;

				case 5:
					fread(
						&sromHandle->decompFlag,
						sizeof(u32),
						1,
						sromHandle->fp);
					break;

				case 6:
					fread(
						&sromHandle->destAddr,
						sizeof(u64),
						1,
						sromHandle->fp);
					break;

				case 7:
					fread(&sromHandle->hdrRev, sizeof(u8), 1, sromHandle->fp);
					break;

				case 8:
					fread(&sromHandle->fwID, sizeof(u8), 1, sromHandle->fp);
					break;

				case 9:
					fread(
						&sromHandle->hdrRevExt,
						sizeof(u8),
						1,
						sromHandle->fp);
					break;

				case 10:
					fread(&sromHandle->res, sizeof(u8), 1, sromHandle->fp);
					break;

				case 11:
					fread(
						&sromHandle->romImgSize,
						sizeof(u32),
						1,
						sromHandle->fp);
					break;

				case 12:
					fread(&sromHandle->optFwID, sizeof(u64), 1, sromHandle->fp);
					break;

				case 13:
					fread(
						&sromHandle->romOffset,
						sizeof(u32),
						1,
						sromHandle->fp);
					if (sromHandle->romOffset & 1)
						sromHandle->romOffsetValid = true;
					sromHandle->romOffset &= ~1;
					break;

				case 14:
					fread(
						&sromHandle->hdrChecksum,
						sizeof(u32),
						1,
						sromHandle->fp);
					break;
			}

			/*
			 * If we read the end of the file, then this is not a valid SROM
			 * file.
			 */
			if (feof(sromHandle->fp) != 0)
			{
				printf(errMsg, "BADROM", "This is not a valid ROM file.");
				retVal = true;
			}
			ii++;
		}

		/*
		 * Now that we have read in the header, check to see if the checksum
		 * specified in the header file matches a calculated value.
		 */
		if ((retVal == false) &&
			(AXP_Crc32(
					(u8 *) &sromHandle->validPat,
					AXP_ROM_HDR_LEN, false,
					0) == sromHandle->hdrChecksum))
		{
			printf(errMsg, "BADCSC", "Header CSC-32 is not valid.");
			retVal = true;
		}

		/*
		 * If an error occurred, then we need to close the file, since we
		 * successfully opened it.
		 */
		if (retVal == true)
		{
			fclose(sromHandle->fp);
			memset(sromHandle, 0, sizeof(AXP_SROM_HANDLE));
		}
		else if (AXP_UTL_BUFF)
		{
			u8	*optFwID = (u8 *) &sromHandle->optFwID;

			printf("SROM Header Information:\n\n");
			printf("Header Size......... %u bytes\n", sromHandle->hdrSize);
			printf("Image Checksum...... 0x%08x\n", sromHandle->imgChecksum);
			printf("Image Size (Uncomp). %u (%u KB)\n", sromHandle->imgSize, sromHandle->imgSize/ONE_K);
			printf("Compression Type.... %u\n", sromHandle->decompFlag);
			printf("Image Destination... 0x%016llx\n", sromHandle->destAddr);
			printf("Header Version...... %d\n", sromHandle->hdrRev);
			printf("Firmware ID......... %d - %s\n", sromHandle->fwID, _axp_fwid_str[sromHandle->fwID]);
			printf("ROM Image Size...... %u (%u KB)\n", sromHandle->romImgSize, sromHandle->romImgSize/ONE_K);
			printf("Firmware ID (Opt.).. ");
			for (ii = 0; ii < sizeof(sromHandle->optFwID); ii++)
				printf("%02d", optFwID[ii]);
			printf("\nHeader Checksum..... 0x%08x\n", sromHandle->hdrChecksum);
		}
	}
	else
	{
		printf(errMsg, "FNF", "File not found.");
		retVal = true;
	}
	return(retVal);
}

/*
 * AXP_OpenWrite_SROM
 *	This function is called to open a Serial ROM file for writing.
 *
 * Input Parameters:
 *	fileName:
 *		A null-terminated string containing the name of the file to open.
 *
 * Output Parameters:
 *	sromHandle:
 *		A pointer to a location to receive the header information and hold the
 *		file pointer, which is utilized in the read and close functions.
 *
 * Return Values:
 *	false:	Normal successful completion.
 *	true:	An error occurred.
 */
bool AXP_OpenWrite_SROM(
			char *fileName,
			AXP_SROM_HANDLE *sromHandle,
			u64 destAddr,
			u32 fwID)
{
	bool		retVal = false;

	if (AXP_UTL_BUFF)
		printf(
			"\n\nDECaxp: opened SROM file %s for writing\n\n",
			fileName);

	/*
	 * Initialize the handle structure with all zeros.
	 */
	memset(sromHandle, 0, sizeof(AXP_SROM_HANDLE));

	/*
	 * Copy the filename into the handle, then try opening the file.
	 */
	strcpy(sromHandle->fileName, fileName);
	sromHandle->fp = fopen(fileName, "w");
	if (sromHandle->fp != NULL)
	{
		sromHandle->openForWrite = true;
		sromHandle->validPat = AXP_ROM_VAL_PAT;
		sromHandle->inverseVP = AXP_ROM_INV_VP_PAT;
		sromHandle->hdrSize = AXP_ROM_HDR_LEN;
		sromHandle->decompFlag = 0;
		sromHandle->destAddr = destAddr;
		sromHandle->hdrRev = AXP_ROM_HDR_VER;
		sromHandle->fwID = fwID;
		sromHandle->hdrRevExt = 0;
		sromHandle->romImgSize = 0;

		/*
		 * Firmware ID: V2.0.0 yymmdd hhmm = 0200001711212316
		 */
		sromHandle->optFwID = 0200001711212316;
		sromHandle->romOffset = 0;	/* no ROM offset */
	}
	else
		retVal = true;

	/*
	 * Return the result of this call back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_Read_SROM
 *	This function is called to read data from the Serial ROM file opened by the
 *	AXP_Open_SRM function.
 *
 * Input Parameters:
 *	sromHandle:
 *		A pointer to a location containing the file pointer to be closed.
 *	bufLen:
 *		A value specifying the maximum length of the 'buf' parameter.
 *
 * Output Parameters:
 *	buf:
 *		A pointer to a u8 buffer to receive the data next set of data from the
 *		SROM file.
 *
 * Return Values:
 *	AXP_E_EOF:			End-of-file reached.
 *	AXP_E_READERR:		Error reading file.
 *	AXP_E_BADSROMFILE:	Bad SROM file detected.
 *	>0:					Number of bytes successfully read (up to bufLen unsigned
 *						bytes)
 */
i32 AXP_Read_SROM(AXP_SROM_HANDLE *sromHandle, u8 *buf, u32 bufLen)
{
	i32		retVal = 0;

	/*
	 * Make sure we are not at the end of file before trying to read from the
	 * SROM file.
	 */
	if (feof(sromHandle->fp) == 0)
	{

		/*
		 * Read up to bufLen worth of unsigned bytes.
		 */
		retVal = fread(buf, bufLen, 1, sromHandle->fp);

		/*
		 * If an error was returned, the return one to the caller.  Otherwise,
		 * update the running image CRC.
		 */
		if (retVal < 0)
			retVal = AXP_E_READERR;
		else
		{
			sromHandle->verImgChecksum = AXP_Crc32(
											buf,
											retVal,
											true,
											sromHandle->verImgChecksum);

			/*
			 * If we hit the end-of-file, then inverse the CRC and check it
			 * against the one from the file header.
			 */
			if (feof(sromHandle->fp) != 0)
			{
				sromHandle->verImgChecksum = ~sromHandle->verImgChecksum;

				/*
				 * If the newly calculated image checksum does not match the
				 * one read in the header file.
				 */
				if (sromHandle->verImgChecksum != sromHandle->imgChecksum)
					retVal = AXP_E_BADSROMFILE;
			}
		}
	}
	else
		retVal = AXP_E_EOF;
	return(retVal);
}

/*
 * AXP_Write_SROM
 *	This function is called to write data to the Serial ROM file opened by the
 *	AXP_OpenWrite_SRM function.
 *
 * Input Parameters:
 *	sromHandle:
 *		A pointer to a location containing the file pointer to be closed.
 *	buf:
 *		A pointer to a u8 buffer to receive the data next set of data from the
 *		SROM file.
 *	bufLen:
 *		A value specifying the maximum length of the 'buf' parameter.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 */
bool AXP_Write_SROM(AXP_SROM_HANDLE *sromHandle, u8 *buf, u32 bufLen)
{
	bool		retVal = false;

	/*
	 * [re]allocate the write buffer to be able to store the next bit of data.
	 */
	sromHandle->writeBuf = realloc(
								sromHandle->writeBuf,
								sromHandle->imgSize + bufLen);

	/*
	 * If the buffer was reallocated, copy the next chunk of data.
	 */
	if (sromHandle->writeBuf != NULL)
	{
		memcpy(&sromHandle->writeBuf[sromHandle->imgSize], buf, bufLen);
		sromHandle->imgSize += bufLen;
	}
	else
		retVal = true;
	return(retVal);
}

/*
 * AXP_Close_SROM
 *	This function is called to close the Serial ROM file opened by the
 *	AXP_OpenRead_SROM or AXP_OpenWrite_SROM functions.
 *
 * Input Parameters:
 *	sromHandle:
 *		A pointer to a location containing the file pointer to be closed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	false:	Normal successful completion.
 *	true:	An error occurred.
 */
bool AXP_Close_SROM(AXP_SROM_HANDLE *sromHandle)
{
	const char	*errMsg = "%%AXP-E-%s, %s\n";
	bool		retVal = false;

	/*
	 * If we have a non-NULL file pointer, then we need to close it.
	 */
	if ((sromHandle->fp != NULL) &&
		(sromHandle->validPat == AXP_ROM_VAL_PAT) &&
		(sromHandle->validPat == AXP_ROM_INV_VP_PAT) &&
		(sromHandle->hdrSize == AXP_ROM_HDR_LEN))
	{

		/*
		 * If the file was opened for write, then we have been collecting the
		 * information, but have not written it out, yet.  We need to first
		 * calculate the image checksum, then the header checksum before
		 * writing the header information, followed by the image data.
		 */
		if (sromHandle->openForWrite == true)
		{

			/*
			 * Calculate the 2 checksums (header and image).
			 */
			sromHandle->imgChecksum = AXP_Crc32(
										sromHandle->writeBuf,
										sromHandle->imgSize,
										true,
										0);
			sromHandle->hdrChecksum = AXP_Crc32(
										(u8 *) &sromHandle->validPat,
										AXP_ROM_HDR_LEN,
										false,
										0);
			if (AXP_UTL_BUFF)
			{
				u8	*optFwID = (u8 *) &sromHandle->optFwID;
				int	ii;

				printf("SROM Header Information:\n\n");
				printf("Header Size......... %u bytes\n", sromHandle->hdrSize);
				printf("Image Checksum...... 0x%08x\n", sromHandle->imgChecksum);
				printf("Image Size (Uncomp). %u (%u KB)\n", sromHandle->imgSize, sromHandle->imgSize/ONE_K);
				printf("Compression Type.... %u\n", sromHandle->decompFlag);
				printf("Image Destination... 0x%016llx\n", sromHandle->destAddr);
				printf("Header Version...... %d\n", sromHandle->hdrRev);
				printf("Firmware ID......... %d - %s\n", sromHandle->fwID, _axp_fwid_str[sromHandle->fwID]);
				printf("ROM Image Size...... %u (%u KB)\n", sromHandle->romImgSize, sromHandle->romImgSize/ONE_K);
				printf("Firmware ID (Opt.).. ");
				for (ii = 0; ii < sizeof(sromHandle->optFwID); ii++)
					printf("%02d", optFwID[ii]);
				printf("\nHeader Checksum..... 0x%08x\n", sromHandle->hdrChecksum);
			}

			/*
			 * Write out all the header data.
			 */
			fwrite(&sromHandle->validPat, sizeof(u32), 1, sromHandle->fp);
			fwrite(&sromHandle->inverseVP, sizeof(u32), 1, sromHandle->fp);
			fwrite(&sromHandle->hdrSize, sizeof(u32), 1, sromHandle->fp);
			fwrite(&sromHandle->imgChecksum, sizeof(u32), 1, sromHandle->fp);
			fwrite(&sromHandle->imgSize, sizeof(u32), 1, sromHandle->fp);
			fwrite(&sromHandle->decompFlag, sizeof(u32), 1, sromHandle->fp);
			fwrite(&sromHandle->destAddr, sizeof(u64), 1, sromHandle->fp);
			fwrite(&sromHandle->hdrRev, sizeof(u8), 1, sromHandle->fp);
			fwrite(&sromHandle->fwID, sizeof(u8), 1, sromHandle->fp);
			fwrite(&sromHandle->hdrRevExt, sizeof(u8), 1, sromHandle->fp);
			fwrite(&sromHandle->res, sizeof(u8), 1, sromHandle->fp);
			fwrite(&sromHandle->romImgSize, sizeof(u32), 1, sromHandle->fp);
			fwrite(&sromHandle->optFwID, sizeof(u64), 1, sromHandle->fp);
			fwrite(&sromHandle->romOffset, sizeof(u32), 1, sromHandle->fp);
			fwrite(&sromHandle->hdrChecksum, sizeof(u32), 1, sromHandle->fp);

			/*
			 * Write out the image data.
			 */
			fwrite(
				sromHandle->writeBuf,
				1,
				sromHandle->imgSize,
				sromHandle->fp);

			/*
			 * Free the buffer we allocated for the image data.  We no longer
			 * need it.
			 */
			free(sromHandle->writeBuf);
		}

		/*
		 * We are done with the file, so close it, then clear the handle.
		 */
		fclose(sromHandle->fp);
		memset(sromHandle, 0, sizeof(AXP_SROM_HANDLE));
	}
	else
	{
		printf(errMsg, "FNO", "File not opened.");
		retVal = true;
	}
	return(retVal);
}

/*
 * AXP_MaskReset
 *	This function is called to reset the indicated mask.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	mask:
 *		A pointer to a 64-bit unsigned integer to have all its bits cleared.
 *
 * Return Value:
 * 	None.
 */
void AXP_MaskReset(u64 *mask)
{
	*mask = 0;

	/*
	 * return back to the caller.
	 */
	return;
}

/*
 * AXP_MaskSet
 *	This function is called to set some bits in a mask, representing occupied
 *	locations within a 64 byte buffer.
 *
 * Input Parameters:
 *	mask:
 *		A pointer containing the current set of masked bits.
 *	basePA:
 *		A 64-bit unsigned value representing the base address of the buffer.
 *	pa:
 *		A 64-bit unsigned value representing the base address of the data
 *		written to the buffer.
 *	len:
 *		A value indicating the length of the data written to the buffer.
 *
 * Output Parameters:
 * 	mask:
 * 		A pointer with the updated set of bit masks.
 *
 * Return Value:
 * 	None.
 */
void AXP_MaskSet(u64 *mask, u64 basePa, u64 pa, int len)
{
	switch(len)
	{
		case BYTE_LEN:
			*mask |= (AXP_MASK_BYTE << (pa - basePa));
			break;

		case WORD_LEN:
			*mask |= (AXP_MASK_WORD << (pa - basePa));
			break;

		case LONG_LEN:
			*mask |= (AXP_MASK_LONG << (pa - basePa));
			break;

		case QUAD_LEN:
			*mask |= (AXP_MASK_QUAD << (pa - basePa));
			break;

		case CACHE_LEN:
			*mask = AXP_LOW_QUAD;
			break;
	}

	/*
	 * return back to the caller.
	 */
	return;
}

/*
 * AXP_MaskStartGet
 *	This function is called to set up the pointer into the mask to its initial
 *	value.  This pointer is used to determine where we left off for successive
 *	AXP_MaskGets.  The pointer needs to be supplied for each of the AXP_MaskGet
 *	calls.  A call to this function must be performed prior to any set of
 *	AX_MaskGets.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	curPtr:
 *		A pointer to a location indicating where we left off in returning the
 *		set of currently set masks.
 *
 * Return Value:
 * 	None.
 */
void AXP_MaskStartGet(int *curPtr)
{
	*curPtr = 0;

	/*
	 * return back to the caller.
	 */
	return;
}

/*
 * AXP_MaskGet
 *	This function is called to return the offset to the next location within
 *	the buffer with the valid data.  The current point into the buffer is set
 *	to the next location.
 *
 * Input Parameters:
 *	curPtr:
 *		A pointer to a location with the current location within the mask to be
 *		parsed.
 *	mask:
 *		A 64-bit value representing the mask being parsed.
 *	len:
 *		A value indicating the length of the data written to the buffer.
 *
 * Output Parameters:
 *	curPtr:
 *		A pointer to a location to receive the next place to look for valid
 *		data within the buffer.  A value of -1 indicate that there is no more
 *		to parse.
 *
 * Return Value:
 *	-1:		Nothing to parse.
 *	>=0:	A value representing the byte offset within a buffer containing
 *			valid data.
 */
int AXP_MaskGet(int *curPtr, u64 mask, int len)
{
	int		retVal = -1;
	int		curPtrMax;
	u16		maskBits;

	switch(len)
	{
		case BYTE_LEN:
			maskBits = AXP_MASK_BYTE;
			curPtrMax = 64;
			break;

		case WORD_LEN:
			maskBits = AXP_MASK_WORD;
			curPtrMax = 63;
			break;

		case LONG_LEN:
			maskBits = AXP_MASK_LONG;
			curPtrMax = 61;
			break;

		case QUAD_LEN:
			maskBits = AXP_MASK_QUAD;
			curPtrMax = 57;
			break;

		case CACHE_LEN:
			maskBits = AXP_LOW_QUAD;
			curPtrMax = 1;
			break;
	}

	/*
	 * If this is the first time we are parsing this mask, then the current
	 * pointer may not be pointing to the next valid data.  We need to do an
	 * initial search for the first one.
	 */
	if (*curPtr == 0)
	{

		/*
		 * Loop through each of the mask bits, trying to find a set for the
		 * current data length, or until we run out of bits to check.
		 */
		while (((mask & (maskBits << *curPtr)) != (maskBits << *curPtr)) &&
			   (*curPtr < curPtrMax))
		{
			*curPtr = *curPtr + 1;
		}
		if (*curPtr == curPtrMax)
			*curPtr = -1;
	}

	/*
	 * At this point the current point either needs to point to the next set of
	 * mask bits or is negative, indicating that there is nothing more to
	 * process.  If the former, then return the current pointer value to the
	 * caller and determine the next, if any, offset to be returned.
	 */
	if (*curPtr > 0)
	{
		retVal = *curPtr;

		/*
		 * Loop through each of the mask bits, trying to find a set for the
		 * current data length, or until we run out of bits to check.
		 */
		while (((mask & (maskBits << *curPtr)) != (maskBits << *curPtr)) &&
			   (*curPtr < curPtrMax))
		{
			*curPtr = *curPtr + 1;
		}
		if (*curPtr == curPtrMax)
			*curPtr = -1;
	}

	/*
	 * Return what we found back to the caller.
	 */
	return(retVal);
}
