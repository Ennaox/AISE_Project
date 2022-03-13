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

struct UPT_info *ui;
unw_addr_space_t as;
unw_cursor_t cursor;

int attach(pid_t child)
{
    long verif = ptrace(PTRACE_SEIZE, child, NULL, NULL);
    if(verif==-1)
	{
		printf("Error on PTRACE_SEIZE\n");
		return 9;
	}	

	ptrace(PTRACE_INTERRUPT, child, NULL,NULL);

   	wait(NULL);

    ptrace(PTRACE_CONT, child, NULL,NULL);

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

	return 0;
}

int init_backtrace(pid_t child)
{
	as = unw_create_addr_space(&_UPT_accessors,0);
	
	if (!as) {
        printf("unw_create_addr_space failed\n");
        return 10;
    }

    ui = _UPT_create(child);
	if (!ui) 
	{
		kill(child,SIGINT);
       	printf("_UPT_create failed\n");
       	return 5;
   	}

	int init_state = unw_init_remote(&cursor,as,ui);

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

void backtrace()
{
	int ret = 1;
	while (ret > 0) {

		unw_word_t offset, ip, sp;
		char sym[1024];

		sym[0] = '\0';

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
		printf("%lx et %lx\n",ip, sp);

		unw_get_proc_name(&cursor, sym, sizeof(sym), &offset);
		printf("(%s+0x%lx)\n", sym, offset);
		ret = unw_step(&cursor);
	}	
}

void backtrace_step(unw_cursor_t cursor)
{
		unw_word_t offset, ip, sp;
		char sym[1024];

		sym[0] = '\0';

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
		printf("%lx et %lx\n",ip, sp);

		unw_get_proc_name(&cursor, sym, sizeof(sym), &offset);
		printf("(%s+0x%lx)\n", sym, offset);
		//unw_step(&cursor);
}

void end_backtrace()
{
	unw_destroy_addr_space(as);
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

	child = fork();
	
	if(child)
	{
		int status = 0;
		if(status = attach(child))
		{
			kill(child,SIGINT);
			return status;
		}

  		wait(NULL);

  		init_backtrace(child);

  		backtrace();

  		end_backtrace();

	    siginfo_t result;
    	ptrace(PTRACE_GETSIGINFO, child, 0, &result);
		printf("erreur = %d\n",result.si_signo);

    	status = detach(child);
    	if(status)
    	{
    		kill(child,SIGINT);
    		return status;
    	}
	}
	else
	{
		execvp(argv[1],argv+sizeof(char *));
		return 0;
	}
	return 0;
}