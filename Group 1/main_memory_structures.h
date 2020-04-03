#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define NO_OF_FRAMES 32768
#define PAGE_SIZE 1024
#define NO_OF_PT_ENTRIES 512
#define NUM_PROCESSES 5
#define REQUIRED_PREFETCHED_PAGES 2
#define MM_REQUIRED_ACCESSES 8

//The logical address which is generated. As virtual address is 32 bits, the logical address is divided into 4 parts
    //tlb_print_tlb(tlb);
//Offset, inner, middle and outer bits. Offset = 10 bits (size of 1 page = 1KB). 
//As number of entries in one page table is 256, inner bits = 8, middle bits = 8 and outer bits = 6.
typedef struct logical_address{
    unsigned int offset:10;
    unsigned int outer_pt:4;
    unsigned int middle_pt:9;
    unsigned int inner_pt:9;
}logical_address_struct;

typedef struct frame{
    char data[PAGE_SIZE/1024];
}frame;

typedef struct page_info{
    unsigned int frame_no:15;
    void* page_pointer;
}page_info;


//The free frames list
typedef struct free_frames_struct{
    unsigned int frame_no:15;
    struct free_frames_struct *next;
}free_frames_struct;

//One entry in the frame table. Contains the valid/invalid bit, pid, LRU counter
//and a description specifying whether the frame stores a page table or a page.
typedef struct frame_table_entry{
    unsigned int valid:1;
    unsigned int modified:1;
    unsigned int pid;
    unsigned int counter:16;
    char desc;
    logical_address_struct page_no;
    // void* page_pointer;

}frame_table_entry;

//The complete frame table
typedef struct frame_table_struct{
    frame_table_entry frame_entry[NO_OF_FRAMES];
}frame_table_struct;

//One page table entry. Contains the valid/invalid bit, modified bit, referenced bit and the frame no.
//Size of one page table entry = 1 + 15 = 16 bits which is equal to 2 bytes
typedef struct page_table_entry{
    unsigned int valid:1;
    page_info page;
}page_table_entry;

//The complete page table. Size of one page table entry is 3 bytes but to get a round number, we are assuming it as 4 bytes.
//Thus the number of entries in one page table = 1KB/4bbytes = 256 entries.
typedef struct page_table_struct{
    page_table_entry page_entry[NO_OF_PT_ENTRIES];
}page_table_struct;

//The main memory structure. It contains the free frame list, the all frames array and the frame table.
typedef struct main_memory_struct{
    frame_table_struct frame_table;
    free_frames_struct *free_frames_head;
}main_memory_struct;


typedef struct pcb_struct{
    unsigned int valid:1;
    unsigned int pid;
    page_info outer_page;
}pcb_struct;

typedef struct kernel_struct{
    page_info CR3_reg;
    int number_of_processes;
    int currently_executing_process;
    unsigned int no_of_references;
    pcb_struct *pcb;
}kernel_struct;