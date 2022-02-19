all: main crash
	./main ./crash

main: main.c
	gcc main.c -o main -g -lunwind-ptrace -lunwind-generic -lunwind-x86_64

crash: crash.c
	gcc crash.c -o crash -g

clear:
	rm -f main