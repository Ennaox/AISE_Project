all: dbg crash 
	./dbg ./crash

compile: dbg

dbg: main.o pdwarf.h pdwarf.o main.h
	gcc pdwarf.o main.o -o dbg -g -Wall -Wextra -lunwind-ptrace -lunwind-generic -lunwind-x86_64 -lunwind

main.o: main.c main.h
	gcc -c main.c -o main.o -g

pdwarf.o: pdwarf.c pdwarf.h
	gcc -c pdwarf.c -o pdwarf.o -g

crash: crash.c
	gcc crash.c -o crash -g

clean:
	rm -f main crash dbg