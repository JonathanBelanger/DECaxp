/*
 * common.h


 *
 *  Created on: Sep 9, 2015
 *      Author: quamber
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <string.h>
#include <iostream.h>
#include "CacheSet.h"

extern int offsetbits_tot;
extern int tagbits_tot;
extern int indexbits_tot;
extern int mem_capacity ;
extern int associativity;
extern int datasize;
extern int totalWords ;
extern unsigned int* ramAllocation;
extern int ramSize ;
extern int missReadIC;
extern int totalICReads;
extern CacheSet *chacheAllocation;
extern CacheSet *chacheAllocationIns;
extern int blockSize ;
extern std::string traceFile;
extern int total_Read ;
extern int total_Write ;
extern bool split;
extern int chacheCapacity,chacheSize,totalChacheBlocks;
extern double missrate_tot,missrate_read,missrate_write;
extern int dirtyBlockEvictionCounter;
extern int writepolicy;
extern int hitpolicy;
extern int hit , miss;
extern int missRead , missWrite ;
extern int totalCacheSets;
extern  bool writeBack;
extern  bool writeThrough;
extern bool writeAllocate;
extern bool writeNoAllocate;

#endif	/* _COMMON_H_ */