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
        frame_table->frame_entry[i].valid = '0';
        frame_table->frame_entry[i].counter = 0;
        frame_table->frame_entry[i].pid = 0;
    }
}

int mm_new_page_table(page_table_struct *page_table, free_frames_struct **free_frame_list){
    int free_frame = mm_get_free_frame(free_frame_list);

    for(int i=0; i<NO_OF_FRAMES; i++){
        page_table->page_entry[i].valid = '0';
    }

    return free_frame;
}

page_table_struct* mm_initialize_page_table(int pid, free_frames_struct **free_frames_list, frame_table_struct *frame_table){
    page_table_struct *inner_page_table = malloc(sizeof(page_table_struct));
    int inner_page_table_frame = mm_new_page_table(inner_page_table, free_frames_list);
    frame_table->frame_entry[inner_page_table_frame].pid = pid;
    frame_table->frame_entry[inner_page_table_frame].valid = '1';

    page_table_struct *middle_page_table = malloc(sizeof(page_table_struct));
    int middle_page_table_frame = mm_new_page_table(middle_page_table, free_frames_list);
    frame_table->frame_entry[middle_page_table_frame].pid = pid;
    frame_table->frame_entry[middle_page_table_frame].valid = '1';

    middle_page_table->page_entry[0].frame_no = inner_page_table;
    middle_page_table->page_entry[0].valid = '1';
    middle_page_table->page_entry[0].modified_bit = '0';
    middle_page_table->page_entry[0].referenced_bit = '0';

    page_table_struct *outer_page_table = malloc(sizeof(page_table_struct));
    int outer_page_table_frame = mm_new_page_table(outer_page_table, free_frames_list);
    frame_table->frame_entry[outer_page_table_frame].pid = pid;
    frame_table->frame_entry[outer_page_table_frame].valid = '1';

    outer_page_table->page_entry[0].frame_no = middle_page_table;
    outer_page_table->page_entry[0].valid = '1';
    outer_page_table->page_entry[0].modified_bit = '0';
    outer_page_table->page_entry[0].referenced_bit = '0';

    return outer_page_table;
}

void mm_load_page(int pid, page_table_struct **page_table, frame_table_struct *frame_table, free_frames_struct **free_frame_list){
    int free_frame = mm_get_free_frame(free_frame_list);
    (frame_table)->frame_entry[free_frame].valid = '1';
    (frame_table)->frame_entry[free_frame].pid = pid;
    (frame_table)->frame_entry[free_frame].counter ;

    page_table_struct* middle_page_table = (*page_table)->page_entry[0].frame_no;
    page_table_struct* inner_page_table = middle_page_table->page_entry[0].frame_no;

    for(int i=0; i<NO_OF_FRAMES; i++){
        if((inner_page_table)->page_entry[i].valid = '0'){
            (inner_page_table)->page_entry[i].valid = '1';
            (inner_page_table)->page_entry[i].frame_no = (&free_frame);
            (inner_page_table)->page_entry[i].modified_bit = '0';
            break;
        }
    }
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
    //new_mm->frame_table = malloc(sizeof(frame_table_struct));
    new_mm->free_frames_head = NULL;
    

    mm_initialize_free_frames(&(new_mm->free_frames_head));
    mm_initialize_frame_table(&(new_mm->frame_table));
    return new_mm;
}

int main(){
    //Initialize main memory;
    main_memory_struct *main_memory = mm_initialize_mm();
    free_frames_struct* free_frames_list = main_memory->free_frames_head;
    frame_table_struct frame_table = main_memory->frame_table;

    //Process 1
    int pid = 400;
    page_table_struct* outer_page_table = mm_initialize_page_table(pid, (&free_frames_list), (&frame_table));
    mm_load_page(pid, (&outer_page_table), (&frame_table), (&free_frames_list));

    for(int i=0; i<10; i++){
        printf("i = %d, Valid = %c, pid = %d\n", i, frame_table.frame_entry[i].valid, frame_table.frame_entry[i].pid);
    }

}