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
#include <libunwind.h>
#include <libunwind-x86_64.h>
#include <libunwind-ptrace.h>
#include <string.h>
#include <time.h>

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

int attach(pid_t child)
{
	ui = _UPT_create(child);
	if (!ui) 
	{
       	printf("_UPT_create failed\n");
       	return 5;
   	}

    long verif = ptrace(PTRACE_SEIZE, child, NULL, NULL);
    if(verif==-1)
	{
		printf("Error on PTRACE_SEIZE\n");
		return 9;
	}	

	ptrace(PTRACE_INTERRUPT, child, NULL,NULL);

   	wait(NULL);
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

	_UPT_destroy(ui);
}

int run(arg_struct arg)
{
	printf("Calling run function with %d argument: ",arg.eargc);
	for(int i=0;i<arg.eargc;i++)
		printf("%s ",arg.eargv[i]);
	printf("\n");
}

int break_point(int eargc,char ** eargv)
{
	printf("Calling break function with %d argument: ",eargc);
	for(int i=0;i<eargc;i++)
		printf("%s ",eargv[i]);
	printf("\n");
}

int backtrace(int eargc,char ** eargv)
{
	printf("Calling backtrace function with %d argument: ",eargc);
	for(int i=0;i<eargc;i++)
		printf("%s ",eargv[i]);
	printf("\n");
}

int reg(int eargc,char ** eargv)
{
	printf("Calling register function with %d argument: ",eargc);
	for(int i=0;i<eargc;i++)
		printf("%s ",eargv[i]);
	printf("\n");
}

int step(int eargc,char ** eargv)
{
	printf("Calling step function with %d argument: ",eargc);
	for(int i=0;i<eargc;i++)
		printf("%s ",eargv[i]);
	printf("\n");
}

int next(int eargc,char ** eargv)
{
	printf("Calling next function with %d argument: ",eargc);
	for(int i=0;i<eargc;i++)
		printf("%s ",eargv[i]);
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
	
	pid_t child = 1;
	arg_struct arg;
	char isAttach = 1;

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
			backtrace(parsed.eargc-1,&parsed.eargv[1]);
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
			step(parsed.eargc-1,&parsed.eargv[1]);
		}
		else if(!strcmp(parsed.eargv[0],"attach"))
		{
			arg = attach_funct(parsed.eargc-1,&parsed.eargv[1]);
			isAttach = 1;
		}
		else if(!strcmp(parsed.eargv[0],"quit"))
		{
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