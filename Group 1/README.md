# DSTN_Assignment1
DSTN Assignment 1 Group #1 &amp; #2

Group Members:<br>
Kevin Boban<br>
Mayur Arvind<br>
Amol Pai<br>
Ritvik Agarwal<br>

## DESCRIPTION OF THE PROBLEM:<br>
TLB - Identifier based TLB. Invalidation occurs at termination of process.
L1 cache - 4 KB, 16 B, 4 way set associative way prediction cache. Virtually indexed and physically tagged. Implementation is write through and look through. Replacement is LRU square matrix<br>
L2 cache - 32 KB, 32 B, 8 way set associative cache. Implementation is write back and look aside. Replacement is LRU counter.<r>
Main memory - 32 MB. Pure paging main memory. Replacement is LRU and global replacement.<br>

## TECHNICAL DETAILS ABOUT THE STRUCTURES - 
TLB - Number of entries = 32. Page no = 22 bits, frame no = 15 bits.<br>
L1 cache - Number of entries = 64. Number of ways = 4. Tag = 15, Index = 6, Offset = 4.<br>
L2 cache - Number of entries = 128. Number of ways = 8. Tag = 13, Index = 7, Offset = 5.<br>
Main memory - Number of frames = 32768. Size of page = 1KB. Hierarchical paging with three level page tables. Number of entries in page table = 512. Size of one page table entry = 2B.<br>

## FILES IN THE DIRECTORY:<br>
1. main_memory_structures.h : Contains the data structures required for main memory. <br>
   - Data structures included are : page_table, frame_table, free_frames_list, main_memory, pcb, kernel, logical_address <br>

2. tlb_structures.h : Contains data structures required for the TLB. tlb_entry defines a single entry in the TLB, whose fields are a valid bit, a pid identifier, page no, frame no and a counter for replacement. tlb_buffer describes the entire TLB, with 32 instances of tlb_entry.

3. l1_structures.h :

4. l2_structures.h :

5. global_variables.h : It contains the various global variables which are used in different programs. It contains pointers to tlb, l1, l2 and main memory. It also contains the buses between l1-l2 and l2-main memory. It also contains the hit and miss counts of tlb, l1, l2 and main memory.

6. functions.h : It contains the declaration of all the functions defined in different programs. These contain the functions of tlb, l1, l2, main memory and the kernel.


## COMPILING THE PROGRAM<br>
Execute the makefile given to compile all .c files<br>
Command : make<br><br>

## EXECUTING THE PROGRAM<br>
Command : ./test input.txt<br>

## OUTPUT OF THE PROGRAM:<br>
The output of the program is stored in output.txt. In the start, it contains the various hit and miss rates for tlb, l1, l2 and main memory for each process. Then it states the total time taken for the execution of all processes and the effective main memory access time.
<br>
After that, it displays various fields in each line. The fields are:<br>
instruction count of executing process.<br>
Whether the instruction is an instruction or data.<br>
Pid of executing process<br>
The logical address being executed<br>
Whether it is a tlb miss, tlb hit, l1 miss, l1 hit, l2 miss, l2 hit, page hit or page fault.<br>
Whether the cpu is writing to l1 or not.<br>
<br>
The output file also contains a line when the process gets terminated.
