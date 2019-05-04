/*
 * The idea for the Bcache and Dcache is that the Bcache will be a consolidated
 * cache in that for each index within the Bcache, there will be an entry for
 * each CPU, indexed by CPU ID (whami value).
 *
 * For the Bcache:
 *	What this will allow is that the Bcache will maintain self coherency across
 *	CPU caches.  If an index entry is indicated as "Shared", and update to that
 *	block will be updated in all the other CPU cache entries that have the same
 *	TAG value.  Note, it is possible for an index item to be indicated as
 *	"Shared" when it is not.  When a TAG value is updated at a particular
 *	index, the other TAG values at that same index will be marked as "Shared".
 *	Also note that only one CPU specific entry can be "Shared/Dirty" and the
 *	other entries will be indicated as "Shared/Clean".  When a Bcache block is
 *	updated through the Dcache, the Bcache handling code will deal with
 *	updating the Bcache records in other CPUs (in a controlled manner).
 *
 * For the Dcache:
 *	Since the Dcache is a subset of the Bcache (Every valid entry in the Dcache
 *	must also be in the Bcache - the opposite is not true).  So what the Dcache
 *	will be is the index into the Dcache will contain a TAG of it's own (as the
 *	Dcache and Bcache TAG may be different), and a pointer to the Bcache record
 *	for the specific CPU.  There will not be a direct way for the modification
 *	of a Dcache record in one CPU to update the Bcache record in another CPU.
 */
#include "21264Processor/Caches/AXP_21264_CacheDefs.h"

#define AXP_21264_BLK_SIZE	8	/* 8 quadwords = 64 bytes */

typedef struct
{
    AXP_21264_CACHE_ST state;
    u64 tag;
    u64 blk[AXP_21264_BLK_SIZE];
} AXP_21264_BCACHE_BLK;

typedef AXP_21264_BCACHE_BLK *AXP_21264_DCACHE_BLK;

#define AXP_21264_PAGE_SIZE	8196	/* 8K page size */
typedef struct
{
    pthread_mutex_t pageMutex;
    pthread_cond_t pageCond;
} AXP_21264_PAGE_BLK;

typedef struct
{
    u64 offset :13; /* Offset within page */
    u64 index :51; /* Index to page */
} AXP_21264_PAGE_IDX;

#define AXP_21264_CACHE_BLOCKS(size)	(size) / AXP_21264_BLK_SIZE
#define AXP_21264_CACHE_PAGES(size)		(size) / AXP_21264_PAGE_SIZE
