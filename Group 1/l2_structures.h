#ifndef _L2_STRUCTURES_H_
#define _L2_STRUCTURES_H_

#include <stdio.h>	
#include <stdlib.h>
#include <stdbool.h>
#define SETS 128
#define L2_BLOCK 32
#define NO_OF_L2_WAYS 8
#define READ 0
#define WRITE 1

typedef struct {
	int tag;
	int index;
	int offset;
} l2_address;

typedef struct {
	bool valid;
	bool dirty;
	int tag;
	char data[L2_BLOCK];
} l2_entry;

typedef struct {
	l2_entry l2_entries[NO_OF_L2_WAYS];
} l2_set;

typedef struct {
	l2_set l2_sets[SETS];
} l2_cache;

typedef struct {
	int lru_bits[NO_OF_L2_WAYS];
} lru_set;

typedef struct {
	lru_set lru_sets[SETS];
} counter_lru;

#endif