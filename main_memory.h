#include "main_memory_structures.h"

extern void mm_initialize_free_frames(free_frames_struct**);
extern void mm_initialize_frame_table(frame_table_struct*);
extern int mm_new_page_table(page_table_struct*, free_frames_struct**);
extern int mm_get_free_frame(free_frames_struct**);
extern page_table_struct* mm_initialize_page_table(int, free_frames_struct**, frame_table_struct*);
extern main_memory_struct* mm_initialize_mm(void);