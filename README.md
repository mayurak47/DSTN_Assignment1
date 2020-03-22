# DSTN_Assignment1
DSTN Assignment 1 Group #1 &amp; #2

Group Members:<br>
Kevin Boban<br>
Mayur Arvind<br>
Amol Pai<br>
Ritvik Agarwal<br>

All files in the directory:<br>
1. main_memory_structures.h : Contains the data structures required for main memory. <br>
   - Data structures included are : page_table, frame_table, free_frames_list, main_memory, pcb, kernel, logical_address <br>

2. main_memory.h &amp; main_memory.c : main_memory.h contains the functions that will be needed in other .c files and main_memory.c contains the implementations of those functions along with other functions.
   - Functions included in main_memory.h are:
      - initialize_page_table
      - initialize_frame_table
      - initialize_main_memory
      - initialize_kernel
      - prefetch_pages
      - search_page_table
      - terminate_process
      - write_to_disk
      - write_to_mm
      - print_frame_table
      - print_page_table
      - convert_logical_address
