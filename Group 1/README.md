# DSTN_Assignment1
DSTN Assignment 1 Group #1 &amp; #2

# GROUP MEMBERS:<br>
Kevin Boban - 2016A7PS0185G<br>
Mayur Arvind - 2016B1A70603G<br>
Amol Pai - 2017A7PS0038G<br>
Ritvik Agarwal - 2017A7PS0136G<br>

##CONTRIBUTION OF EACH MEMBER:<br>
Kevin Boban - Implementation of L2 cache and functions<br>
Mayur Arvind - Implementation of TLB and its functions<br>
Amol Pai - Implementation of main memory and kernel structures and functions and the driver cpu program<br>
Ritvik Agarwal - Implementation of L1 cache and functions<br>

## DESCRIPTION OF THE PROBLEM:<br>
TLB - Identifier based TLB. Invalidation occurs at termination of process.<br>
L1 cache - 4 KB, 16 B, 4 way set associative way prediction cache. Virtually indexed and physically tagged. Implementation is write through and look through. Replacement is LRU square matrix<br>
L2 cache - 32 KB, 32 B, 8 way set associative cache. Implementation is write back and look aside. Replacement is LRU counter.<br>
Main memory - 32 MB. Pure paging main memory. Replacement is LRU and global replacement.<br>

## TECHNICAL DETAILS ABOUT THE STRUCTURES AND ADDRESSES -
Virtual Address - 32 bits, Physical Address - 25 bits. One page size = 1 KB. 
TLB - Number of entries = 32. Page no = 22 bits, frame no = 15 bits.<br>
L1 cache - Number of entries = 64. Number of ways = 4. Tag = 15, Index = 6, Offset = 4.<br>
L2 cache - Number of entries = 128. Number of ways = 8. Tag = 13, Index = 7, Offset = 5.<br>
Main memory - Number of frames = 32768. Hierarchical paging with three level page tables. Number of entries in page table = 512. Size of one page table entry = 2B.<br>

## FILES IN THE DIRECTORY:<br>

### Header files : <br>

1. main_memory_structures.h : Contains the data structures required for main memory. <br>
   - Data structures included are : page_table, frame_table, free_frames_list, main_memory, pcb, kernel, logical_address <br>

2. tlb_structures.h : Contains data structures required for the TLB. tlb_entry defines a single entry in the TLB, whose fields are a valid bit, a pid identifier, page no, frame no and a counter for replacement. tlb_buffer describes the entire TLB, with 32 instances of tlb_entry.

3. l1_structures.h :

4. l2_structures.h :

5. global_variables.h : It contains the various global variables which are used in different programs. It contains pointers to tlb, l1, l2 and main memory. It also contains the buses between l1-l2 and l2-main memory. It also contains the hit and miss counts of tlb, l1, l2 and main memory.

6. functions.h : It contains the declaration of all the functions defined in different programs. These contain the functions of tlb, l1, l2, main memory and the kernel.

7. config.h : Contains the time taken for various transfers and overheads.

### C files : <br>

1. main_memory.c : Contains functions needed for working of main memory.<br>
2. l1.c : Contains functions needed for working of L1 cache.<br>
3. l2.c : Contains functions needed for working of L2 cache.<br>
4. tlb.c : Contains functions needed for working of TLB.<br>
5. kernel.c : Contains functions needed for kernel. Also contains functions required for simulation.<br>
6. cpu.c : The driver program which does the simulation. This program calls the functions defined in the above programs. <br>

### Input and output files :

1. input.txt : It contains the number of processes and the names of all processes.<br>
2. output.txt : It contains the output of the simulation. The details in the output file is given below.<br>
3. APSI.txt, CC1.txt, LI.txt, M88KSIM.txt, VORTEX.txt : The process files. These files contains the virtual addresses accessed by the process.<br>


## COMPILING THE PROGRAM<br>
Execute the makefile given to compile all .c files<br>
Command : make or make all<br><br>

## EXECUTING THE PROGRAM<br>
Command : ./test input.txt<br>

## STRUCTURE OF PROGRAM:<br>
		PROCESSOR
		   TLB
	L1 DATA		L1 INSTRUCTION
		 L2 CACHE
		MAIN MEMORY
	<br><br>
	The cpu program takes the inputs of the processes. Two pages of each process is prefetched in main memory. Then, the cpu calls a function in the kernel which executes this process's request.
	The flowchart of the execution is as follows :
	1. CPU searches the tlb for page no.
	2. If it is a tlb miss, then the processor moves to main memory to search for the frame number for this page number.
	3. In main memory, page tables is arranged as three level hierarchical paging. So, we have to search all page tables for the frame number. 
	4. If the frame number is found, then it is a page hit. The TLB is updated with this frame number and the instruction is restarted.
	5. If the frame number is not found, then it is a page fault. The page table is updated after bringing a page from disk to main memory and a context switch happens to another process.
	6. If it was a tlb hit, then we first search the L1 instruction or data cache depending on the type of instruction for the data. As L1 is look through, we will not search L2 unless L1 is a miss.
	7. If it is a L1 hit, then we directly give the data to the cpu and move on to the next instruction.
	8. If it is a L1 miss, then we search in L2 and main memory parallely as L2 is look aside. 
	9. If it is a L2 hit, then we update the L1 cache with the data from L2, give the data to CPU and move to next instruction.
	10. If it is a L2 miss, then we get the data from main memory, update the L2 cache, give the data to CPU and move on to next instruction.
	11. If it is a CPU write, then while writing from cpu to L1, corresponding write from L1 to L2 also happens as L1 is write through.
	12. While replacement in L2, if the dirty bit in the entry is set, then we write the block to main memory as L2 is write back.
	13. After all the instructions of a process have executed, the process is terminated. While termination, if the dirty bit of the frame is set, it is first written to disk and then removed. Corresponding entries of the process in the tlb are also invalidated.

## OUTPUT OF THE PROGRAM:<br>
The output of the program is stored in output.txt. In the start, it contains the various hit and miss rates for tlb, l1, l2 and main memory for each process. Then, it states the total time taken for the execution of all processes.
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
