#include "functions.h"

int main(int argc, char* argv[]){

    //The input file contains the number of processes and the names of the processes.
    FILE* input_file;
    input_file = fopen(argv[1], "r");

    //Output files. output_rates.txt contains the various hit and miss rates of the processes. output_times.txt contains the detailed information about each instruction of each process executed.
    FILE* output_rates_file;
    output_rates_file = fopen("output_rates.txt", "w");
    output_times_file = fopen("output_times.txt", "w");

    //NUM_PROCESSES is declared global.
    fscanf(input_file, "%d", &NUM_PROCESSES);
    char process_names[NUM_PROCESSES][20];
    char c;
    for(int i=0; i<NUM_PROCESSES; i++){
        fscanf(input_file, "%c", &c);
        fscanf(input_file, "%[^\n]s", process_names[i]);
        fprintf(output_rates_file, "Process %d : %s\n", i+1, process_names[i]);
    }


    //Initialize main memory;
    mm_initialize_mm();

    //Initialize kernel structure
    kernel_struct *kernel = kernel_initialize_kernel(&main_memory);

    //Initialize TLB
    tlb_create_tlb();

    //Initialize l2 cache
    l2_initialize();

    //Initialize L1 cache
    //L1 cache is split into L1_instruction and L1_data
    L1_struct* l1_instruction = l1_instruction_initialize();
    L1_struct* l1_data = l1_data_initialize();

    //Opening the individual process's files.
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

    page_hit = malloc(sizeof(int)*NUM_PROCESSES);
    page_fault = malloc(sizeof(int)*NUM_PROCESSES);
    tlb_hit = malloc(sizeof(int)*NUM_PROCESSES);
    tlb_miss = malloc(sizeof(int)*NUM_PROCESSES);
    l1_hit = malloc(sizeof(int)*NUM_PROCESSES);
    l1_miss = malloc(sizeof(int)*NUM_PROCESSES);   
    l2_hit = malloc(sizeof(int)*NUM_PROCESSES);
    l2_miss = malloc(sizeof(int)*NUM_PROCESSES);

    terminate_process = malloc(sizeof(int)*NUM_PROCESSES);
    instruction_count = malloc(sizeof(int)*NUM_PROCESSES);

    for(int i=0; i<NUM_PROCESSES; i++){
        page_hit[i] = 0;
        page_fault[i] = 0;
        l1_hit[i] = 0;
        l1_miss[i] = 0;
        l2_hit[i] = 0;
        l2_miss[i] = 0;
        tlb_hit[i] = 0;
        tlb_miss[i] = 0;
        terminate_process[i] = 0;
        instruction_count[i] = 0;
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

    int total_instructions_executed = 1;

    int i=0;
    int count = 1;
    while(1){
        //If the process is the first process to execute, context switch to this process.
        if(kernel->currently_executing_process == -1){
            kernel->currently_executing_process = pid[i];
            context_switch(kernel, pid[i]);
        }

        //If the process has executed its fixed number of instructions, then context switch to another process.
        if(total_instructions_executed%NUMBER_OF_INSTRUCTIONS_FOR_CONTEXT_SWITCH == 0){
            fprintf(output_times_file, "\t\t\t\t\t\t\t\t\t\t\tProcess needs to be swapped out\t\t\t\t\t\t\n");
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

            fprintf(output_times_file, "\n\nProcess %d terminated\n\n", pid[i]);
            
            i = (i+1)%NUM_PROCESSES;

            //Context switch to another process
            context_switch(kernel, pid[i]);
            total_access_time += context_switch_time;
            continue;
        }
        unsigned int address;
        fscanf(process[i], "%x", &address);

        fprintf(output_times_file, "%d\t| ", instruction_count[i]);

        //Getting the request type for the current instruction
        int request_type = get_request_type(address);   
        if(request_type == DATA_REQUEST){
            l1 = l1_data;
            fprintf(output_times_file, "Data Request        | ");
        }
        else
        {
            l1 = l1_instruction;
            fprintf(output_times_file, "Instruction Request | ");
        }

        fprintf(output_times_file, "Process = %d | Address = %x |" , pid[i], address);
        
        int success = execute_process_request(address, i, pid[i], process[i], request_type, kernel);

    // printf("No segment fault\n");
        if(success == 0){
            i = (i+1)%NUM_PROCESSES;
            context_switch(kernel, pid[i]);
            total_access_time += context_switch_time;
        }

        total_instructions_executed ++;

    }


    //Calculating tlb miss rate for each process and the overall tlb miss and hit counts
    double tlb_miss_rate[NUM_PROCESSES];
    fprintf(output_rates_file, "\nTLB miss for each process\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        total_tlb_miss += tlb_miss[i];
        total_tlb_hit += tlb_hit[i];
        tlb_miss_rate[i] = ((double)tlb_miss[i]/(double)(tlb_hit[i]))*100;
        fprintf(output_rates_file, "Process %d : tlb miss rate = %f\n", i+1, tlb_miss_rate[i]);
    }

    //Calculating l1 miss rate for each process and the overall l1 miss and hit counts
    double l1_miss_rate[NUM_PROCESSES];
    fprintf(output_rates_file, "\nL1 miss for each process\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        total_l1_hit += l1_hit[i];
        total_l1_miss += l1_miss[i];
        l1_miss_rate[i] = ((double)l1_miss[i]/(double)(l1_miss[i] + l1_hit[i]))*100;
        fprintf(output_rates_file, "Process %d : l1 miss rate = %f\n", i+1, l1_miss_rate[i]);
    }

    //Calculating l2 miss rate for each process and the overall l2 miss and hit counts
    double l2_miss_rate[NUM_PROCESSES];
    fprintf(output_rates_file, "\nL2 miss for each process\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        total_l2_hit += l2_hit[i];
        total_l2_miss += l2_miss[i];
        l2_miss_rate[i] = ((double)l2_miss[i]/(double)(l2_miss[i] + l2_hit[i]))*100;
        fprintf(output_rates_file, "Process %d : l2 miss rate = %f\n", i+1, l2_miss_rate[i]);
    }

    //Calculating page fault rate for each process and the overall page fault and page hit counts.
    double page_fault_rate[NUM_PROCESSES];
    fprintf(output_rates_file, "\nPage faults rate of each process:\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        total_page_fault += page_fault[i];
        total_page_hit += page_hit[i];
        page_fault_rate[i] = ((double)page_fault[i]/(double)(page_fault[i] + page_hit[i]))*100;
        fprintf(output_rates_file, "Process %d : page fault = %d, page hit = %d, page fault rate = %f\n", i+1, page_fault[i], page_hit[i], page_fault_rate[i]);
    }
    fprintf(output_rates_file, "\n");

    fprintf(output_rates_file, "Total TLB miss rate = %f\n", ((double)total_tlb_miss / (double)(total_tlb_hit))*100);
    fprintf(output_rates_file, "Total L1 miss rate = %f\n", ((double)total_l1_miss/(double)(total_l1_hit + total_l1_miss))*100);
    fprintf(output_rates_file, "Total L2 miss rate = %f\n", ((double)total_l2_miss/(double)(total_l2_hit + total_l2_miss))*100);
    fprintf(output_rates_file, "Total page fault rate = %f\n", ((double)total_page_fault/(double)(total_page_fault + total_page_hit))*100);

    fprintf(output_rates_file, "\nTotal time taken for all processes = %d\n\n", total_access_time);

    for(int i=0; i<NUM_PROCESSES; i++){
        fclose(process[i]);
    }
    fclose(input_file);

    fclose(output_rates_file);
    fclose(output_times_file);

    for(int i=0; i<NUM_PROCESSES; i++){
        free(tlb_hit);
        free(tlb_miss);
        free(l1_hit);
        free(l1_miss);
        free(l2_hit);
        free(l2_miss);
        free(page_fault);
        free(page_hit);
        free(terminate_process);
        free(instruction_count);
    }

}

/*
    Executable file : ./test input.txt
*/