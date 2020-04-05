#include "functions.h"

/*
    Input of function - pid of process, file descriptor of process, main memory structure and kernel structure
    Purpose of the function - Load a new process into main memory, prefetch two pages and update the kernel to reflect that a new process is loaded.
    Output/Result of the function - This function doesnt return any value. The result is a new process is ready to execute in main memory.
*/
void kernel_load_new_process(int pid, FILE* fd, main_memory_struct* main_memory, kernel_struct *kernel){
    page_info outer_page = mm_initialize_page_table(pid, &main_memory, kernel); 
    for(unsigned int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == -1){
            kernel->pcb[i].pid = pid;
            kernel->pcb[i].outer_page = outer_page;
            kernel->pcb[i].valid = 1;
            break;
        }
    }
    kernel->CR3_reg = outer_page;

    mm_prefetch_pages(pid, fd, &main_memory, kernel);
}

/*
    Input of function - Input is the main memory.
    Purpose of the function - Initializes the kernel structure. It also updates the frame table to reflect that frame 0 is occupied by this kernel structure and it is not replaced.
    Output/Result of function - Returns the pointer to the kernel structure.
*/
kernel_struct* kernel_initialize_kernel(main_memory_struct** main_memory){
    kernel_struct* kernel = malloc(sizeof(kernel_struct));
    kernel->pcb = malloc(sizeof(pcb_struct)*NUM_PROCESSES);

    for(unsigned int i=0; i<NUM_PROCESSES; i++){
        kernel->pcb[i].pid = -1;
        kernel->pcb[i].outer_page.page_pointer = NULL;
        kernel->pcb[i].outer_page.frame_no = -1;
        kernel->pcb[i].valid = 0;
    }

    unsigned int free_frame = mm_get_free_frame(main_memory, kernel); //Frame 0 will always be given as this function is called during initialization.
    logical_address_struct invalid;
    invalid.outer_pt = invalid.middle_pt = invalid.inner_pt = -1;

    mm_update_frame_table(&((*main_memory)->frame_table), free_frame, 0, 'k', invalid);

    return kernel;
}

//Function used to check whether the outer page table of the process is valid or not.
unsigned int kernel_check_valid_bit(unsigned int pid, kernel_struct *kernel){
    for(unsigned int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == pid){
            return ((kernel->pcb[i].valid & 0x1) == 1);
        }
    }
    return 0;
}

//Function used to set the valid bit of the outer page table to 1.
void kernel_set_valid_bit(unsigned int pid, kernel_struct *kernel){
    for(unsigned int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == pid){
            kernel->pcb[i].valid = 1;
        }
    }
}

//Function to invalidate the outer page table in the pcb if the frame selected to replace is the outer page table frame.
void kernel_invalidate_outer_page_table(int pid, kernel_struct* kernel){
    for(unsigned int i=0; i<NUM_PROCESSES; i++){
            if(kernel->pcb[i].pid == pid){
                kernel->pcb[i].valid = 0;
            }
    }
}

page_info kernel_get_outer_page_table(int pid, kernel_struct* kernel){
    for(int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == pid)
            return kernel->pcb[i].outer_page;
    }

    //Ideally unreachable
    page_info invalid;
    invalid.frame_no = -1;
    invalid.page_pointer = NULL;
    return invalid;
}

int kernel_get_number_of_pages(int pid, kernel_struct* kernel){
    for(int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == pid)
            return kernel->pcb[i].number_of_pages;
    }
    return -1; //Ideally unreachable
}

//Function to terminate the process by setting the pid in the pcb of the process as -1.
void kernel_terminate_process(int pid, kernel_struct* kernel){
    for(unsigned int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == pid){
            kernel->pcb[i].pid = -1;
        }
    }

}

//Function to check whether all processes have reached end of file or not
int check_eof(FILE* process[NUM_PROCESSES]){
    int flag = 0;
    for(int i=0; i<NUM_PROCESSES; i++){
        if(terminate_process[i] == 0){
            return 0;
        }
    }


    for(int j=0; j<NUM_PROCESSES; j++){
        if(!feof(process[j]))
            flag = 1;        
    }

    return (flag == 0);
}

//Function to context switch to process with process id as pid.
void context_switch(kernel_struct* kernel, int pid){
    (*kernel).currently_executing_process = pid;
    for(int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == pid && (kernel->pcb[i].valid & 0x1) == 1){
            (*kernel).CR3_reg = kernel->pcb[i].outer_page;
            break;
        }
    }
}

int rand50() 
{ 
    // rand() function will generate odd or even 
    // number with equal probability. If rand() 
    // generates odd number, the function will 
    // return 1 else it will return 0. 
    return rand() & 1; 
} 
  
// Random Function to that returns 1 with 75% 
// probability and 0 with 25% probability using 
// Bitwise OR 
bool rand75() 
{ 
    return rand50() | rand50(); 
} 

//Returns 0 25% of the time and 1 75% of the time
int check_cpu_write(){
    return rand75();
}


//Finds the request type of the instruction. If the 1st 4 bits are 7, then it is a instruction, else it is a data.
int get_request_type(int virtual_address){
    int request = virtual_address >> 28;
    if(request == 7)
        return INSTRUCTION_REQUEST;
    else
        return DATA_REQUEST;
}

/*
    Input of function - Logical address, currently executing process, pid of that process, file descriptor of that process
                        Request type of the instruction - can be DATA_REQUEST or INSTRUCTION_REQUEST, kernel structure
    Purpose of the function - It executes the instruction requested by the process. The instruction has a logical address given by 'address'.
                              This function performs all the operations necessary for the execution of the process.
                              The flow chart of executing the instruction is as follows -
                              Search in TLB. If it is a TLB miss, search the page no in the main memory and update the tlb.
                              If it is a TLB hit, search L1 and L2 cache for the data. If both caches are misses, we search the main memory for the data.
                              The L1 cache is split into two parts - L1_data and L1_instruction. 
                              check_cpu_write function is used to check whether cpu wants to write data in l1 cache. We have used a random number generator which will generate 75%read requests and 75% write requests.
    Output/Result of the function - It returns a value 1 if the instruction was completed successfully. Else, it returns 0 indicating it was a page fault and we need to context switch to a different process.
*/
int execute_process_request(int address, int process_executing, int pid, FILE* process, int request_type, kernel_struct* kernel){
    logical_address_struct logical_address = mm_convert(address);
    unsigned int frame_no;

    int write_bit = 0;
    //function returns 75% read request and 25% write request
    if(check_cpu_write()==1)   
        write_bit = 1;

    //Indexing of cache and searching of tlb is done in parallel.
    frame_no = tlb_search(tlb, pid, address);
    int index = l1_get_index(address);
    int offset = l1_get_offset(address);

    total_access_time += max(tlb_lookup_time, l1_cache_indexing_time);
    
    //If it is a tlb miss, then we search in the main memory.
    if(frame_no == -1){
        fprintf(output_times_file, "TLB Miss | ");
        tlb_miss[process_executing]++;
        
        //We need to restart the current instruction.
        fseek(process, -8, SEEK_CUR);
        total_access_time += restart_overhead_time;

        //Search the hierarchical page tables in main memory for this logical address.
        frame_no = mm_search_page_table(logical_address, pid, &main_memory, kernel);

        //If value returned is -1, it is a page fault and we need to context switch to another process.
        if(frame_no == -1){
            fprintf(output_times_file, "Page Fault | Context Switch |\n");
            page_fault[process_executing]++;
            total_access_time += context_switch_time;
            return 0;
        }

        //Else, it is a page hit. We update the tlb with the frame number we got and restart the instruction.
        else{
            fprintf(output_times_file, "Page hit |\n");
            page_hit[process_executing]++;
            tlb_store_mapping(tlb, pid, address, frame_no);
        }

    }

    //If it is already a tlb hit, we search the L1 and L2 cache for the data.
    else{
        tlb_hit[process_executing]++;
        fprintf(output_times_file, "TLB Hit  | Frame no = %d | ", frame_no);

        //Generating the physical address from the frame number.
        int physical_address;
        physical_address = frame_no << 10 | logical_address.offset;

        char data;

        //As the cache is already indexed, we just need to check the tag bits in that particular index in the l1 cache.
        data = l1_search_cache(physical_address, index, offset);

        //If the data returned is NULL, it is a L1 cache miss. We then need to search in L2.
        if(data == '\0'){
            fprintf(output_times_file, "L1 miss | ");
            l1_miss[process_executing]++;

            //As L2 and main memory are look aside, we need to search both parallely to get the data.
            int way_no = l2_search_cache(physical_address);
            mm_get_data_from_frame(physical_address);

            if(way_no == -1){
                fprintf(output_times_file, "L2 miss | Getting data from mm\n");
                l2_miss[process_executing]++;
                total_access_time += mm_page_lookup_time;
                l2_service_cache_miss(physical_address);
            }
            else{
                //If it is a l2 hit, then data will need to be transferred to bus16B so that it can be sent to L1.
                fprintf(output_times_file, "L2 hit  |\n");
                total_access_time += l2_cache_lookup_time;
                l2_hit[process_executing]++;
            }

            //After this step, the data will be available on the bus32B. This data will be transferred to L1 cache to ensure inclusivity of L1 and L2.
            //l2_read_to_l1 function will transfer the data from bus32B to bus16B (bus between l1 and l2).
            //L1_cacheMiss will update the entry corresponding to index with the data present on bus16B.
            l2_read_to_l1(physical_address, way_no);
            l1_cacheMiss(physical_address, index);
        }

        //If it is a l1 hit, the data will not be null and will be directly sent to the cpu.
        else{
            fprintf(output_times_file, "L1 hit  | ");
            l1_hit[process_executing]++;

            char cpu_data = 'x';

            //If cpu wants to write to L1 and the instruction is of type DATA_REQUEST, i.e. L1 cache is data cache, then only allow write.
            if(write_bit == 1 && request_type == DATA_REQUEST){
                fprintf(output_times_file, "Write to L1 |");
                //Write data from cpu and L1 and update the bus between L1 and L2 (bus16B)
                l1_write_to_l1(physical_address, cpu_data);

                //As it is write through, every write to L1 will result in a block transfer from L1 to L2.
                //The data is present on bus16B.
                l2_write_from_l1_to_l2(physical_address);

                total_access_time += max(cpu_to_l1_write_time, l1_to_l2_write_time);
            }
            fprintf(output_times_file, "\n");
        }
        instruction_count[process_executing]++;
    }
    return 1;
}
