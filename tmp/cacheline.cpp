/*
 * cache.cpp
 *
 *  Created on: Sep 10, 2015
 *      Author: Muhammad Quamber Ali
 */
#ifndef _CACHE_CPP_
#define _CACHE_CPP_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Define the cacheline class
 *
 * NOTE: There does not appear to be a class destructor.
 */
class cacheline
{
	public:

		/*
		 * Class constructor
		 */
		cacheline(int blocksize,int tagbits,int indexbits,int offsetbits)
		{
			int size;

			/*
			 * Initially a cache line is not valid, dirty, or having a physical
			 * address.
			 */
			v = 0;
			dirty = 0;
			fromAddress = 0;

			/*
			 * Allocate space for the data
			 */
			size = blocksize * sizeof(unsigned int);
			data = (unsigned int *) calloc(size);

			/*
			 * Allocate space for the binary string for the index (set).
			 */
			size = 1 + (indexbits * sizeof(char));
			index = (char *) calloc(size);

			/*
			 * Allocate space for the binary string for the tag
			 */
			size = tagbits * sizeof(char);
			tag = (char *) calloc(size);

			/*
			 * Allocate space for the binary string for the offset
			 */
			size = 1 + (offsetbits * sizeof(char));
			offset = (char *) calloc(size);
		};

		/*
		 * All the initialization is done in the constructor.
		 */
		void init(int chacheCapacity,int tagbits, int indexbits,int offsetbits){};

		int v;						// valid bit
		int dirty;					// dirty bit
		unsigned int *data;			// pointer to data
		char *index;				// pointer to index (set) binary string
		char *tag;					// pointer to tag binary string
		char *offset;				// pointer to offset binary string
		int fromAddress;			// memory address associated with this cache line
};

#endif	/* _CACHE_CPP_ */
