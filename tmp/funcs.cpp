#ifndef __X_H_INCLUDED__
#endif
#include <stdio.h>
#include <unistd.h> //for the getopt function
#include <stdlib.h> //for atoi
#include <iostream.h>
#include <string.h>
#include <strings.h>
#include <cstring.h>
#include <fstream.h>
#include <algorithm.h>
#include <assert.h>
#include <set.h>
#include <getopt.h>
#include "common.h"
#include <cstdlib.h>
#include <stdlib.h>
#include <sstream.h>
#include <iomanip.h>
#include <stddef.h>
#include "CacheSet.h"
#define KILOBYTE	1024
#define INT_SIZE	4
#define MEM_READ 	0
#define MEM_WRITE 	1

/*
 * Store a 32-bit value into the memory.
 */
void updateDataRam(int index, unsigned int data)
{
	ramAllocation[index] = data;
}

/*
 * Read a 32-bit value out of the memory.
 */
unsigned int readDataRamInt(int index)
{
	return ramAllocation[index];
}

/*
 * Allocate the cache.
 */
void cacheMemmoryAllocation()
{
	ptrdiff_t k = 0;
	int size;

	/*
	 * Allocate the array to hold each of the cache sets.
	 */
	size = totalCacheSets * sizeof(CacheSet);
	chacheAllocation = (CacheSet *) calloc(size);

	/*
	 * Establish a condition handler in case the allocation fails.
	 */
	try
	{

		/*
		 * Allocate the individual cache sets.
		 */
		for (; k < totalCacheSets; k++)
			new (chacheAllocation + k) CacheSet(
											blockSize,
											tagbits_tot,
											indexbits_tot,
											offsetbits_tot,
											associativity,
											k);
	}

	/*
	 * If a condition was triggered, clean-up and memory just allocated and
	 * throw the condition to the next handler.
	 */
	catch (...)
	{
		for (; k > 0; k--)
			(chacheAllocation + k)->~CacheSet();
		throw;
	}

	/*
	 * If we have a separate Icache and Dcache, then allocate the memory for
	 * the instruction cache.
	 */
	if (split)
	{
		k = 0;
		chacheAllocationIns = (CacheSet*) calloc(
									((totalCacheSets) * sizeof(CacheSet)));

		/*
		 * Establish a condition handler in case the allocation fails.
		 */
		try
		{
			for (; k < totalCacheSets; k++)
				new (chacheAllocationIns + k) CacheSet(
												blockSize,
												tagbits_tot,
												indexbits_tot,
												offsetbits_tot,
												associativity,
												k);
		}

		/*
		 * If a condition was triggered, clean-up and memory just allocated and
		 * throw the condition to the next handler.
		 */
		catch (...)
		{
			for (; k > 0; k--)
				(chacheAllocationIns + k)->~CacheSet();
			throw;
		}
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * Read the data at the specific cache line.
 */
unsigned int readDataCache(
				int index,
				int blockNoInSet,
				int offset)
{
	return(chacheAllocation[index].set[blockNoInSet].data[offset]);
}

/*
 * Allocate an emptry (clear) offset.
 */
char* emptyoffset()
{
	return((char*) calloc(sizeof(char) * (offsetbits_tot + 1)));
}

/*
 * Allocate the memory that cache will be representing.
 */
void ramMemmoryAllocation()
{
	int i;

	ramAllocation = (unsigned int*) calloc(ramSize * 4);
	return;
}

/*
 * Display cache statistics.
 */
void displayStatistics()
{
	int totalAccess = (total_Read + total_Write);

	std::cout << "STATISTICS:" << std::endl;
	std::cout << "";
	std::cout << "Total Mem Acces = " << totalAccess << std::endl;

	if (!split)
	{
		printf(
			"Misses: Total %d DataReads %d DataWrites %d\n",
			(missRead+ missWrite),
			missRead, missWrite);
		printf(
			"Miss Rate: %f %f %f\n",
			((float) miss / totalAccess),
			((float) missRead / totalAccess),
			((float) missWrite / totalAccess));
		printf(
			"Number of Dirty Blocks Evicted From the Cache: %d\n",
			dirtyBlockEvictionCounter);
	}
	if (split)
	{
		printf(
			"L1I Misses: Total %d InstructionReads %d\n",
			totalICReads,
			missReadIC);
		printf("L1I Miss Rate: %f\n", missReadIC / (float) totalICReads);
		printf(
			"L1D Misses: Total %d DataReads %d DataWrites %d\n",
			(missRead+ missWrite),
			missRead,
			missWrite);
		printf(
			"L1D Miss Rate: %f %f %f\n",
			((float) miss / totalAccess),
			((float) missRead / totalAccess),
			((float) missWrite / totalAccess));
		printf("OverAll cache Misses %d\n", (missRead+ missWrite+missReadIC));
		printf(
			"Number of Dirty Blocks Evicted From L1D Cache: %d\n",
			dirtyBlockEvictionCounter);
	}
}

/*
 * This is the opposite of the getBinary function.  This converts a binary
 * string to a integer value.
 *
 * I rewrote this.  It is easier to start at the most signficate bit, convert
 * it to a 0/1 value and add to to the current value.  For subsequent
 * significant bits, shirt the current value 1 bit to the left and insert the
 * next bit into the current value.  If at any time we see something other than
 * a '0' or '1', then the current value set to 0 and we return this to the
 * caller.
 */
int binaryToInteger(char *bin)
{
	int k;
	int len, sum;

	sum = 0;
	len = strlen(bin) - 1;
	for (k = len; k >= 0; k--)
		if ((bin[k] == '0') || (bin[k] == '1')
			sum = (sum << 1) + (bin[k] - '0');
		else
		{
			sum = 0;
			break;
		}
	return(sum);
}

/*
 * Display cache information as part of the statistics summary.
 */
void displayCache(CacheSet *cacheSet, int type)
{
	if (split)
	{
		if (type == 0)
		{
			std::cout << "L1 DATA CACHE CONTENTS:\n";
			std::cout << "Set\tV\tTag\t\tDirty\t\tWords\n";
		}
		if (type == 1)
		{
			std::cout << "L1 INSTRUCTION CACHE CONTENTS:\n";
			std::cout << "Set\tV\tTag\t\tWords\n";

		}
	}
	else
	{
		std::cout << "CACHE CONTENTS:\n";
		std::cout << "Set\tV\tTag\t\tDirty\t\tWords\n";
	}
	for (int i = 0; i < (totalCacheSets); i++)
	{
		for (int j = 0; j < associativity; j++)
		{
			std::cout << 
				std::hex <<
				i <<
				"\t" <<
				cacheSet[i].set[j].v <<
				"\t" <<
				std::setfill('0') <<
				std::setw(8) <<
				std::hex <<
				(int) binaryToInteger(cacheSet[i].set[j].tag) <<
				"\t";
			if (type != 1)
			{
				std::cout << cacheSet[i].set[j].dirty << "\t" << "\t";
			}
			std::cout << " ";
			for (int k = 0; k < blockSize; k++)
			{
				std::cout <<
					std::setfill('0') <<
					std::setw(8) <<
					std::hex <<
					(int) readDataCache(i, j, k) <<
					" ";
			}
			std::cout << std::endl;
		}
	}
}

/*
 * Display memory information as part of the summary statistics.
 */
void displayMainMemory()
{
	int begin_address = strtoul("003f7f00", NULL, 16);

	std::cout <<
		std::endl <<
		"MAIN MEMORY:" <<
		std::endl <<
		"Address      Words";

	for (int i = begin_address / INT_SIZE;
		i < (begin_address + KILOBYTE) / INT_SIZE;
		i++)
	{
		if (i % 8 == 0)
			std::cout <<
				std::endl <<
				std::setfill('0') <<
				std::setw(8) <<
				std::hex <<
				i * INT_SIZE <<
				"   ";
		std::cout <<
			"  " <<
			std::setfill('0') <<
			std::setw(8) <<
			ramAllocation[i];
	}
	std::cout << std::endl;
}

/*
 * Convert a memory address into its cache component parts:
 *	- tag
 *	- index
 *	- offset
 */
void parseMemoryAddress(
		char *bformatted,
		char* tag,
		char* index,
		char* offset)
{
	int i = 0;
	int start, end;

	/*
	 * Make sure we have the places to store the information being requested.
	 */
	assert(tag != NULL);
	assert(index != NULL);
	assert(offset != NULL);

	/*
	 * Pull out the tag bits.
	 */
	start = 0;
	end = tagbits_tot;
	for (i = start; i < end; i++)
		tag[i] = bformatted[i];
	tag[tagbits_tot] = '\0';

	/*
	 * Pull out the index bits.
	 */
	start = end + 1;
	end += (indexbits_tot + 1);
	for (i = start; i < end; i++)
		index[i - tagbits_tot - 1] = bformatted[i - 1];
	index[indexbits_tot] = '\0';

	/*
	 * Pull out the offset bits.
	 */
	start = end + 1;
	end += (offsetbits_tot + 1);
	for (i = start; i < end; i++)
		offset[i - indexbits_tot - tagbits_tot - 2] = bformatted[i - 2];
	offset[offsetbits_tot] = '\0';

	// printf("Tag: %s (%i)\n", tag, binaryToInteger(tag));
	// printf("Index: %s (%i)\n", index, binaryToInteger(index));
	// printf("Offset: %s (%i)\n", offset, binaryToInteger(offset));
	return;
}

/*
 * Convert a numeric value to a binary string (32-bits, plus \0)
 *
 * The original version of this code allocated memory, which was never
 * freed.  This could be a huge memory leak.  Now the caller provides
 * the location to receive the binary string.
 */
void getBinary(unsigned int num, char *bstring)
{
	int i;

	bstring[32] = '\0';
	for (i = 0; i < 32; i++)
		bstring[32 - 1 - i] = (num & (1 << i)) ? '1' : '0';

	return;
}

/*
 * Check that the user supplied the correct block size.
 */
 bool checkMemSize(int memSize)
{
	switch (memSize)
	{
		case 4:
		case 8:
		case 16:
		case 32:
		case 64:
			return true;
			break;

		default:
			break;
	}
	return false;
}

/*
 * Check that the user supplied the correct block size.
 */
bool checkBlockSize(int blockSize)
{
	bool retVal = false;

	switch (blockSize)
	{
		case 4:
		case 8:
		case 16:
		case 32:
		case 64:
		case 128:
		case 256:
		case 512:
			retVal = true;
			break;
	
		default:
			break;
	}
	return(retVal);
}

/*
 * Parse and verify the input parameters.
 */
bool parseParams(
		int argc,
		char *argv[],
		int& mem_capacity,
		int& blockSize,
		int &assosiativity,
		std::string &filename,
		bool &split)
{
	extern char *optarg;
	extern int optopt;
	int c = 0;
	bool c_flag = false;
	bool b_flag = false;
	bool a_flag = false;
	bool t_flag = false;
	bool errflg = false;
	bool retVal = true;
	int digit_optind = 0;

	/*
	 * Loop through all the supplied parameters until they have all been
	 * processed.
	 */
	while (c != -1)
	{
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] =
		{
			{ "wbwa", 0, 0, 0 },
			{ "wtwn", 0, 0, 0 },
			{ "wbwn", 0, 0, 0 },
			{ "wtwa", 0, 0, 0 },
			{ 0, 0, 0, 0 }
		};

		/*
		 * Get the information to process the next parameter (flag and value).
		 */
		c = getopt_long(
				argc,
				argv,
				"c:a:b:s::0:1:2:3:",
				long_options,
				&option_index);

		/*
		 * Process the next command line option (parameter).
		 */
		switch (c)
		{
			case 0:
				if (strcmp(long_options[option_index].name, "wbwa") == 0)
				{
					writeBack = true;
					writeThrough = false;
					writeAllocate = true;
					writeNoAllocate = false;
				}
				if (strcmp(long_options[option_index].name, "wtwn") == 0)
				{
					writeThrough = true;
					writeBack = false;
					writeNoAllocate = true;
					writeAllocate = false;
				}
				if (strcmp(long_options[option_index].name, "wbwn") == 0)
				{
					writeThrough = false;
					writeBack = true;
					writeNoAllocate = true;
					writeAllocate = false;
				}
				if (strcmp(long_options[option_index].name, "wtwa") == 0)
				{
					writeThrough = true;
					writeBack = false;
					writeNoAllocate = false;
					writeAllocate = true;
				}
			break;

			case 's':
				split = true;
				break;

			case 't':
				filename = optarg;
				t_flag = true;
				break;

			case 'c':
				mem_capacity = atoi(optarg);
				c_flag = true;
				break;

			case 'b':
				blockSize = atoi(optarg);
				b_flag = true;
				break;

			case 'a':
				assosiativity = atoi(optarg);
				a_flag = true;
				break;

			case ':': //:  -c without operand
				fprintf(stderr, "Option -%c requires an operand\n", optopt);
				errflg++;
				break;

			case '?':
				fprintf(stderr, "Unrecognised option: -%c\n", optopt);
				errflg = true;
				break;

			case -1:
				/* We are all done parsing.  Nothing else to do here. */
				break;

			default:
				printf("?? getopt returned character code 0%o ??\n", c);
				break;
		}
	}

	/*
	 * At this point we should have process all the parameters.  If there are
	 * additional parameters, then report those to the user.
	 */
	if (optind < argc)
	{
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}

	/*
	 * Verify the value of the supplied block size.
	 */
	if (!checkBlockSize(blockSize))
	{
		std::cout
				<< "\nPossible Block capacities are 4, 8, 16, 32, 64, 128, 256, or 512.\n";
		errflg = true;
	}

	/*
	 * Verify the value of the supplied memory size.
	 */
	if (!checkMemSize(mem_capacity))
	{
		std::cout << "\nPossible Memmory capacities are 4, 8, 16, 32, or 64.";
		errflg = true;
	}

	/*
	 * If there were any errors detected, then report that to the user.
	 */
	if (errflg || !c_flag || !b_flag || !a_flag)
	{
		fprintf(stderr,
				"usage: %s -c<capacity> -b<wordsize> -a<associativity> -s<for Split> < inputTrace.trace > outputFile.txt \nWrite policies are as follows: --wbwa/--wbwn/--wtwa/--wtwn",
				argv[0]);
		retVal = false;
	}

	/*
	 * Return the results back to the caller.
	 */
	return(retVal);
}
