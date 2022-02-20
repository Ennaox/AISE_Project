#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <libunwind.h>
#include <libunwind-x86_64.h>
#include <libunwind-ptrace.h>
#include <string.h>
#include <time.h>

unw_addr_space_t as;
struct UPT_info *ui;
int cdt = 1;

void on_segfault(int sig)
{
	printf("Erreur de segmation c'est relou\n");
	exit(1);
}

void child_ready(int sig)
{
	cdt = 0;
}

void backtrace(pid_t child)
{
	ui = _UPT_create(child);
	if (!ui) 
	{
       	printf("_UPT_create failed\n");
   	}
	ptrace(PTRACE_ATTACH, child, 0, 0);
	unw_cursor_t cursor;

	int remote_cursor = unw_init_remote(&cursor, as, ui);
	 if (remote_cursor != 0) {
        if (remote_cursor == UNW_EINVAL) {
            printf("unw_init_remote: UNW_EINVAL\n");
        } else if (remote_cursor == UNW_EUNSPEC) {
            printf("unw_init_remote: UNW_EUNSPEC\n");
        } else if (remote_cursor == UNW_EBADREG) {
            printf("unw_init_remote: UNW_EBADREG\n");
        } else {
            printf("unw_init_remote: UNKNOWN\n");
        }
    }
    wait(NULL);

	while (unw_step(&cursor) > 0)
	{
		unw_word_t offset, reg;
       	char func_name[256];
       	memset(func_name,'\0',256*sizeof(char));
       	
       	unw_get_reg(&cursor, UNW_REG_IP, &reg);
       	unw_get_proc_name(&cursor, func_name, 256*sizeof(char), &offset);
		printf("%p : (%s+0x%x) [%p]\n", (void *)reg,
                                          func_name,
                                          (int) offset,
                                          (void *) reg);
	}

	ptrace(PTRACE_DETACH, child, 0, 0);
	_UPT_destroy(ui);
}

int main(int argc,char **argv)
{
	signal(SIGSEGV,on_segfault);
	if(argc<2)
	{
		printf("Need argument\n");
		exit(2);
	}
	as = unw_create_addr_space(&_UPT_accessors, 0);
	pid_t child = 0;
	child = fork();
	
	if(child)
	{
		printf("j'attend %u\n",getpid());
		raise(SIGSTOP);
		printf("j'attend plus\n");
		backtrace(child);
		wait(NULL);
	}
	else
	{	
		kill(SIGCONT,getppid());
		printf("Papa attend plus %u\n",getppid());
		execvp(argv[1],argv+sizeof(char*));
	}
	return 0;
}