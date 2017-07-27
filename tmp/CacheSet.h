/*
 * CacheSet.h
 *
 *  Created on: Sep 26, 2015
 *      Author: quamber
 */
#ifndef _CACHESET_H_
#define _CACHESET_H_

#include "cacheline.cpp"
#include <cstddef.h>

class CacheSet
{
	public:

		/*
		 * Constructor and Destructor
		 */
		CacheSet(int blocksize,int tagbits,int indexbits,int offsetbits,int linesinSet, int indexNo);
		virtual ~CacheSet();

		/*
		 * Class functions.
		 */
		void init(int blocksize,int tagbits,int indexbits,int offsetbits,int sets);
		void updateLRU(int index);
		int getLRU(int index);
		int minimumLRUBlock();

		/*
		 * Class fields.
		 */
		cacheline *set;
		unsigned char** lru;
		int noofblocks;
		int index;
};

#endif /* _CACHESET_H_ */
