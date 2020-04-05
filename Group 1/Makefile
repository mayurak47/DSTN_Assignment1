flags = -c -Wall

test: main_memory.o cpu.o tlb.o l1.o l2_cache.o kernel.o
	gcc -o test cpu.o main_memory.o tlb.o l1.o l2_cache.o kernel.o
	@echo "Execute using ./test input.txt"

kernel.o: kernel.c
	gcc $(flags) kernel.c

cpu.o: cpu.c
	gcc $(flags) cpu.c
	
tlb.o: tlb.c
	gcc $(flags) tlb.c
	
l1.o: l1.c
	gcc $(flags) l1.c

l2_cache.o: l2_cache.c
	gcc $(flags) l2_cache.c

main_memory.o: main_memory.c
	gcc $(flags) main_memory.c

clean:
	rm -f *.o test
