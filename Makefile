all: main crash
	./main ./crash

main: main.c
	gcc main.c -o main -g -Wall -Wextra -lunwind-ptrace -lunwind-generic -lunwind-x86_64 -lunwind

crash: crash.c
	gcc crash.c -o crash -g

clean:
	rm -f main