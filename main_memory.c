#include "main_memory.h"

void mm_print_frame_table(frame_table_struct frame_table);
void mm_print_page_table(page_table_struct *page_table);
void mm_initialize_free_frames(free_frames_struct** free_frames_head);
void mm_initialize_frame_table(frame_table_struct* frame_table);
void mm_update_page_table(page_table_struct** page_table, int entry, int frame_no);
void mm_update_frame_table(frame_table_struct* frame_table, int free_frame, int pid, char desc, void* pointer, logical_address la);
void mm_update_counter(frame_table_struct *frame_table, int frame);
void mm_update_pcb(int pid, kernel_struct **kernel, int outer_frame_no);
int mm_get_outer_pt_frame_no(int pid, kernel_struct *kernel);
page_table_struct* mm_get_page_table(page_table_struct* page_table, main_memory_struct *main_memory, int entry);
int mm_get_free_frame(main_memory_struct** main_memory);
int mm_new_page_table(int pid, logical_address la, main_memory_struct** main_memory);
int mm_load_page(int pid, logical_address la, main_memory_struct** main_memory);
void mm_invalidate_page_table_entries(logical_address page_no, int outer_frame_no, main_memory_struct *main_memory);
int mm_convert_back(logical_address la);
int mm_check_dirty_bit(int frame_no, main_memory_struct *main_memory);

//Function to print frame table.
void mm_print_frame_table(frame_table_struct frame_table){
    printf(" ________________________________________________________________________________________________________________________\n");
    printf("|    FRAME NO   |     VALID\t|\tPID\t|\tP/F\t|     Counter\t|\tLogical Address\t|\tPage Pointer\t|\n");
    for(int i=0; i<NO_OF_FRAMES; i++){
        if(frame_table.frame_entry[i].valid & 0x1 == 1){
            printf("|\t%d\t|\t%d\t|\t%d\t|\t%s\t|\t%x\t|\t%x\t|\t%p\t|\n", i, frame_table.frame_entry[i].valid, frame_table.frame_entry[i].pid, frame_table.frame_entry[i].desc == 'f' ? "Frame" : (frame_table.frame_entry[i].desc == 'p' ? "Page" : "Kernel"), frame_table.frame_entry[i].counter & 0xFFFF, mm_convert_back(frame_table.frame_entry[i].page_no), frame_table.frame_entry[i].page_pointer);
        }
    }
    printf(" ------------------------------------------------------------------------------------------------------------------------\n");
}

//Function to print page table.
void mm_print_page_table(page_table_struct *page_table){
    printf("Page table\n");
    printf("_________________________________________________\n");
    printf("|    INDEX\t|\tV/I\t|    FRAME NO   |\n");
    for(int i=0; i<NO_OF_ENTRIES; i++){
        if(page_table->page_entry[i].valid & 0x1 == 1){
            printf("|\t%d\t|\t%d\t|\t%d\t|\n", i, page_table->page_entry[i].valid, page_table->page_entry[i].frame_no);
        }
    }
    printf("-------------------------------------------------\n");
}

//Initialize the free frames list. Initially, all the frames will be in the free frame list.
void mm_initialize_free_frames(free_frames_struct **free_frames_head){
    free_frames_struct *free_frames_tail = (*free_frames_head);

    for(int i=0; i<NO_OF_FRAMES; i++){
        free_frames_struct *new_free_frame = malloc(sizeof(free_frames_struct));
        new_free_frame->frame_no = i;
        new_free_frame->next = NULL;

        if((*free_frames_head) == NULL){
            (*free_frames_head) = new_free_frame;
            (free_frames_tail) = new_free_frame;
        }
        else
        {
            free_frames_tail->next = new_free_frame;
            free_frames_tail = new_free_frame;
        }
    }
    return;
}

//Initialize the frame table. Make all entries in the table as invalid.
void mm_initialize_frame_table(frame_table_struct *frame_table){
    for(int i=0; i<NO_OF_FRAMES; i++){
        frame_table->frame_entry[i].valid = 0;
        frame_table->frame_entry[i].desc = 'i';
        frame_table->frame_entry[i].page_pointer = NULL;
    }
}

//Initialize the outer page table for the process. Update the pcb of that process with the outer page. table frame
void mm_initialize_page_table(int pid, main_memory_struct **main_memory){

    //Finding frame for the inner page table.
    logical_address la;
    la.inner_pt = la.middle_pt = la.outer_pt = -1;
    int outer_pt_frame = mm_new_page_table(pid, la, main_memory);

    //Update the pcb of this process in the kernel structure.
    kernel_struct *kernel = (*main_memory)->frame_table.frame_entry[0].page_pointer;
    for(int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == -1){
            kernel->pcb[i].pid = pid;
            kernel->pcb[i].outer_pt_frame_no = outer_pt_frame;
            break;
        }
    }


}

//Initializing main memory. Initialize the frame table and put all the frames in the free frame list.
main_memory_struct* mm_initialize_mm(){
    main_memory_struct* new_mm = malloc(sizeof(main_memory_struct));
    new_mm->free_frames_head = NULL;

    mm_initialize_free_frames(&(new_mm->free_frames_head));
    mm_initialize_frame_table(&(new_mm->frame_table));
    return new_mm;
}

//Initializing the kernel. Frame no 0 is given to the kernel to store all the pcbs of the processess. This frame will never be replaced.
void mm_initialize_kernel(main_memory_struct** main_memory){
    kernel_struct* kernel = malloc(sizeof(kernel));
    kernel->pcb = malloc(sizeof(pcb_struct)*NUM_PROCESSES);

    for(int i=0; i<NUM_PROCESSES; i++){
        kernel->pcb[i].pid = -1;
    }

    int free_frame = mm_get_free_frame(main_memory); //Frame 0 will always be given as this function is called during initialization.
    logical_address invalid;
    invalid.outer_pt = invalid.middle_pt = invalid.inner_pt = -1;

    mm_update_frame_table(&((*main_memory)->frame_table), free_frame, 0, 'k', kernel, invalid);
    mm_update_counter(&((*main_memory)->frame_table), free_frame);

    return;
}

//Prefetch two pages of the process in main memory.
void mm_prefetch_pages(int pid, FILE* process, main_memory_struct** main_memory){

    //Pcb stores the frame number of outer page table of the process.
    kernel_struct *kernel = (*main_memory)->frame_table.frame_entry[0].page_pointer;
    int outer_pt_frame_no = mm_get_outer_pt_frame_no(pid, kernel);

    //Get the pointer to outer page table from the frame table.
    page_table_struct *outer_page_table = (*main_memory)->frame_table.frame_entry[outer_pt_frame_no].page_pointer;

    int prefetch_pages = 0;

    //Prefetch two pages.
    while(!feof(process)){
        int n;
        fscanf(process, "%x", &n);
        logical_address la = mm_convert(n);

        
        int frame_no;
        int count = 0;
        
        //Search for the logical address in the page tables
        while((frame_no = mm_search_page_table(la, pid, main_memory)) == -1){
            count++;
        }

        //If count > 0, the logical address resulted in a page fault and thus, it was prefetched in the main memory. Thus, increase the number of prefetched pages.
        if(count>0)
            prefetch_pages++;
        
        if(prefetch_pages == REQUIRED_PREFETCHED_PAGES)
            break;

    }

    //Resetting the file pointer to start from the first address again.
    fseek(process, 0, SEEK_SET);

}

//Function to update the page table. The entry for that frame no is made valid and the frame no is stored in that entry.
void mm_update_page_table(page_table_struct **page_table, int entry, int frame_no){
    (*page_table)->page_entry[entry].frame_no = frame_no;
    (*page_table)->page_entry[entry].valid = 1;
}

//Function to update the frame table. Frame table stores the pointer to the data structure present
//It also stores the logical page no which is required during replacement.
void mm_update_frame_table(frame_table_struct *frame_table, int free_frame, int pid, char desc, void* pointer, logical_address la){
    frame_table->frame_entry[free_frame].valid = 1;
    frame_table->frame_entry[free_frame].pid = pid;
    frame_table->frame_entry[free_frame].counter = 0;
    frame_table->frame_entry[free_frame].desc = desc;
    frame_table->frame_entry[free_frame].page_pointer = pointer;
    frame_table->frame_entry[free_frame].page_no = la;
    frame_table->frame_entry[free_frame].modified = 0;

}

//Function to update the LRU counter. All the entries will be right-shifted by 1 and the currently accessed entry's MSB will be made 1.
void mm_update_counter(frame_table_struct *frame_table, int frame){
    for(int i=0; i<NO_OF_FRAMES; i++){
        if(frame_table->frame_entry[i].valid & 0x1 == 1){
            frame_table->frame_entry[i].counter = (frame_table->frame_entry[i].counter & 0xFFFF)>> 1;
        }
    }

    frame_table->frame_entry[frame].counter = (frame_table->frame_entry[frame].counter & 0xFFFF) | 0x8000;   

}

//Function to update the PCB. PCB will be updated when the frame no occupied by the outer page table is changed.
void mm_update_pcb(int pid, kernel_struct **kernel, int frame_no){
    for(int i=0; i<NUM_PROCESSES; i++){
        if((*kernel)->pcb[i].pid == pid){
            (*kernel)->pcb[i].outer_pt_frame_no = frame_no;
        }
    }
}

//Function to get the frame number of the outer page table for that process.
int mm_get_outer_pt_frame_no(int pid, kernel_struct* kernel){
    for(int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == pid)
            return kernel->pcb[i].outer_pt_frame_no;
    }
    return -1;
}


//Returns the page table the frame is pointing to.
page_table_struct* mm_get_page_table(page_table_struct* page_table, main_memory_struct *main_memory, int entry){
    int frame_no = page_table->page_entry[entry].frame_no;
    return (page_table_struct*)(main_memory->frame_table.frame_entry[frame_no].page_pointer);
}

//Return the frame number of the free frame. If the free frame list is empty, replace a page from the frame table and return that frame no.
int mm_get_free_frame(main_memory_struct **main_memory){
    if((*main_memory)->free_frames_head == NULL){
        //Find a frame to replace. 
        int lru_frame = mm_replace_page(main_memory);
        return lru_frame;
    }
    int free_frame_no = (*main_memory)->free_frames_head->frame_no;
    free_frames_struct *temp = (*main_memory)->free_frames_head;
    (*main_memory)->free_frames_head = (*main_memory)->free_frames_head->next;
    free(temp);
    return free_frame_no;
}


//Create a new page table if the previous tables become full.
int mm_new_page_table(int pid, logical_address la, main_memory_struct** main_memory){
    page_table_struct *page_table = malloc(sizeof(page_table_struct));

    //Get a new free frame to store the page table.
    int free_frame = mm_get_free_frame(main_memory);

    //Update the frame table.
    mm_update_frame_table(&(*main_memory)->frame_table, free_frame, pid, 'p', page_table, la);

    //Update the counter of the free frame. That frame is the MRU.
    mm_update_counter(&((*main_memory)->frame_table), free_frame);

    //Make all the entries in the new page table as invalid.
    for(int i=0; i<NO_OF_ENTRIES; i++){
        page_table->page_entry[i].valid = 0;
        page_table->page_entry[i].frame_no = 0;
    }

    return free_frame;
}


//Load new page from secondary memory into main memory.
int mm_load_page(int pid, logical_address la, main_memory_struct **main_memory){
    //Get a free frame to load the page.
    int free_frame = mm_get_free_frame(main_memory);

    //Update the frame table to reflect who is owning this frame.
    mm_update_frame_table(&(*main_memory)->frame_table, free_frame, pid, 'f', NULL, la);
    mm_update_counter(&(*main_memory)->frame_table, free_frame);

    return free_frame;
}


//Search the hierarchical page table for the given logical addresss.
int mm_search_page_table(logical_address la, int pid, main_memory_struct **main_memory){
    kernel_struct *kernel = (*main_memory)->frame_table.frame_entry[0].page_pointer;

    //First get the frame table of the outer page table from the pcb.
    int outer_pt_frame = mm_get_outer_pt_frame_no(pid, kernel);

    //This step is necessary to check whether the outer page table is swapped out or not.
    //If it is swapped out, then the frame number will be -1 and we will need to get a new page table for the outer page table.
    //This is a page fault and the instruction needs to be executed again.
    if(outer_pt_frame == -1){    
        logical_address la;
        la.inner_pt = la.middle_pt = la.outer_pt = -1;
        outer_pt_frame = mm_new_page_table(pid, la, main_memory);
        mm_update_pcb(pid, &kernel, outer_pt_frame);
        return -1;
    }

    page_table_struct* outer_page_table = (*main_memory)->frame_table.frame_entry[outer_pt_frame].page_pointer;

    //Outer page table is the MRU one.
    mm_update_counter(&((*main_memory)->frame_table), outer_pt_frame);

    //Using the outer offset, check whether the entry for the middle page table is valid or not.
    //If the entry is invalid, create a new middle page table and update the outer page table.
    //This is a page fault and the instruction needs to be executed again.
    int outer_offset = la.outer_pt & 0x3f;

    if(outer_page_table->page_entry[outer_offset].valid == 0){
    //    printf("Page fault for middle page table\n");
        int new_middle_pt_frame = mm_new_page_table(pid, la, main_memory);
        mm_update_page_table(&outer_page_table, outer_offset, new_middle_pt_frame);
        return -1;
    }

    int middle_pt_frame = outer_page_table->page_entry[outer_offset].frame_no;
    page_table_struct* middle_page_table = (*main_memory)->frame_table.frame_entry[middle_pt_frame].page_pointer;


    //Middle page table is the MRU one.
    mm_update_counter(&((*main_memory)->frame_table), middle_pt_frame);

    //Using the middle offset, check whether the entry for the inner page table is valid or not.
    //If entry is invalid, create a new inner page table and update the middle page table.
    //This is a page fault and the instruction needs to be executed again.
    int middle_offset = la.middle_pt & 0x1ff;
    
    if(middle_page_table->page_entry[middle_offset].valid  == 0){
    //    printf("Page fault for inner page table\n");
        int new_inner_pt_frame = mm_new_page_table(pid, la, main_memory);
        mm_update_page_table(&middle_page_table, middle_offset, new_inner_pt_frame);
        return -1;
    }


    int inner_pt_frame = middle_page_table->page_entry[middle_offset].frame_no;
    page_table_struct *inner_page_table = (*main_memory)->frame_table.frame_entry[inner_pt_frame].page_pointer;


    //Inner page table is the MRU one.
    mm_update_counter(&((*main_memory)->frame_table), inner_pt_frame);

    //Using the inner offset, check whether the entry for the frame is valid or not.
    //If entry is invalid, load a new page from secondary memory into the primary and update the page table.
    //This is a page fault and the instruction needs to be executed again.
    int inner_offset = la.inner_pt & 0x1ff;
    if(inner_page_table->page_entry[inner_offset].valid == 0){
    //    printf("Page fault for process page\n");
        int new_frame_no = mm_load_page(pid, la, main_memory);
        mm_update_page_table(&inner_page_table, inner_offset, new_frame_no);
        return -1;
    }


    //printf("Page hit\n");

    //Return the frame no if we get a match.
    int frame_no = inner_page_table->page_entry[inner_offset].frame_no;

    //The frame required is the MRU one.
    mm_update_counter(&((*main_memory)->frame_table), frame_no);

    return (frame_no);

}

//Function to replace the page incase the free frames list gets empty.
int mm_replace_page(main_memory_struct **main_memory){
    kernel_struct *kernel = (*main_memory)->frame_table.frame_entry[0].page_pointer;

    frame_table_struct frame_table = (*main_memory)->frame_table;
    int lru_frame = 0;
    int min_counter = __INT32_MAX__;
    int pid, outer_pt_frame_no;
    logical_address page_no;

    //Find the frame entry with the least value of counter.
    for(int frame=1; frame<NO_OF_FRAMES; frame++){
        if(min_counter > (frame_table.frame_entry[frame].counter & 0xFFFF)){
            min_counter = (frame_table.frame_entry[frame].counter & 0xFFFF);
            lru_frame = frame;
        }
    }

    pid = frame_table.frame_entry[lru_frame].pid;
    page_no = frame_table.frame_entry[lru_frame].page_no;
    outer_pt_frame_no = mm_get_outer_pt_frame_no(pid, kernel);

    //printf("Replacing frame = %d\n", lru_frame);

    //Check whether the frame we are replacing is dirty or not. If it is dirty, initiate write to disk and clear the dirty bit.
    if(mm_check_dirty_bit(lru_frame, (*main_memory)) == 1){
        mm_write_to_disk(lru_frame, main_memory);
        (*main_memory)->frame_table.frame_entry[lru_frame].modified = 0;
    }

    //Make that entry in the frame table as invalid and the frame it was pointing to as NULL.
    //The pid is also made as -1. It is used as a delimeter so that we know which entry to invalidate.
    (*main_memory)->frame_table.frame_entry[lru_frame].valid = 0;
    (*main_memory)->frame_table.frame_entry[lru_frame].page_pointer = NULL;
    (*main_memory)->frame_table.frame_entry[lru_frame].pid = -1;

    //Check whether the lru frame contained the outer page table for some other process or not.
    //If it did, then update the pcb of the other process by setting the frame no of outer page table as -1 
    if(outer_pt_frame_no == lru_frame){
        mm_update_pcb(pid, &kernel, -1);
        return lru_frame;
    }

    //Else we invalidate the entries in the page tables for the logical address stored in that frame. 
    mm_invalidate_page_table_entries(page_no, outer_pt_frame_no, (*main_memory));

    return lru_frame;
}

//Function to invalidate the entries in the page table for the given logical address.
//We have to only make that entry invalid which points to an entry whose pid we have set as -1 in the frame table.
void mm_invalidate_page_table_entries(logical_address page_no, int outer_frame_no, main_memory_struct *main_memory){
    int outer_pt = page_no.outer_pt & 0x3f;
    int middle_pt = page_no.middle_pt & 0x1ff;
    int inner_pt = page_no.inner_pt & 0x1ff;

    if(outer_frame_no == -1){
        return;
    }

    page_table_struct *outer_page_table = main_memory->frame_table.frame_entry[outer_frame_no].page_pointer;    


    int middle_frame_no = outer_page_table->page_entry[outer_pt].frame_no;

    //If we are replacing the middle page table, the pid for this frame will be -1 in the frame table and thus, we have to invalidate the entry in the outer page table only.
    if(main_memory->frame_table.frame_entry[middle_frame_no].pid == -1 || outer_page_table->page_entry[outer_pt].valid == 0){
        outer_page_table->page_entry[outer_pt].valid = 0;
        return;
    }

    page_table_struct *middle_page_table = main_memory->frame_table.frame_entry[middle_frame_no].page_pointer;

    int inner_frame_no = middle_page_table->page_entry[middle_pt].frame_no;

    //If we are replacing the inner page table, the pid for this frame will be -1 in the frame table and thus, we have to invalidate the entry in the middle page table only
    //Note that we will not invalidate the entry in the outer page table as the middle page table is not replaced, only the inner page table is replaced.
    if(main_memory->frame_table.frame_entry[inner_frame_no].pid == -1 || middle_page_table->page_entry[middle_pt].valid == 0){
        middle_page_table->page_entry[middle_pt].valid = 0;
        return;
    }

    //If we reach here, that means we are invalidating a frame and thus, we will invalidate the entry only in the inner page table.
    page_table_struct *inner_page_table = main_memory->frame_table.frame_entry[inner_frame_no].page_pointer;
    inner_page_table->page_entry[inner_pt].valid = 0;

    return;
}

//Check if the frame we are replacing is dirty or not. This function is also needed while termination of process.
int mm_check_dirty_bit(int frame_no, main_memory_struct *main_memory){
    return (main_memory->frame_table.frame_entry[frame_no].modified & 0x1 == 1);    

}

//Function for cleaning up when process terminates.
void mm_terminate_process(int pid, main_memory_struct **main_memory){
    kernel_struct *kernel = (*main_memory)->frame_table.frame_entry[0].page_pointer;
    free_frames_struct *frames_tail = (*main_memory)->free_frames_head;

    //Free all the frames owned by this process and add them to the free frames list.
    for(int i=0; i<NO_OF_FRAMES; i++){
        if((*main_memory)->frame_table.frame_entry[i].pid == pid){
            
            //Check whether the frame is dirty or not. If it is, initiate write to disk and clear the dirty bit.
            if(mm_check_dirty_bit(i, (*main_memory)) == 1){
                mm_write_to_disk(i, main_memory);
                (*main_memory)->frame_table.frame_entry[i].modified = 0;
            }

            (*main_memory)->frame_table.frame_entry[i].valid = 0;
            (*main_memory)->frame_table.frame_entry[i].pid = -1;

            //Adding a free frame to the free frame list.
            free_frames_struct *free_frame = malloc(sizeof(free_frames_struct));
            free_frame->frame_no = i;
            free_frame->next = NULL;

            if(frames_tail == NULL){
                frames_tail = free_frame;
                (*main_memory)->free_frames_head = free_frame;
            }
            else{
                while(frames_tail->next != NULL){
                    frames_tail = frames_tail->next;
                }
                frames_tail->next = free_frame;
                frames_tail = free_frame;
            }

        }
    }

    //Make the pid in the pcb as -1 to indicate this process has terminated.
    for(int i=0; i<NUM_PROCESSES; i++){
        if(kernel->pcb[i].pid == pid){
            kernel->pcb[i].pid = -1;
        }
    }

}

//Function to write given frame to disk
void mm_write_to_disk(int frame_no, main_memory_struct **main_memory){
    /*
        Function to write to secondary storage
        Input - Frame for writing to disk
    */
}

//Function to write data from L2 to main memory. The dirty bit for this page is set to 1.
void mm_write_to_mm(int physical_address, main_memory_struct **main_memory, char *data){
    int frame_no = physical_address>>10;
    int offset = physical_address & 0x3ff;

    (*main_memory)->frame_table.frame_entry[frame_no].modified = 1;

    /*
        Write data in the frame using the frame no and the offset. 
    */

   return;

}

//Convert given logical address into offset, inner bits, middle bits and outer bits
logical_address mm_convert(int x){
    logical_address la;
    la.offset = x & 0x3ff;
    x = x>>10;
    la.inner_pt = x & 0x1ff;
    x = x>>9;
    la.middle_pt = x & 0x1ff;
    x = x>>9;
    la.outer_pt = x & 0xf;

    //printf("Outer = %x, middle= %x, inner = %x\n", la.outer_pt & 0xf, la.middle_pt & 0x1ff, la.inner_pt & 0x1ff);

    return la;
}

//Convert the given logical address structure back to an integer.
int mm_convert_back(logical_address la){
    int x = 0;
    x = x << 4;
    x = x | (la.outer_pt & 0xf);
    x = x << 9;
    x = x | (la.middle_pt & 0x1ff);
    x = x << 9;
    x = x | (la.inner_pt & 0x1ff);
    x = x << 10;
    x = x | (la.offset & 0x3ff); 
    return x;
}