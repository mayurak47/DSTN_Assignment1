#ifndef _L1_STRUCTURES_H_
#define _L1_STRUCTURES_H_

#include <stdio.h>	
#include <stdlib.h>
#include <time.h>
#define ENTRIES 64
#define L1_BLOCK 16
#define NO_OF_L1_WAYS 4

typedef struct 
{
	short valid_Tag[NO_OF_L1_WAYS];
	char data[L1_BLOCK*NO_OF_L1_WAYS];

	short LRUBits;
	char wayPrediction;

	// char padding;
}L1_row;

typedef struct
{
	L1_row L1row[ENTRIES];
}L1_struct;

#endif
