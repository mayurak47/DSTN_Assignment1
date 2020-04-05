#ifndef _GLOBAL_VARIABLES_H_
#define _GLOBAL_VARIABLES_H_

#include "l1_structures.h"
#include "l2_structures.h"
#include "main_memory_structures.h"
#include "tlb_structures.h"
#include "config.h"

// #define NUM_PROCESSES 5
#define DATA_REQUEST 0
#define INSTRUCTION_REQUEST 1   
#define NUMBER_OF_INSTRUCTIONS_FOR_CONTEXT_SWITCH 500
#define REQUIRED_PREFETCHED_PAGES 2

int NUM_PROCESSES;

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

int *instruction_count;

//Various hit count and miss count for all processes.
int *page_hit;
int *page_fault;
int *tlb_hit;
int *tlb_miss;
int *l1_hit;
int *l1_miss;
int *l2_hit;
int *l2_miss;

//Indicates whether the process has terminated.
int *terminate_process;

FILE* output_times_file;

#endif