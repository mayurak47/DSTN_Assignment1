#include "functions.h"

int main(int argc, char* argv[]){

    //Initialize main memory;
    mm_initialize_mm();

    //Initialize kernel structure
    kernel_struct *kernel = mm_initialize_kernel(&main_memory);

    //Initialize TLB
    tlb_create_tlb();

    //Initialize l2 cache
    l2_initialize();

    //Initialize L1 cache
    //L1 cache is split into L1_instruction and L1_data
    L1_struct* l1_instruction = l1_instruction_initialize();
    L1_struct* l1_data = l1_data_initialize();

    FILE* input_file;
    input_file = fopen(argv[1], "r");

    output_file = fopen("output.txt", "w");
    for(int i=0; i<3500; i++){
        fprintf(output_file, " ");
    }

    int MAX_NUM_PROCESSES;
    fscanf(input_file, "%d", &MAX_NUM_PROCESSES);
    char process_names[MAX_NUM_PROCESSES][20];
    char c;
    for(int i=0; i<MAX_NUM_PROCESSES; i++){
        fscanf(input_file, "%c", &c);
        fscanf(input_file, "%[^\n]s", process_names[i]);
        printf("%s\n", process_names[i]);
    }

    FILE *process[NUM_PROCESSES];
    int pid[NUM_PROCESSES];
    for(int i=0; i<NUM_PROCESSES; i++){
        process[i] = fopen(process_names[i], "r");
        pid[i] = (i+1)*100;
        if(process[i] == NULL){
            printf("Error: Opening file %d\n", i);
            return 0;
        }
    }

    for(int i=0; i<NUM_PROCESSES; i++){
        page_hit[i] = 0;
        page_fault[i] = 0;
        l1_hit[i] = 0;
        l1_miss[i] = 0;
        l2_hit[i] = 0;
        l2_miss[i] = 0;
        tlb_hit[i] = 0;
        tlb_miss[i] = 0;
    }
    int total_tlb_hit = 0;
    int total_tlb_miss = 0;
    int total_l1_hit = 0;
    int total_l1_miss = 0;
    int total_l2_hit = 0;
    int total_l2_miss = 0;
    int total_page_fault = 0;
    int total_page_hit = 0;

    //Initialize the outer page table for each process.
    for(int i=0; i<NUM_PROCESSES; i++){
        kernel_load_new_process(pid[i], process[i], main_memory, kernel);
    }

    //Initially, there is no process executing
    kernel->currently_executing_process = -1;

    //We will be finding the total time taken to execute all process
    total_access_time = 0;

    int total_instructions_executed = 0;

    int i=0;
    int count = 1;
    while(1){
        //If the process is the first process to execute, context switch to this process.
        if(kernel->currently_executing_process = -1){
            kernel->currently_executing_process = pid[i];
            context_switch(kernel, pid[i]);
        }

        //If the process has executed its fixed number of instructions, then context switch to another process.
        if(total_instructions_executed%NUMBER_OF_INSTRUCTIONS_FOR_CONTEXT_SWITCH == 0){
            i = (i+1)%NUM_PROCESSES;
            context_switch(kernel, pid[i]);
        }

        //If all processes have reached their end of file, then break out of the loop
        if(check_eof(process))
            break;

        //If process is already terminated, continue
        if(terminate_process[i] == 1){
            i = (i+1)%NUM_PROCESSES;
            context_switch(kernel, pid[i]);
            continue;
        }

        //If the process has reached end of file, then terminate the process.
        if(feof(process[i])){
            terminate_process[i] = 1;

            //Terminate the process in main memory.
            mm_terminate_process(pid[i], &main_memory, kernel); 

            //Invalidate the entries corresponding to pid in the tlb
            tlb_invalidate(tlb, pid[i]);

            fprintf(output_file, "\n\nProcess %d terminated\n\n", pid[i]);
            
            i = (i+1)%NUM_PROCESSES;

            //Context switch to another process
            context_switch(kernel, pid[i]);
            total_access_time += context_switch_time;
            continue;
        }

        unsigned int address;
        fscanf(process[i], "%x", &address);

        fprintf(output_file, "%d\t| ", instruction_count[i]);

        //Getting the request type for the current instruction
        int request_type = get_request_type(address);   
        if(request_type == DATA_REQUEST){
            l1 = l1_data;
            fprintf(output_file, "Data Request        | ");
        }
        else
        {
            l1 = l1_instruction;
            fprintf(output_file, "Instruction Request | ");
        }
        fprintf(output_file, "Process = %d | Address = %x |" , pid[i], address);
        

        int success = execute_process_request(address, i, pid[i], process[i], request_type, kernel);

        if(success == 0){
            i = (i+1)%NUM_PROCESSES;
            context_switch(kernel, pid[i]);
            total_access_time += context_switch_time;
            continue;
        }
        total_instructions_executed ++;

    }

    fseek(output_file, 0, SEEK_SET);

    //Calculating tlb miss rate for each process and the overall tlb miss and hit counts
    double tlb_miss_rate[NUM_PROCESSES];
    fprintf(output_file, "\nTLB miss for each process\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        total_tlb_miss += tlb_miss[i];
        total_tlb_hit += tlb_hit[i];
        tlb_miss_rate[i] = ((double)tlb_miss[i]/(double)(tlb_hit[i]))*100;
        fprintf(output_file, "Process %d : tlb miss rate = %f\n", i+1, tlb_miss_rate[i]);
    }

    //Calculating l1 miss rate for each process and the overall l1 miss and hit counts
    double l1_miss_rate[NUM_PROCESSES];
    fprintf(output_file, "\nL1 miss for each process\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        total_l1_hit += l1_hit[i];
        total_l1_miss += l1_miss[i];
        l1_miss_rate[i] = ((double)l1_miss[i]/(double)(l1_miss[i] + l1_hit[i]))*100;
        fprintf(output_file, "Process %d : l1 miss rate = %f\n", i+1, l1_miss_rate[i]);
    }

    //Calculating l2 miss rate for each process and the overall l2 miss and hit counts
    double l2_miss_rate[NUM_PROCESSES];
    fprintf(output_file, "\nL2 miss for each process\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        total_l2_hit += l2_hit[i];
        total_l2_miss += l2_miss[i];
        l2_miss_rate[i] = ((double)l2_miss[i]/(double)(l2_miss[i] + l2_hit[i]))*100;
        fprintf(output_file, "Process %d : l2 miss rate = %f\n", i+1, l2_miss_rate[i]);
    }

    //Calculating page fault rate for each process and the overall page fault and page hit counts.
    double page_fault_rate[NUM_PROCESSES];
    fprintf(output_file, "\nPage faults rate of each process:\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        total_page_fault += page_fault[i];
        total_page_hit += page_hit[i];
        page_fault_rate[i] = ((double)page_fault[i]/(double)(page_fault[i] + page_hit[i]))*100;
        fprintf(output_file, "Process %d : page fault = %d, page hit = %d, page fault rate = %f\n", i+1, page_fault[i], page_hit[i], page_fault_rate[i]);
    }
    fprintf(output_file, "\n");

    fprintf(output_file, "Total TLB miss rate = %f\n", ((double)total_tlb_miss / (double)(total_tlb_hit))*100);
    fprintf(output_file, "Total L1 miss rate = %f\n", ((double)total_l1_miss/(double)(total_l1_hit + total_l1_miss))*100);
    fprintf(output_file, "Total L2 miss rate = %f\n", ((double)total_l2_miss/(double)(total_l2_hit + total_l2_miss))*100);
    fprintf(output_file, "Total page fault rate = %f\n", ((double)total_page_fault/(double)(total_page_fault + total_page_hit))*100);

    fprintf(output_file, "\nTotal time taken for all processes = %d\n", total_access_time);

    fclose(output_file);

}

/*
    Executable file : ./test input.txt
*/