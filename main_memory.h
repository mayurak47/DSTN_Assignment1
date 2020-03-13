#include "structures.h"

extern void mm_initialize_all_frames(all_frames_struct*, free_frames_struct**);
extern void mm_initialize_frame_table(frame_table_struct*);
extern void mm_initialize_page_table(page_table_struct*, free_frames_struct*);
extern int mm_get_free_frame(free_frames_struct**);