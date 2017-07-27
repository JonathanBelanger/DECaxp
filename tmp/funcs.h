#ifndef _FUNCS_H_
#define _FUNCS_H_

#include <strings.h>

/*
 * Function Prototypes for the funcs.cpp module.
 */
bool parseParams(int, char *[], int&, int&, int &, std::string &, bool &);
std::string string_to_hex(const  std::string&);
std::string hex_to_string(const  std::string&);
void getBinary(unsigned int, char *);
int binaryToInteger(char *);
void parseMemoryAddress(char *, char*, char*, char*);
void displayMainMemory(void);
void displayStatistics(void);
unsigned int readDataCache(int, int, int);
void ramMemmoryAllocation(void);
void cacheMemmoryAllocation(void);
void updateDataRam(int, unsigned int);
char* emptyoffset(void);
unsigned int readDataRamInt(int);
void displayCache(CacheSet *, int);
std::string execq(const char*);

#endif	/* _FUNCS_H_ */
