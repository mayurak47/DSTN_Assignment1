#ifndef _GLOBAL_VARIABLES_H_
#define _GLOBAL_VARIABLES_H_

#include "l1_structures.h"
#include "l2_structures.h"
#include "main_memory_structures.h"
#include "tlb_structures.h"
#include "config.h"

#define DATA_REQUEST 0
#define INSTRUCTION_REQUEST 1   

char bus16B[L1_BLOCK]; //Bus between L1 and L2
char bus32B[L2_BLOCK]; //Bus between L2 and main memory

counter_lru l2_lru_counter;

//TLB pointer
tlb_buffer *tlb;

//L1 pointer
L1_struct *l1;

//L2 pointer
l2_cache *l2;

//Main memory pointer
main_memory_struct *main_memory;

//Total execution time for all the processes.
int total_access_time;

int instruction_count[NUM_PROCESSES];

//Various hit count and miss count for all processes.
int page_hit[NUM_PROCESSES];
int page_fault[NUM_PROCESSES];
int tlb_hit[NUM_PROCESSES];
int tlb_miss[NUM_PROCESSES];
int l1_hit[NUM_PROCESSES];
int l1_miss[NUM_PROCESSES];
int l2_hit[NUM_PROCESSES];
int l2_miss[NUM_PROCESSES];

//Indicates whether the process has terminated.
int terminate_process[NUM_PROCESSES];

FILE* output_file;

#endif