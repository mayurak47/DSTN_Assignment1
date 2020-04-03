#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define NO_OF_L1_ENTRIES 64 
#define NO_OF_L1_WAYS 4
#define L1_OFFSET 4
#define L1_INDEX 6
#define L1_TAG 15

typedef struct l1_way_struct{
    int valid:1;
    int tag:15;
    char* data;
}l1_way_struct;

typedef struct lru_square_matrix_struct{
    int lru_bits[NO_OF_L1_WAYS][NO_OF_L1_WAYS];
}lru_square_matrix_struct;

typedef struct l1_entry{
    l1_way_struct way[NO_OF_L1_WAYS];
    lru_square_matrix_struct lru_matrix;
    int way_prediction:2;
}l1_entry;

typedef struct l1_cache_struct{
    l1_entry entry[NO_OF_L1_ENTRIES];
}l1_cache_struct;