/*
 * Copyright (C) Jonathan D. Belanger 2017-2018.
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
 *  This file contains the utility functions needed throughout the Alpha AXP
 *  emulation code.
 *
 * Revision History:
 *
 *  V01.000	29-May-2017	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	01-Jun-2017	Jonathan D. Belanger
 *  Added an LRU function to return the least recently used set with a
 *  particular index.
 *
 *  V01.002	18-Nov-2017	Jonathan D. Belanger
 *  Added a load executable and load/unload ROM set of functions.  The first
 *  assumes the file to be loaded is an VMS executable.  The second set is
 *  used to load and unload Read Only Memory (ROM) data.  This data contains
 *  the console firmware and PALcode to support the Alpha AXP 21264 CPU.
 *
 *  V01.003	19-Nov-2017	Jonathan D. Belanger
 *  Added conditional queue functions to check for empty, insert, remove, and
 *  wait for an entry to be queued.
 *
 *  V01.004	21-Nov-2017	Jonathan D. Belanger
 *  Decided to re-do the ROM load/unload routines to move the CPU specific
 *  structures out of this module.  They are going to be moved into the Cbox.
 *
 *  V01.005	29-Dec-2017	Jonathan D. Belanger
 *  When sending data from the CPU to the System, we do so in upto 64-byte
 *  blocks.  Since these blocks are not necessarily consecutive, there may be
 *  gaps between the end of what section of relevant data and the start of the
 *  next.  In the real CPU, this would be broken down into a series of
 *  WrBytes/WrLWs/WrQWs or ReadBytes/ReadLWs/ReadQWs.  We need to mimic this,
 *  but do it in a single step.  We need some functions to set/decode the
 *  appropriate set of mask bits (each bit represent a single byte).
 *
 *  V01.006	02-Jul-2018	Jonathan D. Belanger
 *  Rewrote the CRC-32 calculation from bitwise to table driven (I'd prefer
 *  table driven, and the bitwise version did not always seem to be working).
 */
#include "AXP_Configure.h"
#include "AXP_Utility.h"
#include "AXP_Trace.h"

/*
 * This format is used throughout this module for writing a message to sysout.
 */
const char *errMsg = "%%AXP-E-%s, %s\n";
static const char *_axp_fwid_str[] =
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

static const u32 AXP_CRC_Table[256] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
    0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
    0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
    0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
    0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
    0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
    0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
    0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
    0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
    0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
    0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
    0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
    0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
    0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
    0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
    0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/*
 * AXP_Crc32
 *  This function is called to determine what the CRC32 from a supplied buffer.
 *
 * Input Parameters:
 *  msg:
 *	A pointer to a binary string containing the data from which to
 *	calculate the CRC-32.
 *  len:
 *	A value indicating the length, in bytes, of the 'msg' parameter.
 *	inverse:
 *  inverse:
 *	A boolean indicating whether the return value should be inverted.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	An unsigned 32-bit value of the calculated CRC-32.
 */
u32 AXP_Crc32(const u8 *msg, size_t len, bool inverse, u32 curCRC)
{
    const u32	mask = 0xffffffff;
    u32		retVal = (curCRC ^= mask);
    int		ii;

    for (ii = 0; ii < len; ii++)
	retVal = AXP_CRC_Table[(retVal ^ msg[ii]) & 0xff] ^ (retVal >> 8);

    retVal ^= mask;

    /*
     * Return the newly calculated CRC, take the inverse if that is what is
     * being requested, back to the caller.
     */
    return(inverse ? ~retVal : retVal);
}

/*
 * AXP_LRUAdd
 *  This function is called to add a particular item into a Least Recently Used
 *  (LRU) queue.  This list is maintained with the LRU Queue Header's
 *  forward-link pointing to the LRU entry and the backward-link pointing to
 *  the Most Recently Used (MRU) item.
 *
 * Input Parameters:
 *  lruQ:
 * 	A pointer to the LRU Queue Head.
 *  entry:
 * 	A pointer to the entry to be made the MRU item.  If the entry is
 * 	already the MRU item, then nothing is done.  If it is already in the
 * 	queue, then it is moved to the end of the queue.  Otherwise, it is
 * 	inserted to the end of the queue.
 *
 * Output Parameters:
 *  lruQ:
 * 	The flink and blink fields may be updated, as well as the flink of the
 * 	current last entry in the queue.
 *  entry:
 *	The flink and blink fields may be updated.
 *
 * Return Value:
 *  None, this always succeeds.
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
 *  This function is called to remove a particular index/set pair off of a
 *  Least Recently Used (LRU) list.  This list is maintained with the LRU item
 *  at the top and the Most Recently Used (MRU) at the bottom.
 *
 * Input Parameters:
 *  entry:
 *	A pointer to the entry to be removed from the queue.
 *
 * Output Parameters:
 *  entry:
 *	The entry before will point to the entry after, the entry after will
 *	point to the entry before, and the entry will point to itself.
 *
 * Return Value:
 *  None.
 */
void AXP_LRURemove(AXP_QUEUE_HDR *entry)
{
    ((AXP_QUEUE_HDR *) entry->blink)->flink = entry->flink;
    ((AXP_QUEUE_HDR *) entry->flink)->blink = entry->blink;
    AXP_INIT_QUEP(entry); /* make the entry point to itself */

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_LRUReturn
 *  This function is called to return the Least Recently Used (LRU) item off
 *  the list.  This list is maintained with the LRU item at the top and the
 *  Most Recently Used (MRU) at the bottom.
 *
 * Input Parameters:
 *  lru:
 *	A pointer to the list of LRU items.
 *  lruMax:
 *	A value of the last item in the list.
 *
 * Output Parameters:
 *  index:
 *	A pointer to receive the LRU index of the cache, of the index/set pair,
 *	on the list.  Index for the Icache is 33 bits long.
 *  set:
 *	A pointer to receive the LRU set of the cache, of the index/set pair,
 *	on the list.
 *
 * Return Value:
 *  true:	If the item was successfully located (the list has items in it).
 *  false:	If there are no items on the list.
 */
AXP_QUEUE_HDR *AXP_LRUReturn(AXP_QUEUE_HDR *lruQ)
{
    AXP_QUEUE_HDR *entry;

    if (AXP_QUEP_EMPTY(lruQ))
	entry = NULL;
    else
	entry = (AXP_QUEUE_HDR *) lruQ->flink;
    return (entry);
}

#ifdef AXP_VERIFY_QUEUES
/*
 * AXP_VerifyCountedQueue
 *  This function is called with a single counted queue entry, locates the
 *  parent and loops forward and backward through the queue to verify the
 *  queue's integrity.
 *
 * Input Parameters:
 *  entry:
 *	A pointer to one of the entries in the counted queue, which will be
 *	to gather the other information needed to verify the queues integrity.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	The queue is valid.
 *  false:	The queue has issues.
 */
bool AXP_VerifyCountedQueue(AXP_COUNTED_QUEUE *parent)
{
    AXP_CQUE_ENTRY *next;
    u32 entries = 0;
    bool retVal = true;

    if (AXP_UTL_BUFF)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv");
	AXP_TraceWrite(
		"Checking integrity of counted queue 0x%016llx, "
		"with %u entries and a maximum of %u",
		(u64) parent,
		parent->count,
		parent->max);
	AXP_TRACE_END();
    }

    next = parent->flink;
    while (((void *) next != (void *)parent) && (entries < parent->max))
    {
	if (AXP_UTL_BUFF)
	{
	    AXP_TRACE_BEGIN();
	    AXP_TraceWrite(
		    "\t0x%016llx --> 0x%016llx",
		    (u64) next,
		    (u64) next->flink);
	    AXP_TRACE_END();
	}
	entries++;
	next = next->flink;
    }

    /*
     * If we got back to the parent and the number of entries we looked at is
     * less than or equal to the maximum number of entries, then go backwards
     * through the queue.
     */
    if (((void *) next == (void *) parent) &&
	(entries <= parent->max) &&
	(entries == parent->count))
    {
	if (AXP_UTL_BUFF)
	{
	    AXP_TRACE_BEGIN();
	    AXP_TraceWrite(
		    "Checking integrity of blink for 0x%016llx",
		    (u64) parent);
	    AXP_TRACE_END();
	}
	next = parent->blink;
	while (((void *) next != (void *)parent) && (entries > 0))
	{
	    if (AXP_UTL_BUFF)
	    {
		AXP_TRACE_BEGIN();
		AXP_TraceWrite(
			"\t0x%016llx <-- 0x%016llx",
			(u64) next,
			(u64) next->blink);
		AXP_TRACE_END();
	    }
	    entries--;
	    next = next->blink;
	}
	if (((void *) next != (void *) parent) || (entries != 0))
	retVal = false;
    }
    else
    retVal = false;

    if (AXP_UTL_BUFF)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
		"Queue @ 0x%016llx integrity is %s",
		(u64) parent,
		(retVal ? "GOOD" : "BAD"));
	AXP_TraceWrite("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}
#endif

/*
 * AXP_InitCountedQueue
 *  This function is called to initialize a counted queue header (parent).
 *
 * Input Parameters:
 *  parent:
 *	A pointer to the queue head to be initialized.
 *  max:
 *	A value indicating the maximum number of entries allowed in the counted
 *	queue.
 *
 * Output Parameters:
 *  parent:
 *	A pointer to the initialized queue head.
 *
 * Return Value:
 *  true:	Queue initialized successfully.
 *  false:	Queue initialization failed.
 */
bool AXP_InitCountedQueue(AXP_COUNTED_QUEUE *parent, u32 max)
{
    bool retVal = true;

    /*
     * Initialize the mutex for this queue.  If successful, then initialize the
     * rest of the queue structure.
     */
    if (pthread_mutex_init(&parent->mutex, NULL) == 0)
    {
	parent->flink = parent->blink = (AXP_CQUE_ENTRY *) parent;
	parent->max = max;
	parent->count = 0;
    }
    else
	retVal = false;

    /*
     * Return the results of the initialization back to the caller.
     */
    return (retVal);
}

/*
 * AXP_LockCountedQueue
 *  This function is called to lock the mutex of a counted queue.
 *
 * Input Parameters:
 *  parent:
 *	A pointer to the queue head to be locked.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  true:	Queue locked successfully.
 *  false:	Queue locked failed.
 */
bool AXP_LockCountedQueue(AXP_COUNTED_QUEUE *parent)
{

    /*
     * Lock the mutex for this queue.  If successful, return true, otherwise
     * false.
     */
    return (pthread_mutex_lock(&parent->mutex) == 0);
}

/*
 * AXP_UnlockCountedQueue
 *  This function is called to unlock the mutex of a counted queue.
 *
 * Input Parameters:
 *  parent:
 *	A pointer to the queue head to be unlocked.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  true:	Queue unlocked successfully.
 *  false:	Queue unlocked failed.
 */
bool AXP_UnlockCountedQueue(AXP_COUNTED_QUEUE *parent)
{

    /*
     * Unlock the mutex for this queue.  If successful, return true, otherwise
     * false.
     */
    return (pthread_mutex_unlock(&parent->mutex) == 0);
}

/*
 * AXP_InsertCountedQueue
 *  This function is called to insert an entry into a specific location within
 *  a counted queue.  The counter at the queue parent (head) is incremented.
 *
 * Input Parameters:
 *  pred:
 *	A pointer to the predecessor entry where 'entry' is to be inserted.
 *  entry:
 *	A pointer to the entry that is to be inserted.
 *
 * Output Parameters:
 *  pred:
 *	The forward link now points to 'entry' and the backward link of the
 *	entry that originally succeeded 'pred' also points to 'entry'.
 *  entry:
 *	The backward link now points to 'pred' and the forward link points to
 *	the entry previously pointed to by 'pred's forward link.
 *
 * Return Value:
 *  -1, if the queue has the maximum number of entries.
 *   0, if the queue was not empty before adding the entry.
 *   1, if the queue was empty.
 */
i32 AXP_InsertCountedQueue(AXP_CQUE_ENTRY *pred, AXP_CQUE_ENTRY *entry)
{
    AXP_COUNTED_QUEUE *parent = entry->parent;
    i32 retVal = -1;

#ifdef AXP_VERIFY_QUEUES
    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
		"AXP_InsertCountedQueue Called for 0x%016llx to "
		"insert 0x%016llx",
		(u64) entry->parent,
		(u64) entry);
	AXP_TRACE_END();
    }

    AXP_VerifyCountedQueue(entry->parent);
#endif

    /*
     * First things first, lock the mutex for this queue.
     */
    pthread_mutex_lock(&parent->mutex);

    /*
     * Check that there is more room in the queue for another entry.
     */
    if (parent->count < parent->max)
    {

	/*
	 * Insert the entry after the pred[ecessor].
	 */
	entry->flink = pred->flink;
	entry->blink = pred;
	pred->flink = entry->flink->blink = entry;

	/*
	 * Finally, since this is a counted queue, increment the counter in the
	 * parent.
	 */
	parent->count++;
	retVal = (parent->count == 1) ? 1 : 0; /* was queue empty */
    }
#ifdef AXP_VERIFY_QUEUES
    AXP_VerifyCountedQueue(entry->parent);
#endif

    /*
     * Return back to the caller.
     */
    pthread_mutex_unlock(&parent->mutex);
    return (retVal);
}

/*
 * AXP_CountedQueueFull
 *  This function is called to determine if a counted queue is full.
 *
 * Input Parameters:
 *  parent:
 *	A pointer to the parent (head) of the queue.
 *  headeRoom:
 *	A value indicating the minimum number of items allowed before the
 *	queue is considered empty.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *  -1, if the queue has the maximum number of entries.
 *   0, if the queue is not empty.
 *   1, if the queue is empty.
 */
i32 AXP_CountedQueueFull(AXP_COUNTED_QUEUE *parent, u32 headRoom)
{
    int retVal = 0; /* not empty and not full */

#ifdef AXP_VERIFY_QUEUES
    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
		"AXP_CountedQueueFull Called for 0x%016llx to "
		"with head-room of %u",
		(u64) parent,
		headRoom);
	AXP_TRACE_END();
    }

    AXP_VerifyCountedQueue(parent);
#endif

    /*
     * First things first, lock the mutex for this queue.
     */
    pthread_mutex_lock(&parent->mutex);

    /*
     * If the queue is empty, then return that indicator.
     */
    if (parent->count == 0)
	retVal = 1;

    /*
     * Otherwise, if the current count, plus the headroom requested, is greater
     * than or equal to the maximum number of possible entries, then return
     * that indicator.
     */
    else if ((parent->count + headRoom) >= parent->max)
	retVal = -1;

    /*
     * Return the results back to the caller.
     */
    pthread_mutex_unlock(&parent->mutex);
    return (retVal);
}
/*
 * AXP_RemoveCountedQueue
 *  This function is called to remove an entry out of a specific location
 *  within a counted queue.  The counter at the queue parent (head) is
 *  incremented.
 *
 * Input Parameters:
 *  entry:
 *	A pointer to the entry that is to be removed.
 *
 * Output Parameters:
 *  entry:
 *	The predecessor entry points to the successor and the successor points
 *	to the predecessor.
 *  locked:
 *	A boolean value indicating that the calling thread already locked the
 *	mutex.  The mutex will still be unlocked by this call.
 *
 * Return Value:
 *  -1, if the queue is empty.
 *   0, if the queue is empty after removing this entry.
 *   1, if the queue is not empty after removing this entry.
 */
i32 AXP_RemoveCountedQueue(AXP_CQUE_ENTRY *entry, bool locked)
{
    AXP_COUNTED_QUEUE *parent = entry->parent;
    i32 retVal;

#ifdef AXP_VERIFY_QUEUES
    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
		"AXP_RemoveCountedQueue Called for 0x%016llx to "
		"remove 0x%016llx",
		(u64) entry->parent,
		(u64) entry);
	AXP_TRACE_END();
    }

    AXP_VerifyCountedQueue(entry->parent);
#endif

    /*
     * First things first, lock the mutex for this queue, but only if it has
     * not already been locked.
     */
    if (locked == false)
	pthread_mutex_lock(&parent->mutex);

    /*
     * If there is at least one entry in the queue, then we can remove it.
     * Otherwise, the entry specified on the call was already not in the queue.
     */
    if (parent->count > 0)
    {

	/*
	 * Let's first have the predecessor and successor point to each other.
	 */
	entry->flink->blink = entry->blink;
	entry->blink->flink = entry->flink;
	entry->flink = entry->blink = NULL;

	/*
	 * Finally, since this is a counted queue, decrement the counter in the
	 * parent.
	 */
	parent->count--;
	retVal = (parent->count == 0) ? 0 : 1;
    }
    else
	retVal = -1;

#ifdef AXP_VERIFY_QUEUES
    AXP_VerifyCountedQueue(entry->parent);
#endif

    /*
     * Return the results back to the caller.
     */
    pthread_mutex_unlock(&parent->mutex);
    return (retVal);
}

/*
 * AXP_CondQueue_Init
 *  This function is called to initialize a conditional queue.  This just
 *  initializes the root of the queue, not the individual entries themselves.
 *  The initialization entails, setting the queue entry type, setting the flink
 *  and blink to the address of the header itself, and finally initializing the
 *  conditional and mutex variables.
 *
 * Input Parameters:
 *  queue:
 *	A pointer to the conditional queue root that needs to be initialized.
 *
 * Output Parameters:
 *  queue:
 *	The fields within this parameter are initialized.
 *
 * Return Value:
 *  false:	Something happened when trying to initialize the conditional and
 *		mutex variables.
 *  true:	Normal successful completion.
 */
bool AXP_CondQueue_Init(AXP_COND_Q_ROOT *queue)
{
    bool retVal = false;

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
	    retVal = true; /* We succeeded with what we set out to do */
    }
    else
	retVal = false;

    /*
     * Return the result of this call back to the caller.
     */
    return (retVal);
}

/*
 * AXP_CondQueueCnt_Init
 *  This function is called to initialize a conditional queue.  This just
 *  initializes the root of the queue, not the individual entries themselves.
 *  The initialization entails, setting the queue entry type, setting the flink
 *  and blink to the address of the header itself, setting the maximum number
 *  of entries that can be in the queue and the count to zero, and finally
 *  initializing the conditional and mutex variables.
 *
 * Input Parameters:
 *  queue:
 *	A pointer to the counted conditional queue root that needs to be
 *	initialized.
 *  max:
 *	A value indicating the total number of items that can actually be
 *	inserted into the queue.
 *
 * Output Parameters:
 *  queue:
 *	The fields within this parameter are initialized.
 *
 * Return Value:
 *  false:	Something happened when trying to initialize the conditional and
 * 		mutex variables.
 *  true:	Normal successful completion.
 */
bool AXP_CondQueueCnt_Init(AXP_COND_Q_ROOT_CNT *queue, u32 max)
{
    bool retVal = false;

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
	    retVal = true; /* We succeeded with what we set out to do */
    }
    else
	retVal = false;

    /*
     * Return the result of this call back to the caller.
     */
    return (retVal);
}

/*
 * AXP_CondQueue_Insert
 *  This function is called to insert a record (what) into the queue at a
 *  location after where.  This function will perform the following steps:
 *	1) Lock the mutex.
 *	2) If the queue is a counted queue, can it take an additional entry?
 *	3) If not a counted queue or #2 is true, then insert the record into
 *	   the queue.
 *	4) If a counted queue, then increment the counter.
 *	5) Signal the condition variable.
 *	6) Unlock the mutex.
 *	7) Return the results back to the caller.
 *
 * Input Parameters:
 *  where:
 *	A pointer to an entry in the queue (or possibly the header), after
 *	which the 'what' parameter is inserted.
 *  what:
 *	A pointer to the entry to be inserted into the queue.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  >0:		The queue cannot take another entry.
 *  =0:		Normal successful completion.
 *  <0:		An error occurred with the mutex or condition variable.
 */
i32 AXP_CondQueue_Insert(AXP_COND_Q_LEAF *where, AXP_COND_Q_LEAF *what)
{
    int retVal = 0;
    AXP_COND_Q_ROOT *parent = (AXP_COND_Q_ROOT *) where->parent;
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
	if ((parent->type == AXPCountedCondQueue)
	        && (countedParent->count >= countedParent->max))
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
    return (retVal);
}

/*
 * AXP_CondQueue_Remove
 *  This function is called to remove a record (to) from the queue at a
 *  location at from.  This function will perform the following steps:
 *	1) Lock the mutex.
 *	2) If the queue is not empty remove the record from the queue.
 *	3) If a counted queue, then decrement the counter.
 *	4) Unlock the mutex.
 *	5) Return the results back to the caller.
 *
 * Input Parameters:
 *  from:
 *	A pointer to an entry in the queue that is to be removed.
 *
 * Output Parameters:
 *  to:
 *	A pointer to a location to receive the removed entry.
 *
 * Return Value:
 *  false:	The entry was not successfully removed.
 *  true:	The entry was successfully removed.
 */
bool AXP_CondQueue_Remove(AXP_COND_Q_LEAF *from, AXP_COND_Q_LEAF **to)
{
    bool retVal = true;
    AXP_COND_Q_ROOT *parent = (AXP_COND_Q_ROOT *) from->parent;

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
    return (retVal);
}

/*
 * AXP_CondQueue_Wait
 *  This function is called to wait on the specified queue until has something
 *  in it.  If there is something in the queue when called, then there will be
 *  not waiting.
 *
 * Input Parameters:
 *  root:
 *	A pointer to the root of the condition queue.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_CondQueue_Wait(AXP_COND_Q_HDR *root)
{
    AXP_COND_Q_ROOT *parent = (AXP_COND_Q_ROOT *) root;

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
 *  This function is called to determine if the queue in question is empty.
 *  This is done by the flink pointing to is own queue.
 *
 * Input Parameters:
 *  queue:
 *	A pointer to void, which is actually either a Condition Queue or a
 *	Counted Condition Queue.  NOTE: Since the important layout between
 *	these two Condition Queue types is the same, we only have to cast to
 *	the simplest common format.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  false:	The condition queue is NOT empty.
 *  true:	The condition queue IS empty.
 */
bool AXP_CondQueue_Empty(AXP_COND_Q_HDR *queue)
{
    bool retVal;
    AXP_COND_Q_ROOT *parent = (AXP_COND_Q_ROOT *) queue;
    int locked;

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
    retVal = (AXP_COND_Q_ROOT *) parent->flink == parent;

    /*
     * If we locked the mutex above, then unlock it now.
     */
    if (locked == 0)
	pthread_mutex_unlock(&parent->qMutex);

    /*
     * Return what we found back to the caller.
     */
    return (retVal);
}

/*
 * AXP_LoadExecutable
 *  This function is called to open an executable file and load the contents
 *  into the specified buffer.  The executable file is assumed to be an OpenVMS
 *  formatted file, so the first 576 (0x240) bytes are skipped (this is the
 *  image header and is not our concern).  The remaining file is read byte by
 *  byte into the supplied buffer.  The function returns the number of bytes
 *  read.
 *
 *  NOTE:	We only ever read in an executable file and never write one.  So,
 *		there is no companion Unload function.
 *
 * Input Parameters:
 *  fineName:
 *	A string containing the name of the file to read.  It includes the file
 *	path (relative or explicit).
 *  bufferLen:
 *	A value indicating the size of the buffer parameter.  This is used to
 *	prevent us from writing past the end of the buffer.
 *
 * Output Parameters:
 *  buffer:
 *	A pointer to an array of unsigned chars in which the contents of the
 *	executable file are written.
 *
 * Return Values:
 *  AXP_E_FNF:		File not found.
 *  AXP_E_BUFTOOSMALL:	Buffer too small to receive contents of file.
 *  0:			No data in file.
 *  >0			Number of bytes read.
 */
i32 AXP_LoadExecutable(char *fileName, u8 *buffer, u32 bufferLen)
{
    FILE *fp;
    int retVal = 0;
    int ii, jj;
    bool eofHit = false;
    u8 scratch[4];

    if (AXP_UTL_BUFF)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("DECaxp: opened executable file %s for reading",
	        fileName);
	AXP_TraceWrite("");
	AXP_TraceWrite("Header bytes:");
	AXP_TRACE_END();
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
		char traceBuf[256];
		int offset;

		AXP_TRACE_BEGIN();
		offset = sprintf(traceBuf, "%x", scratch[ii]);
		if (((ii + 1) % 4) == 0)
		{
		    offset += sprintf(&traceBuf[offset], "\t");
		    for (jj = sizeof(u32) - 1; jj >= 0; jj--)
			offset += sprintf(&traceBuf[offset], "%c",
			        (isprint(scratch[jj]) ? scratch[jj] : '.'));
		    AXP_TraceWrite("%s", traceBuf);
		}
		AXP_TRACE_END();
	    }
	}

	/*
	 * Continue looking, reading 1 byte at a time, until we either,
	 * reach the end of file (we may already be there), or we run out of
	 * buffer to write into.
	 */
	ii = 0;
	if (AXP_UTL_BUFF)
	{
	    AXP_TRACE_BEGIN();
	    AXP_TraceWrite("Executable bytes:");
	    AXP_TRACE_END();
	}
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
		char traceBuf[256];
		int offset;

		offset = sprintf(traceBuf, "%x", buffer[ii]);
		if (((ii + 1) % 4) == 0)
		{
		    offset += sprintf(&traceBuf[offset], "\t");
		    for (ii = 4; jj > 0; jj--)
			offset += sprintf(&traceBuf[offset], "%c",
			        (isprint(buffer[ii - jj]) ?
			                buffer[ii - jj] : '.'));
		    AXP_TRACE_BEGIN();
		    AXP_TraceWrite("%s", traceBuf);
		    AXP_TRACE_END();
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
    return (retVal);
}

/*
 * AXP_OpenRead_SROM
 *  This function is called to open a Serial ROM file for reading and return
 *  various header information from the file.  If the header information does
 *  not look correct, the file will be closed and a failure indication
 *  returned.
 *
 * Input Parameters:
 *  fileName:
 *	A null-terminated string containing the name of the file to open.
 *
 * Output Parameters:
 *  sromHandle:
 *	A pointer to a location to receive the header information and hold the
 *	file pointer, which is utilized in the read and close functions.
 *
 * Return Values:
 *  false:	Normal successful completion.
 *  true:	An error occurred.
 */
bool AXP_OpenRead_SROM(char *fileName, AXP_SROM_HANDLE *sromHandle)
{
    char *errMsg = "%%AXP-E-%s, %s";
    char *localFileName = "";
    int ii = 0;
    bool retVal = false;

    if (fileName != NULL)
	localFileName = fileName;
    if (AXP_UTL_BUFF)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("Opening SROM file %s for reading", localFileName);
	AXP_TRACE_END();
    }

    /*
     * Initialize the handle structure with all zeros.
     */
    memset(sromHandle, 0, sizeof(AXP_SROM_HANDLE));

    /*
     * Copy the filename into the handle, then try opening the file.
     */
    strcpy(sromHandle->fileName, localFileName);
    sromHandle->fp = fopen(localFileName, "r");

    /*
     * If the file was successfully open, then let's try reading in the header.
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
		    fread(&sromHandle->validPat, sizeof(u32), 1,
			    sromHandle->fp);
		    if (sromHandle->validPat != AXP_ROM_VAL_PAT)
		    {
			if (AXP_UTL_BUFF)
			{
			    AXP_TRACE_BEGIN();
			    AXP_TraceWrite(errMsg, "VPB",
				    "Validity pattern bad.");
			    AXP_TRACE_END();
			}
			retVal = true;
		    }
		    break;

		case 1:
		    fread(&sromHandle->inverseVP, sizeof(u32), 1,
			    sromHandle->fp);
		    if (sromHandle->inverseVP != AXP_ROM_INV_VP_PAT)
		    {
			if (AXP_UTL_BUFF)
			{
			    AXP_TRACE_BEGIN();
			    AXP_TraceWrite(errMsg, "IVPB",
				    "Inverse validity pattern bad.");
			    AXP_TRACE_END();
			}
			retVal = true;
		    }
		    break;

		case 2:
		    fread(&sromHandle->hdrSize, sizeof(u32), 1, sromHandle->fp);
		    if (sromHandle->hdrSize != AXP_ROM_HDR_LEN)
		    {
			if (AXP_UTL_BUFF)
			{
			    AXP_TRACE_BEGIN();
			    AXP_TraceWrite(errMsg, "HDRIVP",
				    "Header size invalid.");
			    AXP_TRACE_END();
			}
			retVal = true;
		    }
		    break;

		case 3:
		    fread(&sromHandle->imgChecksum, sizeof(u32), 1,
			    sromHandle->fp);
		    break;

		case 4:
		    fread(&sromHandle->imgSize, sizeof(u32), 1, sromHandle->fp);
		    break;

		case 5:
		    fread(&sromHandle->decompFlag, sizeof(u32), 1,
			    sromHandle->fp);
		    break;

		case 6:
		    fread(&sromHandle->destAddr, sizeof(u64), 1,
			    sromHandle->fp);
		    break;

		case 7:
		    fread(&sromHandle->hdrRev, sizeof(u8), 1, sromHandle->fp);
		    break;

		case 8:
		    fread(&sromHandle->fwID, sizeof(u8), 1, sromHandle->fp);
		    break;

		case 9:
		    fread(&sromHandle->hdrRevExt, sizeof(u8), 1,
			    sromHandle->fp);
		    break;

		case 10:
		    fread(&sromHandle->res, sizeof(u8), 1, sromHandle->fp);
		    break;

		case 11:
		    fread(&sromHandle->romImgSize, sizeof(u32), 1,
			    sromHandle->fp);
		    break;

		case 12:
		    fread(&sromHandle->optFwID, sizeof(u64), 1, sromHandle->fp);
		    break;

		case 13:
		    fread(&sromHandle->romOffset, sizeof(u32), 1,
			    sromHandle->fp);
		    if (sromHandle->romOffset & 1)
			sromHandle->romOffsetValid = true;
		    sromHandle->romOffset &= ~1;
		    break;

		case 14:
		    fread(&sromHandle->hdrChecksum, sizeof(u32), 1,
			    sromHandle->fp);
		    break;
	    }

	    /*
	     * If we read the end of the file, then this is not a valid SROM
	     * file.
	     */
	    if (feof(sromHandle->fp) != 0)
	    {
		if (AXP_UTL_BUFF)
		{
		    AXP_TRACE_BEGIN();
		    AXP_TraceWrite(errMsg, "BADROM",
			    "This is not a valid ROM file.");
		    AXP_TRACE_END();
		}
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
		AXP_ROM_HDR_LEN,
		false,
		0) == sromHandle->hdrChecksum))
	{
	    if (AXP_UTL_BUFF)
	    {
		AXP_TRACE_BEGIN();
		AXP_TraceWrite(errMsg, "BADCSC", "Header CSC-32 is not valid.");
		AXP_TRACE_END();
	    }
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
	    char buffer[132];
	    u8 *optFwID = (u8 *) &sromHandle->optFwID;

	    AXP_TRACE_BEGIN();
	    AXP_TraceWrite("SROM Header Information:");
	    AXP_TraceWrite("");
	    AXP_TraceWrite("Header Size......... %u bytes",
		    sromHandle->hdrSize);
	    AXP_TraceWrite("Image Checksum...... 0x%08x",
		    sromHandle->imgChecksum);
	    AXP_TraceWrite("Image Size (Uncomp). %u (%u KB)",
		    sromHandle->imgSize, sromHandle->imgSize / ONE_K);
	    AXP_TraceWrite("Compression Type.... %u", sromHandle->decompFlag);
	    AXP_TraceWrite("Image Destination... 0x%016llx",
		    sromHandle->destAddr);
	    AXP_TraceWrite("Header Version...... %d", sromHandle->hdrRev);
	    AXP_TraceWrite("Firmware ID......... %d - %s", sromHandle->fwID,
		    _axp_fwid_str[sromHandle->fwID]);
	    AXP_TraceWrite("ROM Image Size...... %u (%u KB)",
		    sromHandle->romImgSize, sromHandle->romImgSize / ONE_K);
	    buffer[0] = '\0';
	    for (ii = 0; ii < sizeof(sromHandle->optFwID); ii++)
		sprintf(&buffer[strlen(buffer)], "%02d", optFwID[ii]);
	    AXP_TraceWrite("Firmware ID (Opt.).. %s");
	    AXP_TraceWrite("Header Checksum..... 0x%08x",
		    sromHandle->hdrChecksum);
	    AXP_TRACE_END();
	}
    }
    else
    {
	if (AXP_UTL_BUFF)
	{
	    AXP_TRACE_BEGIN();
	    AXP_TraceWrite("Error opening SROM file %s for reading",
		    localFileName);
	    AXP_TRACE_END();
	}
	retVal = true;
    }
    return (retVal);
}

/*
 * AXP_OpenWrite_SROM
 *  This function is called to open a Serial ROM file for writing.
 *
 * Input Parameters:
 *  fileName:
 *	A null-terminated string containing the name of the file to open.
 *
 * Output Parameters:
 *  sromHandle:
 *	A pointer to a location to receive the header information and hold the
 *	file pointer, which is utilized in the read and close functions.
 *
 * Return Values:
 *  false:	Normal successful completion.
 *  true:	An error occurred.
 */
bool AXP_OpenWrite_SROM(char *fileName, AXP_SROM_HANDLE *sromHandle,
        u64 destAddr, u32 fwID)
{
    bool retVal = false;

    if (AXP_UTL_BUFF)
    {
	AXP_TRACE_BEGIN();
	printf("\n\nDECaxp: opening SROM file %s for writing\n\n", fileName);
	AXP_TRACE_END();
    }

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
	sromHandle->optFwID = 0x0200001711212316ll;
	sromHandle->romOffset = 0; /* no ROM offset */
    }
    else
	retVal = true;

    /*
     * Return the result of this call back to the caller.
     */
    return (retVal);
}

/*
 * AXP_Read_SROM
 *  This function is called to read data from the Serial ROM file opened by the
 *  AXP_OpenRead_SROM function.
 *
 * Input Parameters:
 *  sromHandle:
 *	A pointer to a location containing the file pointer to be closed.
 *  bufLen:
 *	A value specifying the maximum number of entries in the 'buf'
 *	parameter.
 *
 * Output Parameters:
 *  buf:
 *	A pointer to a u32 buffer to receive the data next set of data from the
 *	SROM file.
 *
 * Return Values:
 *  AXP_E_EOF:		End-of-file reached.
 *  AXP_E_READERR:	Error reading file.
 *  AXP_E_BADSROMFILE:	Bad SROM file detected.
 *  >0:			Number of bytes successfully read (up to bufLen
 *			unsigned bytes)
 */
i32 AXP_Read_SROM(AXP_SROM_HANDLE *sromHandle, u32 *buf, u32 bufLen)
{
    i32 retVal = 0;
    u32 noop = 0x47ff041f;
    int ii;
    bool done = false;

    /*
     * Make sure we are not at the end of file before trying to read from the
     * SROM file.
     */
    if (feof(sromHandle->fp) == 0)
    {

	/*
	 * Read up to bufLen worth of unsigned bytes.
	 */
	for (ii = 0; ((ii < bufLen) && (done == false)); ii++)
	{
	    if (fread(&buf[ii], sizeof(u32), 1, sromHandle->fp) > 0)
		retVal++;
	    else
		done = true;
	}

	/*
	 * Update the running image CRC.
	 */
	sromHandle->verImgChecksum = AXP_Crc32(
					(u8 *) buf,
					(retVal * sizeof(u32)),
					true,
					sromHandle->verImgChecksum);

	/*
	 * If the done flag got set, then we found the end-of-file before
	 * filling in the buffer.  Fill the remaining part of the buffer with
	 * NOOPs.
	 */
	if (done == true)
	{
	    for (ii = retVal; ii < bufLen; ii++)
		buf[ii] = noop;
	}
	retVal = bufLen;

	/*
	 * If we hit the end-of-file, then inverse the CRC and check it
	 * against the one from the file header.
	 */
	if (feof(sromHandle->fp) != 0)
	{
	    sromHandle->verImgChecksum = ~sromHandle->verImgChecksum;

	    /*
	     * If the newly calculated image checksum does not match the
	     * one read in the header file, then we have a bad SROM file.
	     *
	     * TODO:	This is not working for some reason.  We need to look
	     *			into this.  For now, we'll ignore the error.
	     */
	    if (sromHandle->verImgChecksum != sromHandle->imgChecksum)
	    {
		/* retVal = AXP_E_BADSROMFILE; */
		if (AXP_UTL_CALL)
		{
		    AXP_TRACE_BEGIN();
		    AXP_TraceWrite(
			    "Read Image Checksum 0x%08x does not match read "
				    "in Checksum 0x%08x",
			    sromHandle->verImgChecksum,
			    sromHandle->imgChecksum);
		    AXP_TRACE_END();
		}
	    }
	}
    }
    else
	retVal = AXP_E_EOF;
    return (retVal);
}

/*
 * AXP_Write_SROM
 *  This function is called to write data to the Serial ROM file opened by the
 *  AXP_OpenWrite_SRM function.
 *
 * Input Parameters:
 *  sromHandle:
 *	A pointer to a location containing the file pointer to be closed.
 *  buf:
 *	A pointer to a u8 buffer to receive the data next set of data from the
 *	SROM file.
 *  bufLen:
 *	A value specifying the maximum length of the 'buf' parameter.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	Insufficient memory to allocate a new buffer.
 *  false:	Normal successful completion.
 */
bool AXP_Write_SROM(AXP_SROM_HANDLE *sromHandle, u32 *buf, u32 bufLen)
{
    int ii, newSize;
    bool retVal = false;

    /*
     * [re]allocate the write buffer to be able to store the next bit of data.
     */
    newSize = (sromHandle->imgSize + bufLen) * sizeof(u32);
    sromHandle->writeBuf = realloc(sromHandle->writeBuf, newSize);

    /*
     * If the buffer was reallocated, copy the next chunk of data.
     */
    if (sromHandle->writeBuf != NULL)
    {
	for (ii = 0; ii < bufLen; ii++)
	    sromHandle->writeBuf[sromHandle->imgSize + ii] = buf[ii];
	sromHandle->imgSize += bufLen;
    }
    else
	retVal = true;
    return (retVal);
}

/*
 * AXP_Close_SROM
 *  This function is called to close the Serial ROM file opened by the
 *  AXP_OpenRead_SROM or AXP_OpenWrite_SROM functions.
 *
 * Input Parameters:
 *  sromHandle:
 *	A pointer to a location containing the file pointer to be closed.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  false:	Normal successful completion.
 *  true:	An error occurred.
 */
bool AXP_Close_SROM(AXP_SROM_HANDLE *sromHandle)
{
    const char *errMsg = "%%AXP-E-%s, %s\n";
    int ii;
    u32 imageSize = sromHandle->imgSize * sizeof(u32);
    bool retVal = false;

    /*
     * If we have a non-NULL file pointer, then we need to close it.
     */
    if ((sromHandle->fp != NULL) && (sromHandle->validPat == AXP_ROM_VAL_PAT)
	    && (sromHandle->inverseVP == AXP_ROM_INV_VP_PAT)
	    && (sromHandle->hdrSize == AXP_ROM_HDR_LEN))
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
	    sromHandle->imgChecksum = AXP_Crc32((u8 *) sromHandle->writeBuf,
		    imageSize, true, 0);
	    sromHandle->hdrChecksum = AXP_Crc32((u8 *) &sromHandle->validPat,
	    AXP_ROM_HDR_LEN, false, 0);
	    if (AXP_UTL_BUFF)
	    {
		u8 *optFwID = (u8 *) &sromHandle->optFwID;
		int ii;

		printf("SROM Header Information:\n\n");
		printf("Header Size......... %u bytes\n", sromHandle->hdrSize);
		printf("Image Checksum...... 0x%08x\n",
		        sromHandle->imgChecksum);
		printf("Image Size (Uncomp). %u (%u KB)\n", imageSize,
		        imageSize / ONE_K);
		printf("Compression Type.... %u\n", sromHandle->decompFlag);
		printf("Image Destination... 0x%016llx\n",
		        sromHandle->destAddr);
		printf("Header Version...... %d\n", sromHandle->hdrRev);
		printf("Firmware ID......... %d - %s\n", sromHandle->fwID,
		        _axp_fwid_str[sromHandle->fwID]);
		printf("ROM Image Size...... %u (%u KB)\n",
		        sromHandle->romImgSize, sromHandle->romImgSize / ONE_K);
		printf("Firmware ID (Opt.).. ");
		for (ii = 0; ii < sizeof(sromHandle->optFwID); ii++)
		    printf("%02x", optFwID[ii]);
		printf("\nHeader Checksum..... 0x%08x\n",
		        sromHandle->hdrChecksum);
	    }

	    /*
	     * Write out all the header data.
	     */
	    fwrite(&sromHandle->validPat, sizeof(u32), 1, sromHandle->fp);
	    fwrite(&sromHandle->inverseVP, sizeof(u32), 1, sromHandle->fp);
	    fwrite(&sromHandle->hdrSize, sizeof(u32), 1, sromHandle->fp);
	    fwrite(&sromHandle->imgChecksum, sizeof(u32), 1, sromHandle->fp);
	    fwrite(&imageSize, sizeof(u32), 1, sromHandle->fp);
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
	    for (ii = 0; ii < sromHandle->imgSize; ii++)
		fwrite(&sromHandle->writeBuf[ii], sizeof(u32), 1,
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
    return (retVal);
}

/*
 * AXP_MaskReset
 *  This function is called to reset the indicated mask.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  mask:
 *	A pointer to an 8-bit unsigned integer to have all its bits cleared.
 *
 * Return Value:
 *  None.
 */
void AXP_MaskReset(u8 *mask)
{
    *mask = 0;

    /*
     * return back to the caller.
     */
    return;
}

/*
 * AXP_MaskSet
 *  This function is called to set some bits in a mask, representing occupied
 *  locations within a 64 byte buffer.
 *
 * Input Parameters:
 *  mask:
 *	A pointer containing the current set of masked bits.
 *  basePA:
 *	A 64-bit unsigned value representing the base address of the buffer.
 *  pa:
 *	A 64-bit unsigned value representing the base address of the data
 *	written to the buffer.
 *  len:
 *	A value indicating the length of the data written to the buffer.
 *
 * Output Parameters:
 *  mask:
 *	A pointer with the updated set of bit masks.
 *
 * Return Value:
 *  None.
 */
void AXP_MaskSet(u8 *mask, u64 basePa, u64 pa, int len)
{
    switch (len)
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
 *  This function is called to set up the pointer into the mask to its initial
 *  value.  This pointer is used to determine where we left off for successive
 *  AXP_MaskGets.  The pointer needs to be supplied for each of the AXP_MaskGet
 *  calls.  A call to this function must be performed prior to any set of
 *  AX_MaskGets.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  curPtr:
 *	A pointer to a location indicating where we left off in returning the
 *	set of currently set masks.
 *
 * Return Value:
 *  None.
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
 *  This function is called to return the offset to the next location within
 *  the buffer with the valid data.  The current point into the buffer is set
 *  to the next location.
 *
 * Input Parameters:
 *  curPtr:
 *	A pointer to a location with the current location within the mask to be
 *	parsed.
 *  mask:
 *	A 64-bit value representing the mask being parsed.
 *  len:
 *	A value indicating the length of the data written to the buffer.
 *
 * Output Parameters:
 *  curPtr:
 *	A pointer to a location to receive the next place to look for valid
 *	data within the buffer.  A value of -1 indicate that there is no more
 *	to parse.
 *
 * Return Value:
 *   -1:	Nothing to parse.
 *  >=0:	A value representing the byte offset within a buffer containing
 *		valid data.
 */
int AXP_MaskGet(int *curPtr, u8 mask, int len)
{
    int retVal = -1;
    int curPtrMax;
    u16 maskBits;

    switch (len)
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
	while (((mask & (maskBits << *curPtr)) != (maskBits << *curPtr))
	        && (*curPtr < curPtrMax))
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
	while (((mask & (maskBits << *curPtr)) != (maskBits << *curPtr))
	        && (*curPtr < curPtrMax))
	{
	    *curPtr = *curPtr + 1;
	}
	if (*curPtr == curPtrMax)
	    *curPtr = -1;
    }

    /*
     * Return what we found back to the caller.
     */
    return (retVal);
}

/*
 * AXP_WriteAtOffset
 *  This function is called to write a block of data at a particular offset
 *  within a file.
 *
 * Input Parameters:
 *  fp:
 *	A file pointer.
 *  outBuf:
 *	A pointer to the buffer to be written.
 *  outLen:
 *	A value indicating the length of the buffer to be written.
 *  offset:
 *	A value indicating the offset within the file where the buffer should
 *	be written.
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	Normal Successful Completion.
 *  false:	An error occurred writing to the file.
 */
bool AXP_WriteAtOffset(FILE *fp, void *outBuf, size_t outLen, u64 offset)
{
    bool	retVal = true;

    if (fseek(fp, offset, SEEK_SET) == 0)
    {
	fwrite(outBuf, sizeof(u8), outLen, fp);
	if (ferror(fp) != 0)
	    retVal = false;
    }
    else
	retVal = false;

    /*
     * Return the outcome of this call back to the caller.
     */
    return(retVal);
}
