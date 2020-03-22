#include "main_memory_structures.h"

extern void mm_print_frame_table(frame_table_struct frame_table);
extern void mm_print_page_table(page_table_struct *page_table);
extern void mm_initialize_page_table(int pid, main_memory_struct** main_memory);
extern main_memory_struct* mm_initialize_mm(void);
extern void mm_initialize_kernel(main_memory_struct**);
extern void mm_prefetch_pages(int pid, FILE *process, main_memory_struct **main_memory);
extern int mm_replace_page(main_memory_struct** main_memory);
extern int mm_search_page_table(logical_address, int, main_memory_struct** main_memory);
extern void mm_terminate_process(int pid, main_memory_struct** main_memory);
extern void mm_write_to_disk(int frame_no, main_memory_struct **main_memory);
extern void mm_write_to_mm(int physical_address, main_memory_struct **main_memory, char *data);
extern logical_address mm_convert(int address);