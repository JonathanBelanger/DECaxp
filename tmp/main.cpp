#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <unistd.h>
#include <string.h>
#include <cstring.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <cstdlib.h>
#include <stdlib.h>
#include <sstream.h>
#include <iomanip.h>
#include <stddef.h>
#include "common.h"
#include <math.h>
#include "funcs.h"
#include "cacheline.cpp"

extern bool split;
extern int offsetbits_tot;
extern int tagbits_tot;
extern int indexbits_tot;
extern int mem_capacity;
extern int associativity;
extern int totalWords;
extern unsigned int* ramAllocation;
extern int ramSize;
extern int missRead, missWrite;
extern CacheSet *chacheAllocation;
extern CacheSet *chacheAllocationIns;
extern int blockSize;
extern std::string traceFile;
extern int total_Read;
extern int total_Write;
extern int hit, miss;
extern int chacheCapacity, chacheSize;
extern int totalChacheBlocks;
extern int dirtyBlockEvictionCounter;

/*

 */

#define SIZEOFINT sizeof(int)
#define MEM_READ  0
#define MEM_WRITE 1
#define INS_READ  2

void memoryOperations(int ramSize, char *get_argvs[]);
void loadBlock(CacheSet *cacheset, int setNumber, int blockNumber,
		cacheline line, int ramaddress);
void writeBlock(CacheSet *cacheset, int setNumber, int blockNumber,
		cacheline line, int ramaddress);
void updateDataCache(CacheSet *cacheset, int setNumber, int freeBlockNo,
		int offset, unsigned int data, cacheline line, int ramaddress);
void displayStatistics();

/*
 * Find the cache clock with the specific tag.
 */
int findBlockNumber(CacheSet *cacheset, int setNumber, char* tag)
{
	int blockNo = -1;
	for (int i = 0; i < associativity; i++)
	{
		if (strcmp(cacheset[setNumber].set[i].tag, tag) == 0)
		{
			blockNo = i;
			cacheset[setNumber].updateLRU(i);	// no longer LRU
			break;
		}
	}

	/*
	 * Return what we found, if anything, back to the caller.
	 */
	return(blockNo);
}

/*
 * We are looking for an "invalid" block (which is one that has not yet been
 * allocated).
 */
int findVacantBlock(CacheSet *cacheset, int setNumber)
{
	int blockNo = -1;

	/*
	 * Search through each set for the current line for an "invalid" block.
	 */
	for (int i = 0; i < associativity; i++)
	{
		if (cacheset[setNumber].set[i].v == 0)
		{
			blockNo = i;
			break;
		}
	}

	/*
	 * Return what we found back to the caller (could be we found nothing).
	 */
	return(blockNo);
}

/*
 * Return the least recently used cache clock.
 */
int findMinimumLruBlock(CacheSet *cacheset, int setNumber)
{
	return(cacheset[setNumber].minimumLRUBlock());
}

void updateDataCache(
		CacheSet *cacheset,
		int setNumber,
		int freeBlockNo,
		int offset,
		unsigned int data,
		cacheline line,
		int ramaddress)
{
	int ramIndex;

	try
	{
		cacheset[setNumber].set[freeBlockNo].v = 1;
		strcpy(cacheset[setNumber].set[freeBlockNo].tag, line.tag);
		cacheset[setNumber].set[freeBlockNo].data[offset] = data;
		ramIndex = ramaddress;
		cacheset[setNumber].set[freeBlockNo].fromAddress = ramIndex;
	} 
	catch (char* msg)
	{
		std::cout << "Exception" << msg;
	}
}

void loadBlock(
		CacheSet *cacheset,
		int setNumber,
		int blockNumber,
		cacheline line,
		int ramaddress)
{
	cacheline temp = line;
	int ramIndex = ramaddress - (ramaddress % blockSize);

	for (int i = 0; i < blockSize; i++)
	{
		updateDataCache(
			cacheset,
			binaryToInteger(line.index),
			blockNumber,
			i,
			readDataRamInt(ramIndex + (i)),
			line,
			ramaddress);
	}
}

void writeBlock(
		CacheSet *cacheset,
		int setNumber,
		int blockNumber,
		cacheline line,
		int ramaddress)
{
	cacheline temp = line;
	int ramIndex = ramaddress - (ramaddress % blockSize);

	for (int i = 0; i < blockSize; i++)
	{
		updateDataRam(
			(ramIndex + (i)),
			readDataCache(
				binaryToInteger(line.index),
				blockNumber,
				i));
	}
	chacheAllocation[binaryToInteger(line.index)].set[blockNumber].v = 1;
	chacheAllocation[binaryToInteger(line.index)].set[blockNumber].dirty = 0;
	strcpy(
		chacheAllocation[binaryToInteger(line.index)].set[blockNumber].tag,
		line.tag);
	return;
}

/*
 * This tries to find an available cache block to which we can write.
 */
int blockToWrite(CacheSet *cacheset, int setNumber, cacheline line)
{
	int blockNo = findVacantBlock(cacheset, setNumber);

	/*
	 * If we did not find a vacant block, then get the Least Recently Used
	 * block.
	 */
	if (blockNo == -1)
		blockNo = findMinimumLruBlock(cacheset, setNumber);

	/*
	 * Return the block we found (available or LRU).
	 */
	return(blockNo);
}

/*
 * This is the beast that does all the work.
 */
void memoryOperations(int ramSize, char *get_argvs[])
{
	int read_write;
	int address;
	int current_offset = 0;
	unsigned int data;

	/*
	 * Allocate the memory associated with the RAM and cache(s).
	 */
	ramMemmoryAllocation();
	cacheMemmoryAllocation();

	/*
	 * Let's have a condition handler for the big loop.
	 */
	try
	{

		/*
		 * Loop while we are not at the end-of-file.
		 */
		while (!feof(stdin))
		{
			char thirty2Bit[33];	// 32-bits plus the null terminator.

			/*
			 * Read in the command (read/write).
			 */
			std::cin >> std::dec >> read_write;

			/*
			 * If we got an EOF, then retest the while loop, which should get
			 * us out of here.
			 */
			if (feof(stdin))
				continue;

			/*
			 * Read in the address.
			 */
			std::cin >> std::hex >> address;

			/*
			 * Get the 32-bit address (value).
			 */
			getBinary(address, thirty2Bit);

			/*
			 * Get a temporary cache line.
			 */
			cacheline temp(blockSize, tagbits_tot, indexbits_tot, offsetbits_tot);

			/*
			 * Parse out the memory address into, tag, index, and offset.
			 */
			parseMemoryAddress(thirty2Bit, temp.tag, temp.index, temp.offset);
			current_offset = binaryToInteger(temp.offset) / 4;		// We are addressable by int's.

			/*
			 * If the comment was Write.
			 */
			if (read_write == MEM_WRITE)
			{
				int blockNo;

				total_Write++;							// add this to the number of writes performed.
				std::cin >> std::hex >> data;			// read in the data to be written

				/*
				 * Let's see if the block in questions is already in the cache.
				 */
				blockNo = findBlockNumber(
							chacheAllocation,
							binaryToInteger(temp.index),
							temp.tag);

				/*
				 * If it is not, then we need to allocate a cache entry for this write operation.
				 */
				if (blockNo == -1)
				{
					miss++;
					missWrite++;

					/*
					 * Get an available block, or the least recently used block.
					 */
					blockNo = blockToWrite(
									chacheAllocation,
									binaryToInteger(temp.index),
									temp);

					/*
					 * If the block we are going to write into is indicated as dirty, then we need
					 * to right its contents out and evict the block for the new data.
					 */
					if (chacheAllocation[binaryToInteger(temp.index)].set[blockNo].dirty
							== 1)
					{

						/*
						 * Write the current contents out to memory.
						 */
						writeBlock(
							chacheAllocation,
							binaryToInteger(temp.index),
							blockNo,
							temp,
							address);
						dirtyBlockEvictionCounter++;	// Count the evicted cache entry

						/*
						 * If we are doing a Write/Allocate, then load the block, update the
						 * cache, and march the cache entry as dirty (not yet written to memory).
						 */
						if (writeAllocate)
						{
							loadBlock(
								chacheAllocation,
								binaryToInteger(temp.index),
								blockNo,
								temp,
								address);
							updateDataCache(
								chacheAllocation,
								binaryToInteger(temp.index),
								blockNo,
								current_offset,
								data,
								temp,
								address);
							chacheAllocation[binaryToInteger(temp.index)].set[blockNo].dirty = 1;

							/*
							 * This block is not the Least Recently Used block, any more.
							 */
							chacheAllocation[binaryToInteger(temp.index)].updateLRU(blockNo);
						}

						/*
						 * If we are not allocating cache entries on writes, then
						 * move the data into memory, directly.
						 */
						if (writeNoAllocate)
							updateDataRam(address, data);
					}

					/*
					 * An available cache block was located.  No need to evict, just write.
					 */
					else
					{

						/*
						 * If we are doing a Write/Allocate, then load the block, update the
						 * cache, and march the cache entry as dirty (not yet written to memory).
						 */
						if (writeAllocate)
						{
							loadBlock(
								chacheAllocation,
								binaryToInteger(temp.index),
								blockNo,
								temp,
								address);
							updateDataCache(
								chacheAllocation,
								binaryToInteger(temp.index),
								blockNo,
								current_offset,
								data,
								temp,
								address);
							chacheAllocation[binaryToInteger(temp.index)].set[blockNo].dirty = 1;

						/*
						 * This block is not the Least Recently Used block, any more.
						 */
							chacheAllocation[binaryToInteger(temp.index)].updateLRU(blockNo);
						}

						/*
						 * If we are not allocating cache entries on writes, then
						 * move the data into memory, directly.
						 */
						if (writeNoAllocate)
							updateDataRam(address, data);
					}
				}

				/*
				 * We found an entry already in the cache.
				 */
				else
				{
					hit++;					// count the hit

					/*
					 * If this is a write-back cache, then write contents of the
					 * current cache block out to memory and then update the block
					 * with the new data.
					 */
					if (writeBack)
					{
						loadBlock(
							chacheAllocation,
							binaryToInteger(temp.index),
							blockNo,
							temp,
							address);
						updateDataCache(
							chacheAllocation,
							binaryToInteger(temp.index),
							blockNo,
							current_offset,
							data,
							temp,
							address);
						chacheAllocation[binaryToInteger(temp.index)].set[blockNo].dirty = 1;
					}

					/*
					 * If this is a write-through cache, then write the contents of
					 * the current block out to memory, update the block within the cache,
					 * and update the memory.
					 */
					if (writeThrough)
					{
						loadBlock(
							chacheAllocation,
							binaryToInteger(temp.index),
							blockNo,
							temp,
							address);
						updateDataCache(
							chacheAllocation,
							binaryToInteger(temp.index),
							blockNo,
							current_offset,
							data,
							temp,
							address);

						/*
						 * Once written, the block is not dirty.
						 */
						chacheAllocation[binaryToInteger(temp.index)].set[blockNo].dirty = 0;
						updateDataRam(address, data);
					}
				}
			}

			/*
			 * Else, if the comment was Read.
			 */
			else if (read_write == MEM_READ)
			{

				/*
				 * Find the block number fot the cache entry.
				 */
				int blockNo = findBlockNumber(
									chacheAllocation,
									binaryToInteger(temp.index),
									temp.tag);

				/*
				 * If we did not find it, then we need to go get one, first
				 * by writing the contents out to memory.
				 */
				if (blockNo == -1)
				{

					/*
					 * Write the block out to memory.
					 */
					blockNo = blockToWrite(
									chacheAllocation,
									binaryToInteger(temp.index),
									temp);

					/*
					 * Load the block for reading.
					 */
					loadBlock(
						chacheAllocation,
						binaryToInteger(temp.index),
						blockNo,
						temp,
						address);

					/*
					 * This block is not the Least Recently Used block, any more.
					 */
					chacheAllocation[binaryToInteger(temp.index)].updateLRU(blockNo);
					miss++;
					missRead++;
				}

				/*
				 * We found the cache block to read from.
				 */
				else
				{
					hit++;
					loadBlock(
						chacheAllocation,
						binaryToInteger(temp.index),
						blockNo,
						temp,
						address);
				}
				total_Read++;
				//std::cout << std::hex << "memory[" << address << "] = " << readDataCache(binaryToInteger(temp.index),binaryToInteger(temp.current_offset))
				//	<< std::endl;
			}

			/*
			 * Else, if the comment was Instruction Read.
			 */
			else if (read_write == INS_READ)
			{

				/*
				 * Find the block number fot the cache entry.
				 */
				int blockNo = findBlockNumber(
									chacheAllocationIns,
									binaryToInteger(temp.index),
									temp.tag);

				/*
				 * If we did not find it, then we need to go get one, first
				 * by writing the contents out to memory.
				 */
				if (blockNo == -1)
				{
					blockNo = blockToWrite(
									chacheAllocationIns,
									binaryToInteger(temp.index),
									temp);
					loadBlock(
						chacheAllocationIns,
						binaryToInteger(temp.index),
						blockNo,
						temp,
						address);

					/*
					 * This block is not the Least Recently Used block, any more.
					 */
					chacheAllocationIns[binaryToInteger(temp.index)].updateLRU(blockNo);
					miss++;
					if (split)
						missReadIC++;
					else
						missRead++;
				}

				/*
				 * We found the cache block to read from.
				 */
				else
				{
					hit++;
					loadBlock(
						chacheAllocationIns,
						binaryToInteger(temp.index),
						blockNo,
						temp,
						address);
					//chacheAllocationIns[binaryToInteger(temp.index)].updateLRU((blockNo));
				}
				if (split)
					totalICReads++;
				else
					total_Read++;
			}
		}
	}

	/*
	 * If a condition occurs, then display the message.
	 */
	catch (char* msg)
	{
		std::cout << msg;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * Main entry point.  Parse the parameters, set up the variables, allocate some
 * memory, go ahead and process the input file, and finally, display some
 * statictics.
 */
int main(int arg_count, char *arg_values[])
{

	/*
	 * Get the parsed command-line arguments.
	 */
	if (!parseParams(
			arg_count,
			arg_values,
			chacheCapacity,
			blockSize,
			associativity,
			traceFile,
			split))
	{
		exit(2);
	}

	/*
	 * Intialize the various "global" variables.  This is for either both the Icache and Dcache, or
	 * just for the Dcache.  The "split" variable determines which.
	 */
	chacheSize = chacheCapacity * 1024;					// 2, 4, 8, 16, 32, or 64 KB
	totalChacheBlocks = chacheSize / blockSize;			// Total Cache Blocks (cache size/block size)
	totalCacheSets = totalChacheBlocks / associativity;
	indexbits_tot = log2(totalCacheSets);				// Number of bits for Cache Sets
	offsetbits_tot = log2(blockSize);					// Number of bits within block
	blockSize = blockSize / SIZEOFINT;					// Number of ints in a block
	tagbits_tot = 32 - (indexbits_tot + offsetbits_tot);// tag bits = 32 minus other bits (32-bit address)
	ramSize = ((mem_capacity * 1024 * 1024));			// Total zize of memory (in MB)
	totalWords = ramSize / blockSize;					// Total blocks (wors) in memory

	/*
	 * If Icache and Dcache are separate, allocate the Icache as well.
	 */
	if (split)
	{
		chacheSize = chacheCapacity * 1024;
		totalChacheBlocks = chacheSize / (blockSize * 2);
		totalCacheSets = totalChacheBlocks / associativity;
		indexbits_tot = log2(totalCacheSets);
		offsetbits_tot = log2(blockSize);
		tagbits_tot = 32 - (indexbits_tot + offsetbits_tot);
		ramSize = ((mem_capacity * 1024 * 1024));
		totalWords = ramSize / blockSize;

	}

//	printf("Let's check the variables first\n");
//	printf("Chache Capacity : %d KB\n", chacheCapacity);
//	printf("RAm Capacity : %d B\n", ramSize);
//	printf("Block Size (optional): %d bytes\n", blockSize);
//	printf("Associativity Size (optional): %d bytes\n", associativity);
//	printf("Split :  %i : ", split);

	/*
	 * Here's were everything begins.  The argument values are passed onto this function call.
	 * It contains the name of the file to be processed.
	 */
	memoryOperations(ramSize, arg_values);

	/*
	 * Returning from the above call, means that the simulation is over.  Display
	 * some statistics and other pertinent information.
	 */
	displayStatistics();
	displayCache(chacheAllocation, 0);
	if (split)
		displayCache(chacheAllocationIns, 1);
	displayMainMemory();

	/*
	 * OK, we are done here.  Free up the allocated memory and get out.
	 */
	free(ramAllocation);
	free(chacheAllocation);
	return(0);
}
