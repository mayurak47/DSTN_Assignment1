#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NO_OF_TLB_ENTRIES 32
#define PAGE_NO_BITS 22
#define FRAME_NO_BITS 15
#define OFFSET_NO_BITS 10

typedef struct tlb_entry
{
  unsigned int valid:1;
  unsigned int pid;
  unsigned int page_no:PAGE_NO_BITS;
  unsigned int frame_no:FRAME_NO_BITS;
  unsigned char counter;
} tlb_entry;

typedef struct tlb_buffer
{
  tlb_entry entries[NO_OF_TLB_ENTRIES];
} tlb_buffer;
