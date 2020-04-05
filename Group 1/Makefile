test: main_memory.o cpu.o tlb.o l1.o l2_cache.o kernel.o
	gcc -o test cpu.o main_memory.o tlb.o l1.o l2_cache.o kernel.o
	@echo "Execute using ./test input.txt"

kernel.o: kernel.c
	gcc -c kernel.c

cpu.o: cpu.c
	gcc -c cpu.c
	
tlb.o: tlb.c
	gcc -c tlb.c
	
l1.o: l1.c
	gcc -c l1.c

l2_cache.o: l2_cache.c
	gcc -c l2_cache.c

main_memory.o: main_memory.c
	gcc -c main_memory.c

clean:
	rm -f *.o test
