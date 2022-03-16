/*
	C debuger

	List of command:
	'r' or 'run' to launch the sub process and start the tracing
	'st' or 'step' to launche the sub process in step mode and start the tracing
	'b' or 'breakpoint' to set a breakpoint
	'n' or 'next' in step mode to execute the next instruction 
	'bt' or 'backtrace' to show the bactrace of the program
	'reg' or 'register' to show the register of the program
	'attach to attach the debugger to the binary we need to debug'
	'quit' to quit the program
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <libunwind.h>
#include <libunwind-x86_64.h>
#include <libunwind-ptrace.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define BUFF_SIZE 256

typedef struct parsed_str
{
	int eargc;
	char ** eargv;
} 
arg_struct;
 
unw_addr_space_t as;
struct UPT_info *ui;
unw_cursor_t cursor;
unw_cursor_t BASE_cursor;
pid_t child = 0;

char isAttach = 1;
char isRunning = 0;
char isRunning_step = 0;


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

int attach_step(pid_t child)
{
    long verif = ptrace(PTRACE_ATTACH, child, NULL, NULL);
    if(verif==-1)
	{
		printf("Error on PTRACE_SEIZE\n");
		return 9;
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

	int init_state = unw_init_remote(&BASE_cursor,as,ui);
	cursor = BASE_cursor;
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

void get_reg()
{
	char ** name = malloc(16*sizeof(char*));
	for(int i = 0; i < 16; i++)
	{
		name[i] = malloc(3*sizeof(char));
	}

	name[0] = "rax";
	name[1] = "rdx";
	name[2] = "rcx";
	name[3] = "rbx";
	name[4] = "rsi";
	name[5] = "rdi";
	name[6] = "rbp";
	name[7] = "rsp";
	name[8] = "r8";
	name[9] = "r9";
	name[10] = "r10";
	name[11] = "r11";
	name[12] = "r12";
	name[13] = "r13";
	name[14] = "r14";
	name[15] = "r15";

	unw_word_t ip;
	for(int i = 0; i < 16; i++)
	{
		unw_get_reg(&cursor, i, &ip);
		printf("%s\t%lx\t%lu\n",name[i],ip,ip);
	}

	/*struct user_regs_struct regs;

    ptrace (PTRACE_GETREGS,0,NULL,&regs);

    printf( "rax = 0x%llx	%lld\n"
    		"rcx = 0x%llx	%lld\n"
    		"rdx = 0x%llx	%lld\n"
    		"rsi = 0x%llx	%lld\n"
    		"rdi = 0x%llx	%lld\n"
    		"rbp = 0x%llx	%lld\n"
    		"rbx = 0x%llx	%lld\n"
    		"r8 = 0x%llx	%lld\n"
    		"r9 = 0x%llx	%lld\n"
    		"r10 = 0x%llx	%lld\n"
    		"r11 = 0x%llx	%lld\n"
    		"r12 = 0x%llx	%lld\n"
    		"r13 = 0x%llx	%lld\n"
    		"r14 = 0x%llx	%lld\n"
    		"r15 = 0x%llx	%lld\n"
    		,regs.rax,regs.rax,regs.rcx,regs.rcx,regs.rdx,regs.rdx,regs.rsi,regs.rsi,regs.rdi,regs.rdi
    		,regs.rbp,regs.rbp,regs.rbx,regs.rbx,regs.r8,regs.r8,regs.r9,regs.r9,regs.r10,regs.r10
    		,regs.r11,regs.r11,regs.r12,regs.r12,regs.r13,regs.r13,regs.r14,regs.r14,regs.r15,regs.r15);
*/
}

void backtrace()
{
	int ret = 1;
	while (ret > 0) {

		unw_word_t offset, ip, sp;
		char sym[1024];

		sym[0] = '\0';

		// unw_get_reg(&cursor, UNW_REG_IP, &ip);
		// unw_get_reg(&cursor, UNW_REG_SP, &sp);
		// printf("%lx et %lx\n",ip, sp);

		unw_proc_info_t proc_info;
		unw_get_proc_name(&cursor, sym, sizeof(sym), &offset);
		unw_get_proc_info(&cursor, &proc_info);

		printf("%lx: (%s+0x%lx)\n",proc_info.start_ip ,sym, offset);
		ret = unw_step(&cursor);
	}	
	get_reg();
}

void end_backtrace()
{
	unw_destroy_addr_space(as);
	_UPT_destroy(ui);
}

int run(arg_struct arg)
{
	child = 0;
	isRunning = 1;
	child = fork();
	
	if(child)
	{
		char BUFF[BUFF_SIZE];
		realpath(arg.eargv[0],BUFF);
		printf("Starting program: %s\nProgram pid: %d\n\n", BUFF,child);

		int status = 0;
		if(status = attach(child))
		{
			kill(child,SIGINT);
			return status;
		}

  		wait(NULL);
  		printf("\n");
  		init_backtrace(child);

	    siginfo_t result;
    	ptrace(PTRACE_GETSIGINFO, child, 0, &result);
		printf("Program receiv the signal %d: %s\nError raised at 0x%lx\n",result.si_signo,strsignal(result.si_signo),result.si_addr);

    	printf("\n");
	}
	else
	{
		execvp(arg.eargv[0],arg.eargv+sizeof(char *));
		return 0;
	}
	return 0;
}

int backtrace_step(unw_cursor_t cursor)
{
		int val = 1;
		unw_word_t offset, ip, sp;
		char sym[1024];

		sym[0] = '\0';

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
		printf("%lx et %lx\n",ip, sp);

		unw_get_proc_name(&cursor, sym, sizeof(sym), &offset);
		printf("(%s+0x%lx)\n", sym, offset);
		val = unw_step(&cursor);
		return val;
}

int step(arg_struct arg)
{
	isRunning_step = 1;
	child = 0;

	child = fork();
	
	if(child)
	{
		int status = 0;
		if(status = attach_step(child))
		{
			kill(child,SIGINT);
			return status;
		}

  		wait(NULL);

  		init_backtrace(child);
	}
	else
	{
		execvp(arg.eargv[0],arg.eargv+sizeof(char *));
		return 0;
	}
	return 0;
}

int break_point(int eargc,char ** eargv)
{
	printf("Calling break function with %d argument: ",eargc);
	for(int i=0;i<eargc;i++)
		printf("%s ",eargv[i]);
	printf("\n");
}

int backtrace_fct(int eargc,char ** eargv)
{
	if(isRunning_step || isRunning)
	{
		cursor = BASE_cursor;
		backtrace(cursor);
		printf("\n");
	}
	else
	{
		printf("Error: can't backtrace: no program are running\n");
	}
}

int reg(int eargc,char ** eargv)
{
	get_reg();
	printf("\n");
}

int next(int eargc,char ** eargv)
{
	if(isRunning_step)
	{
		int retval = ptrace(PTRACE_SINGLESTEP, child, NULL,NULL);	
		printf("%d\n",retval);
		if(WIFSTOPPED(retval))
		{
	  		end_backtrace();

		    siginfo_t result;
	    	ptrace(PTRACE_GETSIGINFO, child, 0, &result);
			printf("Program receiv the signal %d: %s\nError raised at 0x%lx\n",result.si_signo,strsignal(result.si_signo),result.si_addr);
	    }
	}
	else
	{
		printf("Error: Not running in step by step\n");
	}
    printf("\n");
}

arg_struct attach_funct(int eargc, char ** eargv)
{
	arg_struct arg;
	arg.eargc = eargc;

	arg.eargv = malloc((arg.eargc+1)*sizeof(char*));
	for(int i = 0; i<arg.eargc;i++)
	{
		arg.eargv[i] = malloc(BUFF_SIZE*sizeof(char));
		strcpy(arg.eargv[i],eargv[i]);
	}
	arg.eargv[arg.eargc] = NULL;

	return arg;
}

arg_struct parse_str(char *buff)
{
	arg_struct parsed;
	parsed.eargc = 1;
	for(int i = 0; i<BUFF_SIZE;i++)
	{
		if(buff[i] == ' ')
		{
			parsed.eargc ++;
		}
		if(buff[i] == '\0')
			break;
		if(buff[i] == '\n')
		{
			buff[i] = '\0';
			break;
		}
	}
	parsed.eargv = malloc((parsed.eargc+1)*sizeof(char*));
	for(int i = 0; i<parsed.eargc;i++)
	{
		parsed.eargv[i] = malloc(BUFF_SIZE*sizeof(char));
		memset(parsed.eargv[i],0,BUFF_SIZE*sizeof(char));
	}
	parsed.eargv[parsed.eargc] = NULL;
	int len = strlen(buff);
	int j = 0, k=0;
	for(int i = 0;i<len;i++)
	{
		if(buff[i] == ' ')
		{
			j++;
			parsed.eargv[j][k] = '\0';
			k = 0;
		}
		else
		{
			parsed.eargv[j][k] = buff[i];
			k++;
		}
	}
	return parsed;
}


void deallocate_parsed(arg_struct parsed)
{
	for(int i = 0; i<parsed.eargc+1;i++)
	{
		free(parsed.eargv[i]);
	}
	free(parsed.eargv);
}

void end_process()
{
	int status = 0;
	isRunning_step = 0;
	isRunning = 0;
	end_backtrace();
	status = detach(child);
	if(status)
	{
		kill(child,SIGINT);
		exit(status);
	}
}

int main(int argc, char *argv[])
{
	printf("################################### C debuger ###################################\n\n"
		
		"Made by DIAS Nicolas and LAPLANCHE Alexis\n\n"

		"List of command:\n"
		"\t-'r' or 'run' to launch the sub process and start the tracing\n"
		"\t-'st' or 'step' to launch the sub process in step mode and start the tracing\n"
		"\t-'b' or 'breakpoint' to set a breakpoint\n"
		"\t-'n' or 'next' in step mode to execute the next instruction\n"
		"\t-'bt' or 'backtrace' to show the bactrace of the program\n"
		"\t-'reg' or 'register' to show the register of the program\n"
		"\t-'attach' to attach the debugger to the binary we need to debug\n"
		"\t-'quit' to quit the program\n\n"
		
		"#################################################################################\n\n");
	
	arg_struct arg;
	
	if(argc < 2)
	{
		isAttach = 0;
		printf("WARNING: The debbuger isn't attach to any binary\n\tPlease attach it to a binary with the command 'attach [MyExecutable] [List of argument]'\n");
	}
	else
	{
		arg.eargc = argc - 1;
		arg.eargv = malloc((arg.eargc+1)*sizeof(char*));
		for(int i = 0; i<arg.eargc;i++)
		{
			arg.eargv[i] = malloc(BUFF_SIZE*sizeof(char));
			strcpy(arg.eargv[i],argv[i+1]);
		}
		arg.eargv[arg.eargc] = NULL;
	}
	char buff[BUFF_SIZE];
	memset(buff,0,BUFF_SIZE*sizeof(char));
	
	while(1)
	{
		fgets(buff,BUFF_SIZE,stdin);
		arg_struct parsed = parse_str(buff);

		if(!strcmp(parsed.eargv[0],"r") || !strcmp(parsed.eargv[0],"run"))
		{
			if(isRunning || isRunning_step)
			{
				end_process();
			}
			if(isAttach)
			{
				run(arg);
			}
			else
			{
				printf("Can't run: The debuger isn't attach to a binary\n");
			}
		}
		else if(!strcmp(parsed.eargv[0],"b") || !strcmp(parsed.eargv[0],"breakpoint"))
		{
			break_point(parsed.eargc-1,&parsed.eargv[1]);
		}
		else if(!strcmp(parsed.eargv[0],"bt") || !strcmp(parsed.eargv[0],"backtrace"))
		{
			backtrace_fct(parsed.eargc-1,&parsed.eargv[1]);
		}
		else if(!strcmp(parsed.eargv[0],"reg") || !strcmp(parsed.eargv[0],"register"))
		{
			reg(parsed.eargc-1,&parsed.eargv[1]);
		}
		else if(!strcmp(parsed.eargv[0],"n") || !strcmp(parsed.eargv[0],"next"))
		{
			next(parsed.eargc-1,&parsed.eargv[1]);
		}
		else if(!strcmp(parsed.eargv[0],"st") || !strcmp(parsed.eargv[0],"step"))
		{
			if(isRunning || isRunning_step)
			{
				end_process();
			}
			step(arg);
		}
		else if(!strcmp(parsed.eargv[0],"attach"))
		{
			arg = attach_funct(parsed.eargc-1,&parsed.eargv[1]);
			isAttach = 1;
		}
		else if(!strcmp(parsed.eargv[0],"quit"))
		{
			if(isRunning || isRunning_step)
			{
				end_process();
			}
			deallocate_parsed(arg);
			deallocate_parsed(parsed);
			break;
		}
		else
		{
			printf("Error: '%s' is an unknown command\n",parsed.eargv[0]);
		}
		deallocate_parsed(parsed);
		memset(buff,0,BUFF_SIZE*sizeof(char));
	}
	return 0;
}