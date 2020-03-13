#include "main_memory.h"

void mm_initialize_all_frames(all_frames_struct *all_frames, free_frames_struct **free_frames_head){
    free_frames_struct *free_frames_tail = (*free_frames_head);

    for(int i=0; i<NO_OF_FRAMES; i++){
        for(int j=0; j<PAGE_SIZE; j++){
            (*all_frames).frames[i].bytes[j] = '\0';
        }

        free_frames_struct *new_free_frame = malloc(sizeof(free_frames_struct));
        new_free_frame->frame_no = i;
        new_free_frame->next = NULL;

        if((*free_frames_head) == NULL){
            (*free_frames_head) = new_free_frame;
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

void mm_initialize_page_table(page_table_struct *page_table){
    for(int i=0; i<NO_OF_FRAMES; i++){
        page_table->page_entry[i].valid = '0';
    }
}

void mm_load_page(int pid, page_table_struct *page_table, frame_table_struct *frame_table, free_frames_struct **free_frame_list){
    int free_frame = mm_get_free_frame(free_frame_list);
    frame_table->frame_entry[free_frame].valid = '1';
    frame_table->frame_entry[free_frame].pid = pid;
    

    for(int i=0; i<NO_OF_FRAMES; i++){
        if(page_table->page_entry[i].valid = '0'){
            page_table->page_entry[i].valid = '1';
            page_table->page_entry[i].frame_no = free_frame;
            page_table->page_entry[i].modified_bit = '0';
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