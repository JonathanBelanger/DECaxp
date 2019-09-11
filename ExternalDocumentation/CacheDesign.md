# Cache Designs for the DECaxp Emulator

## Overview

There are three caches associated with the DEC Alpha 21264 Alpha AXP processor.
These are:

1. Instruction Cache (aka: iCache)
   a. 64KB
   b. virtually tagged and indexed
   c. 2-way set _predict_
   d. Each block contains:
    + 16 instructions (64 bytes)
    + virtual tag bits
    + 8-bit ASN field
    + 1-bit ASM field
    + 1-bit PALcode bit (physical addressing)
    + 1-bit valid
    + 4-bit access check bits (KESU = Kernel, Executive, Supervisor, and User)
    + Predecode information
    + Parity bits (not used in the emulator)
2. Data Cache (aka: dCache)
   a. virtually indexed/physically tagged
   b. 64KB
   c. 2-way set _associative_
   d. Each block contains:
    + 64 data bytes
    + physical tag bits
    + Valid, Dirty, Shared, and Modified bits
    + 1-bit round robin set control
    + Parity bits (not used in the emulator)
3. Secondary Cache (aka: bCache or L2 cache)
   a. physically tagged and indexed
   b. 1-16MB
   c. Each block contains:
    + 64 data bytes
    + physical tag bit
    * Valid, Dirty, Shared bits

## System Characteristics

### Addressing

1. Virtual addressing is either 48- or 43-bits long.
2. Physical addressing is 44-bits long.
3. 8192 (8KB) page size

# High-level Cache Architecture

There are a number of commonalities across the various caches within the 21264
processor.  They are:

1. 64-byte blocks
2. 1 valid status bit

Beyond that we have various differences.  These differences are:

1. 2-way sets for iCache and dCache, not sets for bCache
   a. iCache is set-prediction and dCache is set-associative
2. iCache is virtually indexed/tagged, dCache is virtually indexed/physically
   tagged, and bCache is physically indexed/tagged.
3. dCache and bCache have dirty and shared status bits, the iCache does not
4. iCache also has 8-bit ASN, 1-bit ASM, 1-bit PALcode, 4-bits access (KESU),
   and additional pre-decode information
5. dCache has 1-bit per set-pair for each indexed entry

# Design

The following is the design proposed for all three caches.  The following
structure represents a single cache block.  It contains the necessary tag and
bits required to support each of the cache types.  This structure will be part
of a parent cache block.

``` {.c}
struct cacheBlk
{
    union
    {
        u8 block[64];
        AXP_INS_FMT instructions[16];
    } _blk;
    union
    {
        struct
        {
            u64 tag;                             /* Virtual tag */
            u8  kre : 1;                 /* Kernel read/execute */
            u8  ere : 1;              /* Executive read/execute */
            u8  sre : 1;             /* Supervisor read/execute */
            u8  ure : 1;                   /* User read/execute */
            u8  _asm : 1;                /* Address Space Match */
            u8  pal : 1;        /* PALcode (physical addressing */
            u8  valid : 1;                         /* Valid bit */
            u8  res_1 : 1;
            u8  asn;                    /* Address Space Number */
            u16 res_2;                      /* 32-bit alignment */
        } iCache;
        struct
        {
            u64 tag;                            /* Physical tag */
            u8 res_1 : 2;
            u8 modified : 1;                    /* Modified bit */
            u8 storePending : 1;     /* Store operation pending */
            u8 dirty : 1;                          /* Dirty bit */
            u8 shared : 1;                        /* Shared bit */
            u8 valid : 1;                          /* Valid bit */
            u8 evictPending : 1;         /* Eviction is pending */
            u8 res_3;                       /* 16-bit alignment */
            u16 res_4;                      /* 32-bit alignment */
        } dCache;
        struct
        {
            u64 tag;                            /* Physical tag */
            u8 res_1 : 4;
            u8 dirty : 1;                          /* Dirty bit */
            u8 shared : 1;                        /* Shared bit */
            u8 valid : 1;                          /* Valid bit */
            u8 res_2 : 1;                    /* 8-bit alignment */
            u8 res_3;                       /* 16-bit alignment */
            u16 res_4;                      /* 32-bit alignment */
        } bCache;
    } _bits;
};
```

**Note**: The valid bit is always the 7th-bit after the tag.  Also, the shared
and dirty bits immediately precede the valid bit for the dCache and bCache.

The following structure is the parent structure in support of all the possible
cache configurations.

``` {.c}
enum cacheType {iCache, dCache, bCache};
struct cache
{
    struct cacheBlk *blk;             /* address of cache block */
    pthread_mutex_t *mutex;  /* Used for multithreading support */
    struct ctag *ctag  /* Duplicate Tag Array for Cbox (dCache) */
    u32 cacheSize;                 /* size, in 64-bytes chuncks */
    u16 nextSet;                         /* next set to be used */
    u16 writeLock : 1;  /* Array locked for writing (exclusive) */
    u16 res : 15;                           /* 16-bit alignment */
    enum cacheType type;        /* type of cache blk represents */
};
```

The following structure is the Duplicate dCache Tag (DTAG) Array, used by the
Cbox, called the CTAG (note: The use of CTAG is not consistent in the
documentation).  It contains the information the Cbox needs to respond to
probes from the system, dCache fill requests from the Mbox. and iCache fill
requests from the Ibox.  The big difference is that the CTAG array is
physically indexed.

``` {.c}
struct ctagItem
{
    u64 tag;                                    /* Physical tag */
    u8 res_1 : 2;
    u8 modified : 1;                            /* Modified bit */
    u8 storePending : 1;             /* Store operation pending */
    u8 dirty : 1;                                  /* Dirty bit */
    u8 shared : 1;                                /* Shared bit */
    u8 valid : 1;                                  /* Valid bit */
    u8 res_2 : 1;                            /* 8-bit alignment */
    u8 res_4;                               /* 16-bit alignment */
    u16 dCacheSet;                                /* dCache Set */
    u32 dCacheIdx;                              /* dCache Index */
};

struct ctag
{
    struct ctagItem cTag[1024];
    struct cache *dCache;  /* A pointer to the dCache structure */
    pthread_mutex_t *mutex;  /* Used for multithreading support */
    u32 writeLock : 1;  /* Array locked for writing (exclusive) */
    u32 res : 31;                           /* 32-bit alignment */
};
```

The entries in this structure are maintained by the Cbox and updated as needed
by the Mbox.  When the Cbox is requested by the Mbox to perform a fill, the
Cbox looks in its CTAG to determine if the block is already in the dCache, but
at another location (this can happen because a particular virtually indexed
block can reside in one of four possible locations).  If so, the Cbox
invalidates the existing line in the Dcache, potentially writing it to the
bCache.

## Cache state support

A cache block has the following cache state when certain bits are set or clean.
The list of cache states and the bits required to be set or clear.

| Cache State | Valid | Shared | Dirty |
|-------------|-------|--------|-------|
| Invalid | Clear | Any | Any|
| Clean | Set | Clear | Clear |
| Dirty | Set | Clear | Set |
| Shared/Clean | Set | Set | Clear |
| Shared/Dirty | Set | Set | Set |

## Cache Indexing and Set Selection

### iCache

#### Basics

The iCache is 64KB in size and is split up into two sets.  The iCache is
virtually indexed and virtually tagged Successive blocks of instructions will
occupy neighboring sets and lines.  Because all instructions are 32-bits in
size, the low two bits are not used in the addressing.  The low bit is used to
determine when an instruction is to be run in PALmode or not.  The next higher
bit is reserved.  Each block contains 16 instructions, or 64 bytes.  Therefore,
there are 1024 (1K) blocks, in 2 sets, so indexing will be performed to 512
entries.  So what we have is as follows:

```
Instruction 0 is at address  0x00000000:00000000
Instruction 1 is at address  0x00000000:00000004
Instruction 2 is at address  0x00000000:00000008
    .
    .
    .
Instruction 15 is at address 0x00000000:0000003c
Instruction 16 is at address 0x00000000:00000040
```

Then it follows that:

iCache index 0 set 0 will contain instructions for addresses 0x00000000 to
0x0000003c, and index 0 set 1 will contain instructions for addresses
0x00000040 to 0x0000007c.  The tag for the cache entry will be taken from bits
47 through 15, inclusive, of the virtual address.

The address mapping in the iCache is:

```
6 6 6 6 5 5 5 5 5 5 5 5 5 5 4 4 4 4 4 4 4 4 4 4 3 3 3 3 3 3 3 3 3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
+-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
+-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
                                        |<-------------------------- tag ---------------------->|<-- cache index -->|<64 byte>|
```

**Note**: The index for both the iCache and dCache are from the virtual
address.  The index for bCache and CTAG array are from the physical address.
The tag for the iCache is also from the virtual address.  The tag for the
dCache, bCache, and CTAG array are from the physical address.

#### Set prediction

The set prediction mechanism being proposed requires certain information to
determine the instruction previously fetched and if it is a branch that is
predicted to be taken.  The following steps are performed:

1. If the instruction is a branch:
   a. If the branch is likely to be taken,
    + then the prediction is set 0 is selected for the next instruction set
      to be fetched.
   b. If the instruction is not likely to be taken,
    + then the prediction is that the next set will be taken
        - next index and set 0
        - current index and set 1
2. If the instruction is not a branch,
   a. then the next set will be 
    + the current index and set 1
    + the next index and set 0.

#### iCache API

The interface for the iCache is as follows can comprises all the functionality
required by the cache implementation to support the iCache.

``` {.c}
/*
 * AXP_iCache_Init
 *  This function is called to initialize the Instruction Cache.
 *
 * Input Parameters:
 *  iCache:
 *      This is a pointer to a cache structure to be initialized.
 *  blk:
 *      This is a pointer to an appropriately sized iCache block array.  The
 *      size of the iCache is 64KB, which will store 1K (1024) or 64 byte
 *      blocks in 2 sets.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_iCache_Init(struct cache *iCache, struct cacheBlk *blk);

/*
 * AXP_iCache_Fill
 *  This function is called to add a block of instructions to the iCache using
 *  the baseAddr to determine the index into the iCache cache array.  The itb
 *  parameter is used to set the cache settings (bits).  The Intruction
 *  Translation Lookaside Buffer (TLB & ITB) must have been determined prior to
 *  calling this function.  Instructions are always filled in at set 0.
 *
 * Input Parameters:
 *  iCache:
 *      This is a pointer to a cache structure.
 *  index:
 *      This is the index value into the iCache and comes from the virtual
 *      address of the PC.
 *  set:
 *      This is the set into which the data is to be stored.
 *  inst:
 *      This is a pointer to the set of instructions to be put into the iCache
 *      to be potentially fetched later.  There are 16-instructions (64 bytes)
 *      in the set of instructions.
 *  tlb:
 *      This is a pointer to the ITB associated with the set of instructions.
 *      It is used to set various bits in the iCache block.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_iCache_Fill(struct cache *iCache,
                     u16 index,
                     u8 set,
                     AXP_INS_FMT *inst,
                     AXP_TLB *itb);

/*
 * AXP_iCache_Fetch
 *  This function is called to fetch the next 4 instructions from the iCache.
 *  It uses the addr parameter to determine the index where the instructions
 *  should be located.  If the cache line is not valid or the tag does not
 *  match at the set predicted, then the other set is looked at.  If that cache
 *  line is not valid or the tag does not match, then a NULL is returned.
 *  Otherwise, the next set of 4 instructions are return to the caller.  The
 *  branch and predicted parameters are used to determine if we should first be
 *  looking in set 0 or the next set (0 or 1).
 *
 * Input Parameters:
 *  iCache:
 *      This is a pointer to a cache structure.
 *  index:
 *      This is the index value into the iCache and comes from the virtual
 *      address being accessed.
 *  set:
 *      This is the set from which the data is to be retrieved.
 *  branch:
 *      This is a boolean used to indicate that the previous set of
 *      instructions returned to the caller had a branch instruction in it.
 *  predicted:
 *      This is a boolean used to indicate that the previous branch has been
 *      predicted to be taken.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:   The instructions were not found in the iCache.
 *  !NULL:  A pointer to the next 4 instructions.
 */
AXP_INS_FMT *AXP_iCache_Fetch(struct cache *iCache,
                              u16 index,
                              u8 set,
                              bool branch,
                              bool predicted);

/*
 * AXP_iCache_Flush
 *  This function is called to flush the iCache.  If the flushAsm parameter is
 *  set to true, then only iCache entries with the _asm bit set will be
 *  flushed.  Otherwise, the entire iCache will be flushed.
 *
 * Input Parameters:
 *  iCache:
 *      This is a pointer to a cache structure.
 *  flushAsm:
 *      This is a boolean used to indicate that the iCache entries with the
 *      _asm bit set will be flushed (true) or all entries will be flushed
 *      (false).
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_iCache_Flush(struct cache *iCache, bool flushAsm);

/*
 * AXP_iCache_Status
 *
 * Input Parameters:
 *  iCache:
 *      This is a pointer to a cache structure.
 *  vpc:
 *      This is the 64-bit virtual PC associated with the cache block
 *      containing the instructions needing to be fetched.
 *
 * Output Parameters:
 *  status:
 *      A pointer to a location to receive a mask of the following values:
 *          CacheMiss   - instructions not in cache
 *          CacheHit    - instructions in cache
 *  index:
 *      A pointer to a location to receive the index of the iCache item where
 *      the data has been found.
 *  set:
 *      A pointer to a location to receive which set of the iCache the item has
 *      been found.
 *
 * Return Values:
 *  None.
 */
void AXP_iCache_Status(struct cache *iCache,
                       u64 vpc,
                       u32 *status,
                       u16 *index,
                       u8 *set);
```

### dCache

#### Basics

The dCache is 64KB in size and is split up into two sets.  The dCache is
virtually indexed, like the iCache, but physically tagged.  There is also a
duplicate tag array utilized by the Cbox that is also maintained by cache
code, called the CTAG array.  The CTAG array is physically indexed and
physically tagged.  Because of the potential for multiple readers and writers,
the locking mechanism will provide the ability to have multiple shared readers
and a single shared writer.  As such, deadlocking the locks between the dCache
and CTAG array needs to be avoided.  The following possible locking states will
be considered:

|               | CTAG reader | CTAG writer |
|---------------|-------------|-------------|
| dCache reader |     Yes     |     No      |
| dCache writer |     Yes     |     Yes     |

**Note**: Because the dCache and CTAG arrays need to be kept in-sync, there
should never be a dCache reader and CTAG writer.  Since it is possible to
update a dCache entry but no need to update the bits maintained in the
associated CTAG entry, it is possible to have a dCache writer and not a CTAG
writer.

The page size for the Alpha AXP processor is defined as being 8KB in size.
This represents 13 bits of virtual address.  This information is used by the
DTB to translate addresses and store them in the dCache, where the remaining
bits of the virtual address represent the virtual page number (VPN).  The
combination of the address bit requirements for the 64 byte block and the index
is 15 bits.  This means that a particular virtual address might be found in
4 unique locations within the dCache, depending upon the virtual to physical
translation of these 2 bits.  The dCache code will need to prevent this
potential aliasing by keeping only 1 of the 4 possible translated addresses in
the dCache at any time.

The address mapping in the dCache is:

```
6 6 6 6 5 5 5 5 5 5 5 5 5 5 4 4 4 4 4 4 4 4 4 4 3 3 3 3 3 3 3 3 3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
+-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
+-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
                                        |<-------------------------- tag ---------------------->|<-- cache index -->|<64 byte>|
|<-------------------------------------- virtual page number -------------------------------------->|<------- 8KB page ------>|
```

#### Set prediction

Set prediction for the dCache is round-robin.  The set 0 entry at any
particular index maintains an indicator of the next set to be used at that
location.  This indicator is a single bit that has a boolean not performed on
it for each fill operation performed on that row.  Normal reads, writes and
address space match flushed do not modify this bit.  A complete flush will
cause this bit to be set to 0 (false).

#### dCache API

The interface for the dCache is as follows can comprises all the functionality
required by the cache implementation to support the dCache.

``` {.c}
/*
 * AXP_dCache_Init
 *  This function is called to initialize the Data Cache and CTAG array.
 *
 * Input Parameters:
 *  dCache:
 *      This is a pointer to a cache structure to be initialized.
 *  blk:
 *      This is a pointer to an appropriately sized dCache block array.  The
 *      size of the dCache is 64KB, which will store 1K (1024) or 64 byte
 *      blocks in 2 sets.
 *  ctag:
 *      This is a pointer to tghe CTAG to be used by the caching code to be
 *      able to keep the dCache and CTAG array synchronized.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_dCache_Init(struct cache *dCache,
                     struct cacheBlk *blk,
                     struct ctag *ctag);

/*
 * AXP_dCache_Fill
 *  This function is called to add a block of data to the dCache using the
 *  the index and set to store the supplied 64-bytes of data.  The index and
 *  set are known because an attempt to write to or read from the cache line
 *  got a miss, and the work has been previously done to determine what the
 *  index and set should be.  This function is called by the Cbox, as a
 *  response to a fill request by the Mbox.
 *
 * Input Parameters:
 *  dCache:
 *      This is a pointer to a cache structure.
 *  index:
 *      This is the index value into the dCache.
 *  set:
 *      This is the set value into the dCache.
 *  block:
 *      This is a pointer to a 64-byte block to be stored in the dCache.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_dCache_Fill(struct cache *dCache,
                     u16 index,
                     u16 set,
                     u8 *block);

/*
 * AXP_dCache_Fetch
 *  This function is called to fetch a value from the dCache.  This function
 *  will always succeed because a call to AXP_dCache_Status should have been
 *  performed to indicate if the fetch would succeed.  If this is not the case
 *  then this call should not occur.
 *
 * Input Parameters:
 *  dCache:
 *      This is a pointer to a cache structure.
 *  index:
 *      This is the index value into the dCache and comes from the virtual
 *      address being accessed.
 *  set:
 *      This is the set from which the data is to be retrieved.
 *  len:
 *      This is the length of data being requested.  It can only be a power
 *      of 2, between 1 and 8, inclusively.
 *
 * Output Parameters:
 *  data:
 *      A pointer to the location into to have the data in the cache stored.
 *      This parameter must be of the type associated with the len parameter.
 *      There is no attempt to sign extend the returned data.
 *
 * Return Values:
 *  None.
 */
void AXP_dCache_Fetch(struct cache *dCache,
                      u16 index,
                      u8 set,
                      u8 len,
                      u8 *data);

/*
 * AXP_dCache_Write
 *  This function is called to write a value into the dCache.  This function
 *  will always succeed because a call to AXP_dCache_Status should have been
 *  performed to indicate if the write would succeed.  If this is not the case
 *  then this call should not occur.
 *
 * Input Parameters:
 *  dCache:
 *      This is a pointer to a cache structure.
 *  index:
 *      This is the index value into the dCache and comes from the virtual
 *      address being accessed.
 *  set:
 *      This is the set from which the data is to be written.
 *  len:
 *      This is the length of data being written.  It can only be a power
 *      of 2, between 1 and 8, inclusively.
 *  data:
 *      A pointer to the location of the data to be written to the cache.  This
 *      parameter must be of the type associated with the len parameter.
 *      There is no attempt to sign extend the supplied data.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_dCache_Write(struct cache *dCache,
                      u16 index,
                      u8 set,
                      u8 len,
                      u8 *data);

/*
 * AXP_dCache_Flush
 *  This function is called to flush the entire dCache.  If an entry in the
 *  dCache has it's modified bit set, it will be sent to the Cbox to be written
 *  to the bCache and potentially memory.
 *
 * Input Parameters:
 *  dCache:
 *      This is a pointer to a cache structure.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_dCache_Flush(struct cache *dCache);

/*
 * AXP_dCache_Status
 *  This function performs a number of functions.  It should be called prior to
 *  calling AXP_dCache_Fetch to make sure that the cache block is in the state
 *  required for the operation.  Additionally, it is able to return an
 *  indicator that there is an alignment issue.  Doing this here means that we
 *  do not have to do it in any of the other functions.  This function will
 *  return the status whether there is an alignment exception or not.  It is up
 *  to the caller to determine if the exception should be ignored or not.
 *
 * Input Parameters:
 *  dCache:
 *      This is a pointer to a cache structure.
 *  va:
 *      This is the 64-bit virtual address associated with the cache block
 *      looking to be accessed.
 *  pa:
 *      This is the 64-bit physical address translation of the virtual address.
 *      It is used for tag matching.
 *  len:
 *      This is the length of data that is going to be requested.  It can only
 *      be a power of 2, between 1 and 8, inclusively, and is used to determine
 *      if there would be an alignment fault.
 *  evictNext:
 *      This is a boolean value use to indicate that the next access of the
 *      selected cache block should cause an eviction.
 *
 * Output Parameters:
 *  status:
 *      A pointer to a location to receive a mask of the following values:
 *          CacheMiss   - data not in cache
 *          CacheHit    - data in cache (Clean, Clean/Shared, Dirty,
 *                        Dirty/Shared)
 *          CacheDirty  - Indicates that the data is in a dirty state.
 *          CacheShared - Indicates that the data is in a shared state.
 *  index:
 *      A pointer to a location to receive the index of the dCache item where
 *      the data has been found.
 *  set:
 *      A pointer to a location to receive which set of the dCache the item has
 *      been found.
 *
 * Return Values:
 *  NoException:        No alignment issues.
 *  DataAlignmentTrap:  Accessing the data may cause an alignment trap.
 */
AXP_EXCEPTIONS AXP_dCache_Status(struct cache *dCache,
                                 u64 va,
                                 u64 pa,
                                 u8 len,
                                 bool evictNext,
                                 u32 *status,
                                 u16 *index,
                                 u8 *set);

/*
 * AXP_dCache_CTAG_Status
 *  This function is called by the Cbox to get certain peices of information.
 *  When the Mbox has sent a fill request to the Cbox, the Cbox calls this
 *  function to determine if the block is already in the cache.  This can
 *  happen because there is an 2-bit overlap between the Virtual Page Number
 *  (VPN) and the cache index.  This means that a particular physical address
 *  can be in 1 of 4 possible locations within the dCache.  If the cBox
 *  determines that the physical address is already in the dCache, it
 *  invalidates that entry, writing the entry to the bCache if its modified bit
 *  is set.
 *
 * Input Parameters:
 *  ctag:
 *      This is a pointer to a ctag array.
 *  pa:
 *      This is the 64-bit physical address translation of the virtual address.
 *      It is used for tag matching.
 *
 * Output Parameters:
 *  status:
 *      A pointer to a location to receive a mask of the following values:
 *          CacheMiss       - data not in cache
 *          CacheHit        - data in cache (Clean, Clean/Shared, Dirty,
 *                            Dirty/Shared)
 *          CacheDirty      - Indicates that the data is in a dirty state.
 *          CacheShared     - Indicates that the data is in a shared state.
 *  ctagIndex:
 *      A pointer to a location to receive the index of the CTAG item where
 *      the data has been found.
 *
 * Return Values:
 *  None.
 */
void AXP_dCache_CTAG_Status(struct ctag *ctag,
                            u64 pa,
                            u32 *status,
                            u16 *ctagIndex);

/*
 * AXP_dCache_Invalidate
 *  This function is called to invalidate a cache line in the dCache because
 *  the Cbox has determined that the virtual address of this block one inwhich
 *  its physical address is being requested to be stored in another location in
 *  the dCache.  This is to prevent aliasing of the virtual/physical addresses.
 *  If this function returns a true, then the invalidated block was in a
 *  modified state and should be written to the bCache.
 *
 * Input Parameters:
 *  ctag:
 *      This is a pointer to a ctag array.
 *  index:
 *      This is the index into the CTAG array returned on the call to
 *      AXP_dCache_CTAG_Status.
 *
 * Output Parameters:
 *  buf:
 *      This is a pointer to a 64-byte block which may receive the contents of
 *      the dCache block, if the modified bit is set.  If this function returns
 *      a value of true, then this parameter contains the data that needs to be
 *      written to the bCache.
 *
 * Return Values:
 *  false:  The cache line was not in a modified state, so the contents buf
 *          parameter do not contain any valid data.
 *  true:   The cache line was in a modified state, so the content of the buf
 *          must be written to the bCache.
 */
bool AXP_dCache_Invalidate(struct ctag *ctag,
                           u16 index,
                           u8 *buf);

/*
 * AXP_dCache_Evict
 *  This function is called to evict a data cache block from the cache.  If the
 *  block has its modified bit set, then it will be sent to the cBox to be
 *  written to the bCache and possibly memory.  If there is a store operation
 *  pending, then the block will be marked for eviction after the store
 *  completes.
 *
 * Input Parameters:
 *  dCache:
 *      This is a pointer to a cache structure.
 *  index:
 *      This is the index value into the dCache and comes from the virtual
 *      address being accessed.
 *  set:
 *      This is the set from which the data is to be retrieved.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_dCache_Evict(struct cache *dCache, u16 index, u8 set);

/*
 * AXP_dCache_StorePending
 *  This function is called to lock a data cache block in the cache from being
 *  evicted because a store instruction is pending but has not yet completed.
 *
 * Input Parameters:
 *  dCache:
 *      This is a pointer to a cache structure.
 *  index:
 *      This is the index value into the dCache and comes from the virtual
 *      address being accessed.
 *  set:
 *      This is the set from which the data is to be retrieved.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_dCache_Lock(struct cache *dCache, u16 index, u8 set);
```

### bCache

#### Basics

The bCache is the simplest of the three caches.  Only the Cbox accesses the
bCache, additionally, the bCache is not broken up into sets.  The bCache can be
between 1- and 16-MB in size.  The bCache is physically indexed and physically
tagged.  Each block in the bCache is 64B in size.  Because the size of the
bCache is not fixed, the number of pits used for index and tag adjust
accordingly.  The address mapping in the bCache is:

```
6 6 6 6 5 5 5 5 5 5 5 5 5 5 4 4 4 4 4 4 4 4 4 4 3 3 3 3 3 3 3 3 3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
+-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
+-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
                                        |<-------------- tag -------------->|<---------- 1MB cache index ---------->|<64 byte>|
                                        |<------------- tag ------------->|<----------- 2MB cache index ----------->|<64 byte>|
                                        |<------------ tag ------------>|<------------ 4MB cache index ------------>|<64 byte>|
                                        |<----------- tag ----------->|<------------- 8MB cache index ------------->|<64 byte>|
                                        |<---------- tag ---------->|<------------- 16MB cache index -------------->|<64 byte>|
```

#### bCache API

The interface for the bCache is as follows can comprises all the functionality
required by the cache implementation to support the bCache.

``` {.c}
/*
 * AXP_bCache_Init
 *  This function is called to initialize the Second Level Cache (bCache).
 *
 * Input Parameters:
 *  bCache:
 *      This is a pointer to a cache structure to be initialized.
 *  blk:
 *      This is a pointer to an appropriately sized bCache block array.
 *  cacheSize:
 *      This is value indicating the size fo the cache block.  The size is the
 *      number of megabytes (MB) and must be one of 1, 2, 4, 8, or 16.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_bCache_Init(struct cache *bCache,
                     struct cacheBlk *blk,
                     u32 cacheSize);

/*
 * AXP_bCache_Fill
 *  This function is called to add/update a block of data to the bCache using
 *  the physical address to store/modify the supplied 64-bytes of data and/or
 *  the bCache entry status bits.
 *
 * Input Parameters:
 *  bCache:
 *      This is a pointer to a cache structure.
 *  pa:
 *      This is the physical address assocaited with the data being requested.
 *  block:
 *      This is a pointer to a 64-byte block to be stored in the bCache.
 *  status:
 *      A value specifying a mask of the following values:
 *          CacheMiss   - data not in cache
 *          CacheHit    - data in cache (Clean, Clean/Shared, Dirty,
 *                        Dirty/Shared)
 *          CacheDirty  - Indicates that the data is in a dirty state.
 *          CacheShared - Indicates that the data is in a shared state.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_bCache_Fill(struct cache *bCache,
                     u64 pa,
                     u8 *block,
                     u32 status);

/*
 * AXP_bCache_Fetch
 *  This function is called to fetch a value from the bCache.  This function
 *  will always succeed because a call to AXP_bCache_Status should have been
 *  performed to indicate if the fetch would succeed.  If this is not the case
 *  then this call should not be made.
 *
 * Input Parameters:
 *  bCache:
 *      This is a pointer to a cache structure.
 *  pa:
 *      This is the physical address assocaited with the data being requested.
 *
 * Output Parameters:
 *  data:
 *      A pointer to a 64-byte location to receive the data requested.
 *
 * Return Values:
 *  None.
 */
void AXP_bCache_Fetch(struct cache *bCache,
                      u64 pa,
                      u8 *data);

/*
 * AXP_bCache_Flush
 *  This function is called to flush the entire bCache.
 *
 * Input Parameters:
 *  bCache:
 *      This is a pointer to a cache structure.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_bCache_Flush(struct cache *bCache);

/*
 * AXP_bCache_Status
 *  This function is called to return the status of teh cache block associated
 *  with the physical address.
 *
 * Input Parameters:
 *  bCache:
 *      This is a pointer to a cache structure.
 *  pa:
 *      This is the 64-bit physical address associated with the cache block.
 *
 * Output Parameters:
 *  status:
 *      A pointer to a location to receive a mask of the following values:
 *          CacheMiss   - data not in cache
 *          CacheHit    - data in cache (Clean, Clean/Shared, Dirty,
 *                        Dirty/Shared)
 *          CacheDirty  - Indicates that the data is in a dirty state.
 *          CacheShared - Indicates that the data is in a shared state.
 *
 * Return Values:
 *  None
 */
void AXP_bCache_Status(struct cache *bCache,
                       u64 pa,
                       u32 *status)
```

# Usage

## Initialization

When the emulator starts (initializes) it presets a number of internal
structures as well as each of the caches.  The very first call to the API
should be to the AXP_`x`Cache_Init routines.  All the memory used for the
caches should be pre-allocated to the appropriate size.  The following should
be provided to each of the initialization routines:

* AXP_iCache_Init
  + The address of the cache structure for the iCache
  + The address of the array of 1024 cacheBlk structures for the iCache
* AXP_dCache_Init
  + The address of the cache structure for the dCache
  + The address of the array of 1024 cacheBlk structures for the dCache
  + The address of the ctag structure
* AXP_bCache_Init
  + The address of the cache structure for the bCache
  + The address of the array of cacheBlk structures for the bCache
  + The value representing the size of the bCache, in megabytes.

The data structures do not need to be initialized, as the initialization
routines will fully initialize all structures and arrays.  It is important that
these structures and arrays are not modified outside of the interface, as the
interface is fully responsible for keeping the caches synchronized.

**Note**: The initial load of the iCache comes from the external Serial ROM
(SROM).  The SROM provides the only means to configure the system interface.
This iCache code must contain PALcode that starts at location 0x780.  This
code is used to configure the CPUs IPRs as necessary prior to any off-chip
read or write operations.  After configuring the CPU, control is transferred to
code anywhere in memory, including non-cacheable regions.  The iCache should be
flushed by a write to the ITB invalidate all register after control is
transferred.  This control should be to addresses not loaded in the iCache by
the SROM, or the iCache may provide unexpected instructions.

## Initial Load

After the caches have been initialized, the Cbox needs to load the SROM data
into the iCache.  This SROM code is PALcode, that is used to configure the
CPU's IPRs.  The SROM code is also responsible for loading the console firmware
into memory and branching to it.

## iCache Usage

After the initial load, the iCache is flushed.  The next access will cause a
cache miss.  This will begin the normal processing for the iCache.

When the Ibox needs the next set of 4 instructions, the following steps must be
performed:

1. Check that the instructions are in the iCache by calling AXP_iCache_Status.
2. If they are, then call AXP_iCache_Fetch to retrieve them by calling
   AXP_iCache_Fetch, specifying the index and set returned in step 1.
3. If they are not, then perform the following:
    a. Get the virtual-to-physical address translation for the virtual program
       counter (VPC).
    b. Send a fill request to the Cbox, indicating the index and set where the
       64-byte block of instructions associated with that address are to be
       stored, return in step 1.
    c. After the Cbox finalizes the fill request, the Ibox calls
       AXP_iCache_Fetch, specifying the index and set returned in step 1.

## dCache Usage

TBD

## bCache Usage

TBD

