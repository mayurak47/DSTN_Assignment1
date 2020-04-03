#include "global_variables.h"

#define max(x,y) x<y?x:y

//L1 functions
extern void l1_initialize();
extern L1_struct* l1_instruction_initialize();
extern L1_struct* l1_data_initialize();
extern void l1_cacheMiss(int TLBaddress, int index);
extern char l1_search_cache(int TLBaddress, int index, int offset);
extern int l1_get_index(int physical_address);
extern int l1_get_offset(int physical_address);
extern void l1_write_to_l1(int physical_address, char data);
extern void l1_print_cache(L1_struct *l1);

//L2 functions
extern void l2_read_to_l1(int physical_address, int wayNo);
extern void l2_write_from_l1_to_l2(int physical_address);
extern void l2_initialize();
extern int l2_search_cache(int physical_address);
extern int l2_service_cache_miss(int physical_address);

//Main memory functions
extern void mm_print_frame_table(frame_table_struct frame_table);
extern void mm_print_page_table(page_table_struct *page_table);
extern page_info mm_initialize_page_table(unsigned int pid, main_memory_struct** main_memory, kernel_struct *kernel);
extern void mm_initialize_mm(void);
extern kernel_struct* mm_initialize_kernel(main_memory_struct**);
extern void mm_prefetch_pages(unsigned int pid, FILE *process, main_memory_struct **main_memory, kernel_struct *kernel);
extern unsigned int mm_replace_page(main_memory_struct** main_memory, kernel_struct *kernel);
extern unsigned int mm_search_page_table(logical_address_struct, unsigned int pid, main_memory_struct** main_memory, kernel_struct *kernel);
extern void mm_terminate_process(unsigned int pid, main_memory_struct** main_memory, kernel_struct* kernel);
extern void mm_write_to_disk(unsigned int frame_no, main_memory_struct **main_memory);
extern void mm_write_to_mm(unsigned int physical_address, main_memory_struct **main_memory);
extern void mm_get_data_from_frame(int physical_address);
extern void mm_clear_mm(main_memory_struct* main_memory);   
extern logical_address_struct mm_convert(unsigned int address);

//TLB functions
extern void tlb_create_tlb();
extern void tlb_store_mapping(tlb_buffer *, unsigned int, unsigned int, unsigned int);
extern int tlb_search(tlb_buffer *, unsigned int, unsigned int);
extern void tlb_invalidate(tlb_buffer *, unsigned int);

//Kernel functions
extern int execute_process_request(int address, int process_executing, int pid, FILE* process, int request_type, kernel_struct* kernel);
extern int check_cpu_write();
extern void context_switch(kernel_struct* kernel, int pid);
extern void kernel_load_new_process(int pid, FILE* fd, main_memory_struct* main_memory, kernel_struct *kernel);
extern int check_eof(FILE* process[NUM_PROCESSES]);
extern int get_request_type(int virtual_address);