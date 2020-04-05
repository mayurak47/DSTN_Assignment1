#include "functions.h"

void tlb_init_tlb(tlb_buffer *);
int tlb_find_free_entry(tlb_buffer *);
void tlb_right_shift_all(tlb_buffer *);
unsigned int tlb_replace(tlb_buffer *);
void tlb_print_tlb_contents(tlb_buffer *);

// Create memory for TLB
void tlb_create_tlb()
{
  tlb = malloc(sizeof(tlb_buffer));
  tlb_init_tlb(tlb);
}

// Initialize values of TLB
void tlb_init_tlb(tlb_buffer *tlb)
{
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    tlb->entries[i].valid = 0;
    tlb->entries[i].counter = 0;
  }
}

// Find a free entry if it exists
int tlb_find_free_entry(tlb_buffer *tlb)
{
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    if(tlb->entries[i].valid == 0)
      return i;
  }
  return -1;
}

// If free entry doesn't exist, find entry with lowest counter value
unsigned int tlb_replace(tlb_buffer *tlb)
{
  unsigned char min_counter_val = 255;
  unsigned int entry_num = 0;
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    if(tlb->entries[i].counter < min_counter_val)
    {
      min_counter_val = tlb->entries[i].counter;
      entry_num = i;
    }
  }
  return entry_num;
}

// Right shift all counter values
void tlb_right_shift_all(tlb_buffer *tlb)
{
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    tlb->entries[i].counter >>= 1;
  }
}

// Try to find free entry, else replace. Store the {pg. no, frame no.} mapping, update counter values
void tlb_store_mapping(tlb_buffer *tlb, unsigned int pid, unsigned int virtual_addr, unsigned int frame_no)
{
  int entry_num = tlb_find_free_entry(tlb);
  if (entry_num == -1) entry_num = tlb_replace(tlb);
  unsigned int page_no = virtual_addr >> OFFSET_NO_BITS;

  tlb->entries[entry_num].valid = 1;
  tlb->entries[entry_num].pid = pid;
  tlb->entries[entry_num].page_no = page_no;
  tlb->entries[entry_num].frame_no = frame_no;
//   tlb_right_shift_all(tlb);
//   tlb->entries[entry_num].counter |= 0b10000000;
}

// If mapping exists, update counter values and return frame no.
int tlb_search(tlb_buffer *tlb, unsigned int pid, unsigned int virtual_addr)
{
  unsigned int page_no = virtual_addr >> OFFSET_NO_BITS;

  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    if(tlb->entries[i].valid == 1 && tlb->entries[i].pid == pid && tlb->entries[i].page_no == page_no)
    {
      // printf("valid = %u, pid = %u, page_no = %u, frame_no = %u\n", tlb->entries[i].valid, tlb->entries[i].pid, tlb->entries[i].page_no, tlb->entries[i].frame_no);
      tlb_right_shift_all(tlb);
      tlb->entries[i].counter |= 0b10000000;
      return tlb->entries[i].frame_no;
    }
  }

  total_access_time += tlb_miss_OS_overhead;

  return -1;
}

// When a process is terminated, invalidate corresponding entries
void tlb_invalidate(tlb_buffer *tlb, unsigned int pid)
{
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    if(tlb->entries[i].pid == pid)
      tlb->entries[i].valid = 0;
  }
}

// Debugging function to view contents of TLB
void tlb_print_tlb_contents(tlb_buffer *tlb)
{
  printf("Valid\tPID\tPage no.\tFrame no.\tCounter\n");
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    printf("%u\t%u\t%u\t\t%u\t\t%u\n", tlb->entries[i].valid, tlb->entries[i].pid, tlb->entries[i].page_no, tlb->entries[i].frame_no, tlb->entries[i].counter);
  }
}