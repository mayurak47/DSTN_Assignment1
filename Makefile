test: main_memory.o cpu.o
	gcc -o test cpu.o main_memory.o

main_memory.o: main_memory.c
	gcc -c main_memory.c

cpu.o: cpu.c
	gcc -c cpu.c

clean:
	rm -f *.o test