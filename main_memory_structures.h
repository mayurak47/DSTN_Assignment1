#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define NO_OF_FRAMES 32768
#define PAGE_SIZE 1024
#define NO_OF_ENTRIES 512
#define NUM_PROCESSES 5
#define REQUIRED_PREFETCHED_PAGES 2

//The logical address which is generated. As virtual address is 32 bits, the logical address is divided into 4 parts
//Offset, inner, middle and outer bits. Offset = 10 bits (size of 1 page = 1KB). 
//As number of entries in one page table is 256, inner bits = 8, middle bits = 8 and outer bits = 6.
typedef struct logical_address{
    int offset:10;
    int outer_pt:4;
    int middle_pt:9;
    int inner_pt:9;
}logical_address;

//The free frames list
typedef struct free_frames_struct{
    int frame_no:15;
    struct free_frames_struct *next;
}free_frames_struct;

//One entry in the frame table. Contains the valid/invalid bit, pid, LRU counter
//and a description specifying whether the frame stores a page table or a page.
typedef struct frame_table_entry{
    int valid:1;
    int modified:1;
    int pid;
    int counter:16;
    char desc;
    logical_address page_no;
    void* page_pointer;

}frame_table_entry;

//The complete frame table
typedef struct frame_table_struct{
    frame_table_entry frame_entry[NO_OF_FRAMES];
}frame_table_struct;

//One page table entry. Contains the valid/invalid bit, modified bit, referenced bit and the frame no.
//Size of one page table entry = 1 + 15 = 16 bits which is equal to 2 bytes
typedef struct page_table_entry{
    int valid:1;
    int frame_no:15;
}page_table_entry;

//The complete page table. Size of one page table entry is 3 bytes but to get a round number, we are assuming it as 4 bytes.
//Thus the number of entries in one page table = 1KB/4bbytes = 256 entries.
typedef struct page_table_struct{
    page_table_entry page_entry[NO_OF_ENTRIES];
}page_table_struct;

//The main memory structure. It contains the free frame list, the all frames array and the frame table.
typedef struct main_memory_struct{
    frame_table_struct frame_table;
    free_frames_struct *free_frames_head;
}main_memory_struct;

typedef struct pcb_struct{
    int pid;
    int outer_pt_frame_no;
}pcb_struct;

typedef struct kernel_struct{
    pcb_struct *pcb;
}kernel_struct;