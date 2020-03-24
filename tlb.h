#include "tlb_structures.h"

tlb_buffer* tlb_create_tlb();
void tlb_init_tlb(tlb_buffer *);
int tlb_find_free_entry(tlb_buffer *);
void tlb_store_mapping(tlb_buffer *, unsigned int, unsigned int, unsigned int);
int tlb_search(tlb_buffer *, unsigned int, unsigned int);
void tlb_invalidate(tlb_buffer *, unsigned int);
void tlb_right_shift_all(tlb_buffer *);
unsigned int tlb_replace(tlb_buffer *);
void tlb_print_tlb_contents(tlb_buffer *);
