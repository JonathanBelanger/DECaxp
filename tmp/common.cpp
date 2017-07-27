/*
 * common.cpp
 *
 *  Created on: Sep 10, 2015
 *      Author: quamber
 */

#ifndef _COMMON_CPP_
#define _COMMON_CPP_

#include "common.h"
int offsetbits_tot=0;
int tagbits_tot=0;
int indexbits_tot=0;
int mem_capacity = 16;
int associativity =1;
int datasize=sizeof(int);
int totalWords = 0;
unsigned int* ramAllocation;
int ramSize = 0;
CacheSet *chacheAllocationIns;
CacheSet *chacheAllocation;
bool split = false;
int blockSize = 4;
std::string traceFile;
int total_Read = 0;
int total_Write = 0;
int totalCacheSets=0;
int chacheCapacity=0,chacheSize=0,totalChacheBlocks=0;
double missrate_tot=0,missrate_read=0,missrate_write=0;
int dirtyBlockEvictionCounter=0;
int writepolicy;
int hitpolicy;
int hit = 0, miss = 0;
int missRead = 0, missWrite = 0;
int missReadIC=0;
int totalICReads=0;
bool writeBack=true;
bool writeThrough=false;
bool writeAllocate=true;
bool writeNoAllocate=false;

#endif	/* _COMMON_CPP_ */