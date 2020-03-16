#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define NO_OF_FRAMES 32768
#define PAGE_SIZE 1024

// typedef struct frame{
//     int bytes[1024];
// }frame;

// typedef struct all_frames_struct{
//     frame frames[NO_OF_FRAMES];
// }all_frames_struct;

typedef struct free_frames_struct{
    int frame_no:15;
    struct free_frames_struct *next;
}free_frames_struct;

typedef struct frame_table_entry{
    int valid:1;
    int pid;
    short counter;
}frame_table_entry;

typedef struct frame_table_struct{
    frame_table_entry frame_entry[NO_OF_FRAMES];
}frame_table_struct;

typedef struct page_table_entry{
    int valid:1;
    int modified_bit:1;
    int referenced_bit:1;
    void* frame_no;
}page_table_entry;

typedef struct page_table_struct{
    page_table_entry page_entry[NO_OF_FRAMES];
}page_table_struct;

typedef struct main_memory_struct{
    frame_table_struct frame_table;
    free_frames_struct *free_frames_head;
}main_memory_struct;

typedef struct logical_address{
    int offset:10;
    int outer_pt:2;
    int middle_pt:10;
    int inner_pt:10;
}logical_address;