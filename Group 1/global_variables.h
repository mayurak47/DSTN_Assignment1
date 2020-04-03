#include "l1_struct.h"
#include "l2_structures.h"
#include "main_memory_structures.h"
#include "tlb_structures.h"
#include "config.h"

#define DATA_REQUEST 0
#define INSTRUCTION_REQUEST 1   

char bus16B[L1_BLOCK]; //Bus between L1 and L2
char bus32B[L2_BLOCK]; //Bus between L2 and main memory

tlb_buffer *tlb;

L1_struct *l1;

l2_cache *l2;
counter_lru l2_lru_counter;

main_memory_struct *main_memory;

kernel_struct *kernel;

int total_access_time;
int effective_mm_access_time;

int instruction_count[NUM_PROCESSES];

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
