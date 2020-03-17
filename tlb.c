#include "tlb.h"

tlb_buffer* create_tlb()
{
  tlb_buffer *tlb = malloc(sizeof(tlb_buffer));
  return tlb;
}

void init_tlb(tlb_buffer *tlb)
{
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
    tlb->entries[i].valid = 0;
}

int find_free_entry(tlb_buffer *tlb)
{
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    if(tlb->entries[i].valid == 0)
      return i;
  }
  return -1;
}

void store_mapping(tlb_buffer *tlb, unsigned int pid, unsigned int virtual_addr, unsigned int frame_no)
{
  int entry_num = find_free_entry(tlb);
  // if (entry_num == -1) entry_num = replace(tlb);
  unsigned int page_no = virtual_addr >> OFFSET_NO_BITS;

  tlb->entries[entry_num].valid = 1;
  tlb->entries[entry_num].pid = pid;
  tlb->entries[entry_num].page_no = page_no;
  tlb->entries[entry_num].frame_no = frame_no;
}

int search(tlb_buffer *tlb, unsigned int pid, unsigned int virtual_addr)
{
  unsigned int page_no = virtual_addr >> OFFSET_NO_BITS;
  for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
  {
    if(tlb->entries[i].pid == pid && tlb->entries[i].page_no == page_no)
    {
      // printf("valid = %u, pid = %u, page_no = %u, frame_no = %u\n", tlb->entries[i].valid, tlb->entries[i].pid, tlb->entries[i].page_no, tlb->entries[i].frame_no);
      return tlb->entries[i].frame_no;
    }
  }
  return -1;
}

//testing
int main()
{
  tlb_buffer *tlb = create_tlb();
  init_tlb(tlb);
  printf("%d\n", find_free_entry(tlb));
  store_mapping(tlb, 1, 4004127249, 76);
  printf("%d\n", search(tlb, 1, 4004127249));
  printf("%d\n", search(tlb, 2, 76765567));
  printf("%d\n", find_free_entry(tlb));
}
