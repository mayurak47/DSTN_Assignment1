#include "functions.h"

void mm_print_frame_table(frame_table_struct frame_table);
void mm_print_page_table(page_table_struct *page_table);
void mm_initialize_free_frames(free_frames_struct** free_frames_head);
void mm_initialize_frame_table(frame_table_struct* frame_table);
void mm_update_page_table(page_table_struct** page_table, unsigned int entry, page_info page);
void mm_update_counter(frame_table_struct *frame_table, unsigned int frame);
page_info mm_new_page_table(unsigned int pid, logical_address_struct la, main_memory_struct** main_memory, kernel_struct *kernel);
page_info mm_load_page(unsigned int pid, logical_address_struct la, main_memory_struct** main_memory, kernel_struct *kernel);
void mm_invalidate_page_table_entries(logical_address_struct page_no, unsigned int pid, main_memory_struct *main_memory, kernel_struct *kernel);
unsigned int mm_convert_back(logical_address_struct la);
unsigned int mm_check_dirty_bit(unsigned int frame_no, main_memory_struct *main_memory);

/*
    Input of function - Input is the frame table.
    Purpose of the function - Printing the frame table
    Output/Result of function - Does not return any value. Outputs various fields of the frame table
*/
void mm_print_frame_table(frame_table_struct frame_table){
    printf(" ________________________________________________________________________________________________________________________\n");
    printf("|    FRAME NO   |     VALID\t|\tPID\t|\tP/F\t|     Counter\t|\tLogical Address\t|\tPage Pointer\t|\n");
    for(unsigned int i=0; i<NO_OF_FRAMES; i++){
        if((frame_table.frame_entry[i].valid & 0x1) == 1){
            printf("|\t%d\t|\t%d\t|\t%d\t|\t%s\t|\t%x\t|\t%x\t|\n", i, frame_table.frame_entry[i].valid, frame_table.frame_entry[i].pid, frame_table.frame_entry[i].desc == 'f' ? "Frame" : (frame_table.frame_entry[i].desc == 'p' ? "Page" : "Kernel"), frame_table.frame_entry[i].counter & 0xFFFF, mm_convert_back(frame_table.frame_entry[i].page_no));
        }
    }
    printf(" ------------------------------------------------------------------------------------------------------------------------\n");
}

/*
    Input of function - Input is the page table.
    Purpose of the function - Prints the page table
    Output/Result of function - Does not return any value. Outputs various fields of the page table.
*/
void mm_print_page_table(page_table_struct *page_table){
    printf("Page table\n");
    printf("_________________________________________________\n");
    printf("|    INDEX\t|\tV/I\t|    FRAME NO   |\n");
    for(unsigned int i=0; i<NO_OF_PT_ENTRIES; i++){
        if((page_table->page_entry[i].valid & 0x1) == 1){
            printf("|\t%d\t|\t%d\t|\t%d\t|\n", i, page_table->page_entry[i].valid, page_table->page_entry[i].page.frame_no);
        }
    }
    printf("-------------------------------------------------\n");
}

/*
    Input of function - Input is the head to the free frames list.
                    free_frames_head == NULL
    Purpose of the function - Initializes the free frames list. Puts all the frames in the free frames list.
    Output/Result of function - Updated free frames list. Should contain all the free frames.
*/
void mm_initialize_free_frames(free_frames_struct **free_frames_head){
    free_frames_struct *free_frames_tail = (*free_frames_head);

    for(unsigned int i=0; i<NO_OF_FRAMES; i++){
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

/*
    Input of function - Input is the main memory structure and the number of the frame to be added.
    Purpose of the function - Add a new free frame to the free frame list with the frame number equal to the number passed in the function.
*/
void mm_add_free_frame(main_memory_struct **main_memory, int free_frame_no){
    free_frames_struct *frames_tail = (*main_memory)->free_frames_head;
    free_frames_struct *free_frame = malloc(sizeof(free_frames_struct));
            free_frame->frame_no = free_frame_no;
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

/*
    Input of function - Input is the frame table.
    Purpose of the function - Initializes the frame table by setting all the entries as invalid.
                              The desc field specifies the type of structure present in the frame. 
                              i - invalid, p - page table, f- process page, k - kernel
    Output/Result of function - Updated frame table.
*/
void mm_initialize_frame_table(frame_table_struct *frame_table){
    for(unsigned int i=0; i<NO_OF_FRAMES; i++){
        frame_table->frame_entry[i].valid = 0;
        frame_table->frame_entry[i].desc = 'i';
    }
}

/*
    Input of function - Input is the pid of process, main memory structure and kernel structure
    Purpose of the function - Creates an outer page table for the process and returns the information.
    Output/Result of function - Takes a free frame from the free frames list and creates a new page table.
*/
page_info mm_initialize_page_table(unsigned int pid, main_memory_struct **main_memory, kernel_struct* kernel){

    //Finding frame for the inner page table.
    logical_address_struct la;
    la.inner_pt = la.middle_pt = la.outer_pt = -1;
    page_info outer_page = mm_new_page_table(pid, la, main_memory, kernel);

    return outer_page;

}

/*
    Input of function - No input is necessary for the function.
    Purpose of the function - Initializes the main memory. The main memory structure contains the free frames list and the frame table.
                              Initialization for both these structures is called in this function.
    Output/Result of function - Returns the pointer to the main memory structure.
*/
void mm_initialize_mm(){
    main_memory = malloc(sizeof(main_memory_struct));
    main_memory->free_frames_head = NULL;

    mm_initialize_free_frames(&(main_memory->free_frames_head));
    mm_initialize_frame_table(&(main_memory->frame_table));
}

/*
    Input of function - Input is the pid of process, the input file of the process, main memory structure and kernel structure.
    Purpose of the function - Prefetches two pages of the process in the main memory. The CR3 register in the kernel structure stores the outer page table information of the process.
                              Using that, the page tables are updated and two pages are brought corresponding to the first two addresses in the input file.
    Output/Result of function - Updated page tables containing information about the two pages brought into main memory.
*/
void mm_prefetch_pages(unsigned int pid, FILE* process, main_memory_struct** main_memory, kernel_struct* kernel){

    //Pcb stores the frame number of outer page table of the process.
    page_info outer_page = kernel->CR3_reg;
    page_table_struct* outer_page_table = outer_page.page_pointer;

    unsigned int prefetch_pages = 0;

    //Prefetch two pages.
    while(!feof(process)){
        unsigned int n;
        fscanf(process, "%x", &n);
        logical_address_struct la = mm_convert(n);

        
        unsigned int frame_no;
        unsigned int count = 0;
        
        //Search for the logical address in the page tables
        while((frame_no = mm_search_page_table(la, pid, main_memory, kernel)) == -1){
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

/*
    Input of function - Input is the page table, the entry in the page table and the information to be stored in that entry.
    Purpose of the function - Updates the page table by storing the page information in the corresponding page table entry.
    Input of function - Updated page table with the valid bit set.
*/
void mm_update_page_table(page_table_struct **page_table, unsigned int entry, page_info page){
    (*page_table)->page_entry[entry].valid = 1;
    (*page_table)->page_entry[entry].page = page;
}

/*
    Input of function - Input is the frame table, the entry in the frame table and the various values of the fields in the frame table.
    Purpose of the function - Updates the frame table by storing the various values passed in the corresponding frame table entry.
                              The values passed are - pid of process, description of the type of structure present in the frame - 'i' is invalid, 'p' is page table, 'f' is process page and 'k' is kernel,
                              the logical address of the process which holds this frame (this is needed during replacement of the frame).
    Output/Result of function - Updated frame table with the valid bit set and modified bit reset.
*/
void mm_update_frame_table(frame_table_struct *frame_table, unsigned int free_frame, unsigned int pid, char desc, logical_address_struct la){
    frame_table->frame_entry[free_frame].valid = 1;
    frame_table->frame_entry[free_frame].pid = pid;
    frame_table->frame_entry[free_frame].counter = 0;
    frame_table->frame_entry[free_frame].desc = desc;
    frame_table->frame_entry[free_frame].page_no = la;
    frame_table->frame_entry[free_frame].modified = 0;

}

/*
    Input of function - Input is the frame table and the entry in the frame table.
    Purpose of the function - Updates the 16 bit counter corresponding to the frame table entry. All the counters of the frame table are right-shifted by 1 and the MSB of the given entry passed in the function is set to 1.
    Output/Result of function - Updated frame table with the counters right shifted by 1 and the current entry's counter's MSB set to 1.
*/
void mm_update_counter(frame_table_struct *frame_table, unsigned int frame){
    for(unsigned int i=0; i<NO_OF_FRAMES; i++){
        if((frame_table->frame_entry[i].valid & 0x1) == 1){
            frame_table->frame_entry[i].counter = (frame_table->frame_entry[i].counter & 0xFFFF)>> 1;
        }
    }

    frame_table->frame_entry[frame].counter = (frame_table->frame_entry[frame].counter & 0xFFFF) | 0x8000;   

}

/*
    Input of function - Input is the main memory structure and the kernel structure (required during replacement)
    Purpose of the function - If the free frames list is not empty, it removes one free frame from the list and returns the frame number.
                              If the free frames list is empty, the function for replacement is called which will return the lru frame. This lru frame number is returned.
    Output/Result of function - Returns the free frame / lru frame.
*/
unsigned int mm_get_free_frame(main_memory_struct **main_memory, kernel_struct *kernel){
    if((*main_memory)->free_frames_head == NULL){
        //Find a frame to replace. 
        unsigned int lru_frame = mm_replace_page(main_memory, kernel);
        return lru_frame;
    }

    unsigned int free_frame_no = (*main_memory)->free_frames_head->frame_no;
    free_frames_struct *temp = (*main_memory)->free_frames_head;
    (*main_memory)->free_frames_head = (*main_memory)->free_frames_head->next;
    free(temp);
    return free_frame_no;
}

/*
    Input of function - Input is the pid of process, the logical address which the process is executing, the main memory structure and the kernel structure (required for get_free_frame function)
    Purpose of the function - Creates a new page table when there is a page table fault and returns the information about the new page table. The structue page_info which is returned contains two fields 
                              - the frame no occupied by this page and the page table pointer of this page table. The frame table is also updated to indicate that a frame is occupied by the process.
                              All the entries in this new page table are invalid.
    Output/Result of function - Returns the information about the new page table.
*/
page_info mm_new_page_table(unsigned int pid, logical_address_struct la, main_memory_struct** main_memory, kernel_struct *kernel){
    page_table_struct *page_table = malloc(sizeof(page_table_struct));
    // kernel_struct *kernel = (*main_memory)->frame_table.frame_entry[0].page_pointer;

    //Get a new free frame to store the page table.
    unsigned int free_frame = mm_get_free_frame(main_memory, kernel);

    //Update the frame table.
    mm_update_frame_table(&(*main_memory)->frame_table, free_frame, pid, 'p', la);

    //Update the counter of the free frame. That frame is the MRU.
    mm_update_counter(&((*main_memory)->frame_table), free_frame);

    //Make all the entries in the new page table as invalid.
    for(unsigned int i=0; i<NO_OF_PT_ENTRIES; i++){
        page_table->page_entry[i].valid = 0;
        page_table->page_entry[i].page.frame_no = 0;
        page_table->page_entry[i].page.page_pointer = NULL;
    }

    page_info page_info;
    page_info.frame_no = free_frame;
    page_info.page_pointer = page_table;

    return page_info;
}

/*
    Input of function - Input is the pid of process, the logical address which the process is executing, the main memory structure and the kernel structure (required for get_free_frame function)
    Purpose of the function - Loads a new process page from disk to main memory and returns the information about the new page loaded. The structue page_info which is returned contains two fields 
                              - the frame no occupied by this page and the  pointer of the frame occupied by this page. The frame table is also updated to indicate that a frame is occupied by the process.
    Output/Result of function - Returns the information about the new page loaded in main memory.
*/
page_info mm_load_page(unsigned int pid, logical_address_struct la, main_memory_struct **main_memory, kernel_struct* kernel){
    frame* new_frame = malloc(sizeof(frame));

    //Get a free frame to load the page.
    unsigned int free_frame = mm_get_free_frame(main_memory, kernel);

    //Update the frame table to reflect who is owning this frame.
    mm_update_frame_table(&(*main_memory)->frame_table, free_frame, pid, 'f', la);
    mm_update_counter(&(*main_memory)->frame_table, free_frame);

    page_info new_page;
    new_page.frame_no = free_frame;
    new_page.page_pointer = new_frame;

    return new_page;
}

/*
    Input of function - Input is the logical address. pid of process, main memory structure and the kernel structure.
    Purpose of the function - Searches the page tables for the frame occupied by this process using the logical address.
                              This is hierarchical paging with 3 level page tables. The page table pointer of the outer page table is stored in the CR3 register of the kernel.
                              Using the outer offset (first 9 bits of logical address), the middle page table pointer is found. If it is NULL, then it is a page fault. Create a new middle page table and return -1 to indicates it is a page fault.
                              If middle page table is valid, using middle offset, the inner page table pointer is found. Same procedure for inner page table is followed.
                              If inner page table is valid, using inner offset, the process page pointer is found. If it is NULL, we load a new process page into main memory and return -1 to indicate it is a page fault.
                              If process page is valid, we return the frame number.
    Output/Result of function - Returns the frame number of the process page is it is valid. Else. return -1 to indicated it is a page fault.

*/
unsigned int mm_search_page_table(logical_address_struct la, unsigned int pid, main_memory_struct **main_memory, kernel_struct* kernel){

    //First get the frame table of the outer page table from the pcb.
    page_info outer_page = kernel->CR3_reg;
    page_table_struct* outer_page_table = outer_page.page_pointer;
    unsigned int outer_pt_frame_no = outer_page.frame_no;


    //This step is necessary to check whether the outer page table is swapped out or not.
    //If it is swapped out, then the frame number will be -1 and we will need to get a new page table for the outer page table.
    //This is a page fault and the instruction needs to be executed again.
    if(kernel_check_valid_bit(pid, kernel) == 0){    

        // printf("| Page fault : Outer page table exists in swap space |");
        int new_frame_no = mm_get_free_frame(main_memory, kernel);
        mm_update_frame_table(&(*main_memory)->frame_table, new_frame_no, pid, 'p', la);
        kernel_set_valid_bit(pid, kernel);
    }

    //Outer page table is the MRU one.
    mm_update_counter(&((*main_memory)->frame_table), outer_pt_frame_no);

    /*------------------------------------------OUTER LEVEL---------------------------------------*/

    //Using the outer offset, check whether the entry for the middle page table is valid or not.
    //If the entry is invalid, create a new middle page table and update the outer page table.
    //This is a page fault and the instruction needs to be executed again.
    unsigned int outer_offset = la.outer_pt & 0x3f;

    total_access_time += mm_page_lookup_time;

    if(outer_page_table->page_entry[outer_offset].valid == 0){
        //If the page pointer is not null, then the page table exists in the swap space. Thus, the page table has to be brought from the swap space into the main memory. 
        //This is not a page fault.
        if(outer_page_table->page_entry[outer_offset].page.page_pointer != NULL){
            // printf("| Page fault : Middle page table exists in swap space |");
            outer_page_table->page_entry[outer_offset].valid = 1;
            int new_frame_no = mm_get_free_frame(main_memory, kernel);
            outer_page_table->page_entry[outer_offset].page.frame_no = new_frame_no;
            mm_update_frame_table(&(*main_memory)->frame_table, new_frame_no, pid, 'p', la);

            total_access_time += swap_space_to_mm_time;

        }

        //Else, it is a page fault. A new page table has to be created. This results in a context switch and the instruction needs to be restarted.
        else{
            // printf("| Page fault : Middle page table doesn't exist |");
            page_info middle_page = mm_new_page_table(pid, la, main_memory, kernel);
            mm_update_page_table(&outer_page_table, outer_offset, middle_page);

            total_access_time += page_fault_time;

            return -1;
        }
    }

    /*----------------------------------------------------------------------------------------------*/

    unsigned int middle_pt_frame = outer_page_table->page_entry[outer_offset].page.frame_no;
    page_table_struct* middle_page_table = outer_page_table->page_entry[outer_offset].page.page_pointer;


    //Middle page table is the MRU one.
    mm_update_counter(&((*main_memory)->frame_table), middle_pt_frame);

    //Using the middle offset, check whether the entry for the inner page table is valid or not.
    //If entry is invalid, create a new inner page table and update the middle page table.
    //This is a page fault and the instruction needs to be executed again.
    unsigned int middle_offset = la.middle_pt & 0x1ff;
    
    /*------------------------------------------MIDDLE LEVEL---------------------------------------*/

    total_access_time += mm_page_lookup_time;

    if(middle_page_table->page_entry[middle_offset].valid  == 0){
        //If the page pointer is not null, then the page table exists in the swap space. Thus, the page table has to be brought from the swap space into the main memory. 
        //This is not a page fault.
        if(middle_page_table->page_entry[middle_offset].page.page_pointer != NULL){
            // printf("| Page fault : Inner page table exists in swap space |");
            middle_page_table->page_entry[middle_offset].valid = 1;
            int new_frame_no = mm_get_free_frame(main_memory, kernel);
            middle_page_table->page_entry[middle_offset].page.frame_no = new_frame_no;
            mm_update_frame_table(&(*main_memory)->frame_table, new_frame_no, pid, 'p', la);

            total_access_time += swap_space_to_mm_time;

        }
        //Else, it is a page fault. A new page table has to be created. This results in a context switch and the instruction needs to be restarted.
        else{
            // printf("| Page fault : Inner page table doesn't exist |");
            page_info inner_page = mm_new_page_table(pid, la, main_memory, kernel);          
            mm_update_page_table(&middle_page_table, middle_offset, inner_page);

            total_access_time += page_fault_time;

            return -1;
        }
    }

    /*----------------------------------------------------------------------------------------------*/

    unsigned int inner_pt_frame = middle_page_table->page_entry[middle_offset].page.frame_no;
    page_table_struct *inner_page_table = middle_page_table->page_entry[middle_offset].page.page_pointer;

    //Inner page table is the MRU one.
    mm_update_counter(&((*main_memory)->frame_table), inner_pt_frame);


    //Using the inner offset, check whether the entry for the frame is valid or not.
    //If entry is invalid, load a new page from secondary memory into the primary and update the page table.
    //This is a page fault and the instruction needs to be executed again.
    unsigned int inner_offset = la.inner_pt & 0x1ff;

    /*------------------------------------------INNER LEVEL---------------------------------------*/

    total_access_time += mm_page_lookup_time;

    if(inner_page_table->page_entry[inner_offset].valid == 0){
        //If the page pointer is not null, then the page table exists in the swap space. Thus, the page table has to be brought from the swap space into the main memory. 
        //This is not a page fault.
        if(inner_page_table->page_entry[inner_offset].page.page_pointer != NULL){
            // printf("| Page fault : Process page exists in swap space |");
            inner_page_table->page_entry[inner_offset].valid = 1;
            int new_frame_no = mm_get_free_frame(main_memory, kernel);
            inner_page_table->page_entry[inner_offset].page.frame_no = new_frame_no;
            mm_update_frame_table(&(*main_memory)->frame_table, new_frame_no, pid, 'p', la);

            total_access_time += swap_space_to_mm_time;

        }
        //Else, it is a page fault. A new page table has to be created. This results in a context switch and the instruction needs to be restarted.
        else{
            // printf("| Page fault : Process page doesn't exist |");
            page_info new_frame = mm_load_page(pid, la, main_memory, kernel);
            mm_update_page_table(&inner_page_table, inner_offset, new_frame);

            total_access_time += disk_to_mm_time;

            return -1;
        }
    }

    /*----------------------------------------------------------------------------------------------*/

    //printf("Page hit\n");

    /*------------------------------------------FRAME LEVEL---------------------------------------*/

    //Return the frame no if we get a match.
    unsigned int frame_no = inner_page_table->page_entry[inner_offset].page.frame_no;

    total_access_time += mm_page_lookup_time;

    //The frame required is the MRU one.
    mm_update_counter(&((*main_memory)->frame_table), frame_no);

    return (frame_no);

}

/*
    Input of function - Input is the main memory structure and the kernel structure.
    Purpose of the function - Select a frame from the frame table for replacement. The replacement policy used is LRU with global replacement.
                              If the selected frame is dirty (modified bit is set), write this frame back to disk.
                              If the selected frame to replace contains the outer page table of some other process, set the v/i bit of the outer page table for that process in the kernel to 0.
                              Else, invalidate the entry in the page table which pointed to the selected frame.
                              Update the frame table to indicate this frame is invalidated.
    Output/Result of function - Returns the frame number of the replaced frame.
*/
unsigned int mm_replace_page(main_memory_struct **main_memory, kernel_struct* kernel){

    frame_table_struct frame_table = (*main_memory)->frame_table;
    unsigned int lru_frame = 0;
    unsigned int min_counter = __INT32_MAX__;
    unsigned int pid, outer_pt_frame_no;
    logical_address_struct page_no;

    //Find the frame entry with the least value of counter.
    for(unsigned int frame=1; frame<NO_OF_FRAMES; frame++){
        if(min_counter > (frame_table.frame_entry[frame].counter & 0xFFFF)){
            min_counter = (frame_table.frame_entry[frame].counter & 0xFFFF);
            lru_frame = frame;
        }
    }

    pid = frame_table.frame_entry[lru_frame].pid;
    page_no = frame_table.frame_entry[lru_frame].page_no;

    total_access_time += mm_to_swap_space_write_time;

    // printf("| Replacing frame = %d |", lru_frame);

    //Check whether the frame we are replacing is dirty or not. If it is dirty, initiate write to disk and clear the dirty bit.
    if(mm_check_dirty_bit(lru_frame, (*main_memory)) == 1){

        total_access_time += mm_to_disk_write_time;

        mm_write_to_disk(lru_frame, main_memory);
        (*main_memory)->frame_table.frame_entry[lru_frame].modified = 0;
    }

    //Make that entry in the frame table as invalid and the frame it was pointing to as NULL.
    //The pid is also made as -1. It is used as a delimeter so that we know which entry to invalidate.
    (*main_memory)->frame_table.frame_entry[lru_frame].valid = 0;
    (*main_memory)->frame_table.frame_entry[lru_frame].pid = -1;

    //Check whether the lru frame contained the outer page table for some other process or not.
    //If it did, then update the pcb of the other process by setting the frame no of outer page table as -1 
    if(outer_pt_frame_no == lru_frame){
        kernel_invalidate_outer_page_table(pid, kernel);
        return lru_frame;
    }

    //Else we invalidate the entries in the page tables for the logical address stored in that frame. 
    mm_invalidate_page_table_entries(page_no, pid, (*main_memory), kernel);

    return lru_frame;
}

/*
    Input of function - Input is the logical address corresponding to the page, the pid of process and the kernel structure.
    Purpose of the function - Invalidate the page table entry pointing to the frame selected for replacement. 
                              As the pages are swapped out to the swap space, the pointer is not set to NULL, indicating that the page table exists, just not in main memory.
    Output/Result of function - Updated page table with the entry pointing to the replaced frame as invalid.
*/
void mm_invalidate_page_table_entries(logical_address_struct page_no, unsigned int pid, main_memory_struct *main_memory, kernel_struct *kernel){
    unsigned int outer_pt = page_no.outer_pt & 0x3f;
    unsigned int middle_pt = page_no.middle_pt & 0x1ff;
    unsigned int inner_pt = page_no.inner_pt & 0x1ff;

    if(kernel_check_valid_bit(pid, kernel) == 0){
        return;
    }

    page_info outer_page = kernel->CR3_reg;
    page_table_struct* outer_page_table = outer_page.page_pointer;

    page_info middle_page = outer_page_table->page_entry[outer_pt].page;

    unsigned int middle_frame_no = middle_page.frame_no;
    page_table_struct* middle_page_table = middle_page.page_pointer;


    //If we are replacing the middle page table, the pid for this frame will be -1 in the frame table and thus, we have to invalidate the entry in the outer page table only.
    if(main_memory->frame_table.frame_entry[middle_frame_no].pid == -1 || outer_page_table->page_entry[outer_pt].valid == 0){
        outer_page_table->page_entry[outer_pt].valid = 0;
        return;
    }

    page_info inner_page = middle_page_table->page_entry[middle_pt].page;
    unsigned int inner_frame_no = inner_page.frame_no;
    page_table_struct* inner_page_table = inner_page.page_pointer;

    //If we are replacing the inner page table, the pid for this frame will be -1 in the frame table and thus, we have to invalidate the entry in the middle page table only
    //Note that we will not invalidate the entry in the outer page table as the middle page table is not replaced, only the inner page table is replaced.
    if(main_memory->frame_table.frame_entry[inner_frame_no].pid == -1 || middle_page_table->page_entry[middle_pt].valid == 0){
        middle_page_table->page_entry[middle_pt].valid = 0;
        return;
    }

    //If we reach here, that means we are invalidating a frame and thus, we will invalidate the entry only in the inner page table.
    // page_table_struct *inner_page_table = main_memory->frame_table.frame_entry[inner_frame_no].page_pointer;
    inner_page_table->page_entry[inner_pt].valid = 0;

    return;
}

//Check if the frame we are replacing is dirty or not. This function is also needed while termination of process.
unsigned int mm_check_dirty_bit(unsigned int frame_no, main_memory_struct *main_memory){
    return ((main_memory->frame_table.frame_entry[frame_no].modified & 0x1) == 1);    

}

/*
    Input of function - Input is the pid of process, main memory structure and the kernel structure.
    Purpose of the function - Terminates the process by freeing all the frames held by the process. All the frames held by the process are invalidated in the main memory and are added to the free frames list.
                              If the dirty bit is set for the frame, it is first written to disk before freeing.
    Output/Result of function - Process is terminated and the frames owned by the process are invalidated. The process information from the kernel's pcb is also removed.

*/
void mm_terminate_process(unsigned int pid, main_memory_struct **main_memory, kernel_struct *kernel){
    free_frames_struct *frames_tail = (*main_memory)->free_frames_head;

    //Free all the frames owned by this process and add them to the free frames list.
    for(unsigned int i=0; i<NO_OF_FRAMES; i++){
        if((*main_memory)->frame_table.frame_entry[i].pid == pid){
            
            //Check whether the frame is dirty or not. If it is, initiate write to disk and clear the dirty bit.
            if(mm_check_dirty_bit(i, (*main_memory)) == 1){
                total_access_time += mm_to_disk_write_time;

                mm_write_to_disk(i, main_memory);
            }

            (*main_memory)->frame_table.frame_entry[i].valid = 0;
            (*main_memory)->frame_table.frame_entry[i].pid = -1;

            //Adding a free frame to the free frame list.
            mm_add_free_frame(main_memory, i);
        }
    }

    //Make the pid in the pcb as -1 to indicate this process has terminated.
    kernel_terminate_process(pid, kernel);

}

/*
    Input of function - Input is the frame number and the main memory.
    Purpose of the function - Writing the contents of the frame back to disk. Modified bit is cleared after writing.
    Output/Result of function - Modified bit cleared and the frame is written to disk.
*/
void mm_write_to_disk(unsigned int frame_no, main_memory_struct **main_memory){
    /*
        Function to write to secondary storage
        Input - Frame for writing to disk

    */
    (*main_memory)->frame_table.frame_entry[frame_no].modified = 0;
}

/*
    Input of function - Input is the physical address, the main memory structure. The data to be written is present on the variable  bus32B;
    Purpose of the function - Write data from the L2 cache to the frame in the main memory. The frame number and the offset in the frame is found using the physical address.
    Output/Result of function - Data is written to the frame and the modified bit of the frame is set to 1.
*/
void mm_write_to_mm(unsigned int physical_address, main_memory_struct **main_memory){
    unsigned int frame_no = physical_address>>10 & 0x3fffff;
    unsigned int offset = physical_address & 0x3ff;

    (*main_memory)->frame_table.frame_entry[frame_no].modified = 1;

    /*
        Write data in the frame using the frame no and the offset. Data is present on the bus32B.
    */

   return;

}

void mm_get_data_from_frame(int physical_address){
    int frame_no = (physical_address >> 10) & 0x3fffff;
    int offset = physical_address & 0x3ff;
    
    /*
        Get data from the frame and put it in the bus32B (the bus between main memory and L2 cache).
    */

}

void mm_clear_mm(main_memory_struct *main_memory){
    free_frames_struct* frames_tail = main_memory->free_frames_head;
    while(frames_tail!=NULL){
        free_frames_struct* temp = frames_tail;
        frames_tail = frames_tail->next;
        free(temp);
    }
    free(main_memory);
}

//Convert given logical address into offset, inner bits, middle bits and outer bits
logical_address_struct mm_convert(unsigned int x){
    logical_address_struct la;
    la.offset = x & 0x3ff;
    x = x>>10;
    la.inner_pt = x & 0x1ff;
    x = x>>9;
    la.middle_pt = x & 0x1ff;
    x = x>>9;
    la.outer_pt = x & 0xf;

    // printf("| Outer = %x, middle= %x, inner = %x, offset = %x | ", la.outer_pt & 0xf, la.middle_pt & 0x1ff, la.inner_pt & 0x1ff, la.offset & 0x3ff);

    return la;
}

//Convert the given logical address structure back to an integer.
unsigned int mm_convert_back(logical_address_struct la){
    unsigned int x = 0;
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