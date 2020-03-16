#include "main_memory_structures.h"

extern void mm_initialize_free_frames(free_frames_struct**);
extern void mm_initialize_frame_table(frame_table_struct*);
extern page_table_struct* mm_new_page_table(int, main_memory_struct**);
extern int mm_get_free_frame(free_frames_struct**);
extern page_table_struct* mm_initialize_page_table(int, main_memory_struct**);
extern main_memory_struct* mm_initialize_mm(void);
extern void mm_load_page(int, logical_address, page_table_struct**, main_memory_struct**);
extern logical_address mm_convert(int);
extern int mm_search_page_table(logical_address, int, page_table_struct**, main_memory_struct**);