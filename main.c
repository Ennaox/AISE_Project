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
unw_cursor_t cursor;

int attach(pid_t child)
{
	ui = _UPT_create(child);
	if (!ui) 
	{
       	printf("_UPT_create failed\n");
       	return 5;
   	}
	int init_state = unw_init_remote(&cursor,as,&child);

	if (init_state != 0) 
	{
        _UPT_destroy(ui);
        if (-init_state == UNW_EINVAL) 
        {
            printf("unw_init_remote: UNW_EINVAL\n");
            return 1;
        } 
        else if (-init_state == UNW_EUNSPEC) 
        {
            printf("unw_init_remote: UNW_EUNSPEC\n");
            return 2;
        } 
        else if (-init_state == UNW_EBADREG) 
        {
            printf("unw_init_remote: UNW_EBADREG\n");
            return 3;
        } 
        else 
        {
	        printf("unw_init_remote: UNKNOWN\n");
	        return 4;
	    }
    }

    return 0;
}

int detach(pid_t child)
{
	long status = ptrace(PTRACE_DETACH, child, 0, 0);
	if(status==-1)
	{
		printf("Error on PTRACE_DETACH\n");
		return 6;
	}

	_UPT_destroy(ui);
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		perror("USAGE: Need argument\n");
		exit(120);
	}
	
	pid_t child = 0;

	as = unw_create_addr_space(&_UPT_accessors,0);

	child = fork();
	if(child)
	{
		sleep(1);
		int status = 0;
		if(status = attach(child))
		{
			kill(child,SIGINT);
			return status;
		}

		unw_context_t context;
  		char buf[1024];
  		unw_getcontext(&context);
  		unw_init_local(&cursor, &context);

  		sleep(1);

		while(unw_step(&cursor) > 0)
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

	    int result = 0;
    	ptrace(PTRACE_GETSIGINFO, child, 0, &result);
		printf("erreur = %d\n",result);

    	status = detach(child);
    	if(status)
    	{
    		kill(child,SIGINT);
    		return status;
    	}

    	int err = waitpid(child,NULL,0);
		
	}
	else
	{
		ptrace(PTRACE_TRACEME,0,NULL,NULL);
		execvp(argv[1],argv+sizeof(char *));
		return 0;
	}
	return 0;
}