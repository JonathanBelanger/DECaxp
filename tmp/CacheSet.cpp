/*
 * CacheSet.cpp
 *
 *  Created on: Sep 26, 2015
 *      Author: quamber
 */
#include "CacheSet.h"
#include <memory.h>

/*
 * CacheSet Constructor
 */
CacheSet::CacheSet(
		int blocksize,
		int tagbits,
		int indexbits,
		int offsetbits,
		int linesinSet,
		int indexNo)
{
	noofblocks = linesinSet;
	index = indexNo;

	init(blocksize, tagbits, indexbits, offsetbits, linesinSet);

	lru = new unsigned char*[noofblocks];

	for (int i = 0; i < noofblocks; ++i)
		lru[i] = new unsigned char[noofblocks];

	for (int i = 0; i < noofblocks; ++i)
		for (int j = 0; j < noofblocks; ++j)
			lru[i][j] = 0;
	return;
}

/*
 * CacheSet initialization function.
 */
void CacheSet::init(
		int blocksize,
		int tagbits,
		int indexbits,
		int offsetbits,
		int sets)
{
	ptrdiff_t k = 0;

	set = (cacheline*) calloc((sizeof(cacheline) * sets));

	try
	{
		for (; k < sets; k++)
			new (set + k) cacheline(blocksize, tagbits, indexbits, offsetbits);
	}

	catch (...)
	{
		for (; k > 0; k--)
			(this->set + k)->~cacheline();
		throw;
	}
	return;
}

/*
 * Get the smallest LRU.
 */
int CacheSet::minimumLRUBlock()
{
	int min = 4;
	int temp;
	int i, j;

	for (i = 0; i < noofblocks; ++i)
	{
		temp = 0;
		for(j = 0; j < noofblocks; ++j)
			temp += (short)lru[i][j];
		if(temp < min)
			min = temp;
	}
	return(min);
}

/*
 * Update Least Recently Used.
 */
void CacheSet::updateLRU(int index)
{
	int i;

	for (i = 0; i < noofblocks; ++i)
	{
		lru[index][i] = 1;
		lru[i][index] = 0;
	}
	return;
}

/*
 * Get the Least Recently Used entry.
 */
int CacheSet::getLRU(int index)
{
	int temp = 0;
	int i, j;

	for (i = 0; i < noofblocks; ++i)
	{
		for(j = 0; j < noofblocks; ++j)
		{
			temp += (short) lru[index][j];
			break;
		}
	}
	return(temp);
}

/*
 * CacheSet destructor.
 */
CacheSet::~CacheSet()
{
	free(set);
	return;
}
