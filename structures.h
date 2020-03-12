#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define FRAME_SIZE 1024
#define NO_OF_FRAMES 32768
#define PAGE_SIZE 1024

typedef struct frame{
    char bytes[1024];
}frame;

typedef struct all_frames_struct{
    frame frame;
    all_frames_struct *next;
}all_frames_struct;

typedef struct free_frames_struct{
    frame* free_frame;
}free_frames_struct;

typedef struct frame_table_entry{
    char valid;
    int pid;
    int page_no;
    short counter;
}frame_table_entry;

typedef struct frame_table_struct{
    frame_table_entry frame_entry[NO_OF_FRAMES];
}frame_table_struct;

typedef struct page_table_entry{
    char valid;
    char reference_bit;
    char modified_bit;
    int frame_no;
}page_table_entry;

typedef struct page_table_struct{
    page_table_entry page_entry[NO_OF_FRAMES];
}page_table_struct;
