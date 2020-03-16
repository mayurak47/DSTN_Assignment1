#include "main_memory.h"

void mm_initialize_free_frames(free_frames_struct **free_frames_head){
    free_frames_struct *free_frames_tail = (*free_frames_head);

    for(int i=3; i<NO_OF_FRAMES; i++){
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

void mm_initialize_frame_table(frame_table_struct *frame_table){
    for(int i=0; i<NO_OF_FRAMES; i++){
        frame_table->frame_entry[i].valid = 0;
        frame_table->frame_entry[i].counter = 0;
        frame_table->frame_entry[i].pid = 0;
    }
}

page_table_struct* mm_new_page_table(int pid, main_memory_struct** main_memory){
    page_table_struct *page_table = malloc(sizeof(page_table_struct));
    int free_frame = mm_get_free_frame((&(*main_memory)->free_frames_head));
    (*main_memory)->frame_table.frame_entry[free_frame].pid = pid;
    (*main_memory)->frame_table.frame_entry[free_frame].valid = 1;

    for(int i=0; i<NO_OF_FRAMES; i++){
        page_table->page_entry[i].valid = 0;
        page_table->page_entry[i].frame_no = NULL;
    }

    return page_table;
}

page_table_struct* mm_initialize_page_table(int pid, main_memory_struct **main_memory){

    page_table_struct* inner_page_table = mm_new_page_table(pid, main_memory);

    page_table_struct* middle_page_table = mm_new_page_table(pid, main_memory);
    middle_page_table->page_entry[0].frame_no = inner_page_table;
    middle_page_table->page_entry[0].valid = 1;
    middle_page_table->page_entry[0].modified_bit = 1;
    middle_page_table->page_entry[0].referenced_bit = 1;

    page_table_struct* outer_page_table = mm_new_page_table(pid, main_memory);
    outer_page_table->page_entry[0].frame_no = middle_page_table;
    outer_page_table->page_entry[0].valid = 1;
    outer_page_table->page_entry[0].modified_bit = 1;
    outer_page_table->page_entry[0].referenced_bit = 1;

    return outer_page_table;
}

void mm_load_page(int pid, logical_address la, page_table_struct **page_table, main_memory_struct **main_memory){
    int free_frame = mm_get_free_frame(&((*main_memory)->free_frames_head));
    int *free_frame_no = malloc(sizeof(int));
    (*free_frame_no) = free_frame;

    (*main_memory)->frame_table.frame_entry[free_frame].valid = 1;
    (*main_memory)->frame_table.frame_entry[free_frame].pid = pid;
    (*main_memory)->frame_table.frame_entry[free_frame].counter ;

    page_table_struct* middle_page_table = (*page_table)->page_entry[la.outer_pt & 0x3].frame_no;
    page_table_struct* inner_page_table = middle_page_table->page_entry[la.middle_pt & 0x3ff].frame_no;

    int inner_pt = la.inner_pt & 0x3ff;
    inner_page_table->page_entry[inner_pt].valid = 1;
    inner_page_table->page_entry[inner_pt].frame_no = free_frame_no;
    inner_page_table->page_entry[inner_pt].modified_bit = 0;
    inner_page_table->page_entry[inner_pt].referenced_bit = 0;

}

int mm_get_free_frame(free_frames_struct **free_frame_head){
    if((*free_frame_head) == NULL){
        return -1;
    }

    int free_frame_no = (*free_frame_head)->frame_no;
    (*free_frame_head) = (*free_frame_head)->next;
    return free_frame_no;

}

main_memory_struct* mm_initialize_mm(){
    main_memory_struct* new_mm = malloc(sizeof(main_memory_struct));
    new_mm->free_frames_head = NULL;

    mm_initialize_free_frames(&(new_mm->free_frames_head));
    mm_initialize_frame_table(&(new_mm->frame_table));
    return new_mm;
}

logical_address mm_convert(int x){
    logical_address la;
    la.offset = x & 0x3ff;
    x = x>>10;
    la.inner_pt = x & 0x3ff;
    x = x>>10;
    la.middle_pt = x & 0x3ff;
    x = x>>10;
    la.outer_pt = x & 0x3;
    return la;
}

int mm_search_page_table(logical_address la, int pid, page_table_struct **outer_page_table, main_memory_struct **main_memory){
    int outer_offset = la.outer_pt & 0x3;
    if((*outer_page_table)->page_entry[outer_offset].valid == 0){
        printf("Page fault outer\n");
        page_table_struct *new_middle_page_table = mm_new_page_table(pid, main_memory);
        (*outer_page_table)->page_entry[outer_offset].valid = 1;
        (*outer_page_table)->page_entry[outer_offset].frame_no = new_middle_page_table;
        (*outer_page_table)->page_entry[outer_offset].modified_bit = 0;
        (*outer_page_table)->page_entry[outer_offset].referenced_bit = 0;
        return -1;
    }
    else{
        int middle_offset = la.middle_pt & 0x3ff;
        page_table_struct *middle_page_table = (*outer_page_table)->page_entry[outer_offset].frame_no;
        if(middle_page_table->page_entry[middle_offset].valid == 0){
            printf("Page fault middle\n");
            page_table_struct *new_inner_page_table = mm_new_page_table(pid, main_memory);
            (middle_page_table)->page_entry[middle_offset].valid = 1;
            (middle_page_table)->page_entry[middle_offset].frame_no = new_inner_page_table;
            (middle_page_table)->page_entry[middle_offset].modified_bit = 0;
            (middle_page_table)->page_entry[middle_offset].referenced_bit = 0;
            return -1;
        }
        else{
            int inner_offset = la.inner_pt & 0x3ff;
            page_table_struct *inner_page_table = middle_page_table->page_entry[middle_offset].frame_no;
            if(inner_page_table->page_entry[inner_offset].valid == 0){
                printf("Page fault inner\n");
                mm_load_page(pid, la, outer_page_table, main_memory);
                return -1;
            }
            else
            {
                printf("Page hit\n");
                int* frame_no = inner_page_table->page_entry[inner_offset].frame_no;
                return (*frame_no);
            }
        }
    }
}


int main(){
    //Initialize main memory;
    main_memory_struct *main_memory = mm_initialize_mm();
    FILE *fp;
    fp = fopen("APSI.txt", "r");

    //Process 1
    int pid = 400;
    page_table_struct* outer_page_table = mm_initialize_page_table(pid, &main_memory);

    for(int i=0; i<100; i++){
        int n; 
        fscanf(fp, "%x", &n);
        logical_address la = mm_convert(n);
        int frame_no;
        while((frame_no = mm_search_page_table(la, pid, &outer_page_table, &main_memory)) == -1);
        printf("Frame no = %d\n", frame_no);
    }

}