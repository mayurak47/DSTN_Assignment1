#include "tlb_structures.h"

tlb_buffer* create_tlb();
void init_tlb(tlb_buffer *);
int find_free_entry(tlb_buffer *);
void store_mapping(tlb_buffer *, unsigned int, unsigned int, unsigned int);
int search(tlb_buffer *, unsigned int, unsigned int);
