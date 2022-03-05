/*
	C debuger

	List of command:
	'r' or 'run' to launch the sub process and start the tracing
	'b' or 'breakpoint' to set a breakpoint
	'bt' or 'backtrace' to show the bactrace of the program
	'reg' or 'register' to show the register of the program
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
parsed_str;

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

int run(int eargc,char ** eargv)
{
	printf("Calling run function with %d argument: ",eargc);
	for(int i=0;i<eargc;i++)
		printf("%s ",eargv[i]);
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

parsed_str parse_str(char *buff)
{
	parsed_str parsed;
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


void deallocate_parsed(parsed_str parsed)
{
	for(int i = 0; i<parsed.eargc+1;i++)
	{
		free(parsed.eargv[i]);
	}
	free(parsed.eargv);
}

int main(int argc, char *argv[])
{
	
	pid_t child = 0;
	
	if(child)
	{
		char isAttach = 1;

		if(argc < 2)
		{
			isAttach = 0;
			//Not yet implemented
			perror("WARNING: The debbuger isn't attach to any binary\n Please attach it to a binary with the command 'attach [MyExecutable] [List of argument]\n####Not yet implemented####\n'");
			exit(150);
		}
		char buff[BUFF_SIZE];
		memset(buff,0,BUFF_SIZE*sizeof(char));
		while(1)
		{
			fgets(buff,BUFF_SIZE,stdin);
			parsed_str parsed = parse_str(buff);

			if(!strcmp(parsed.eargv[0],"r") || !strcmp(parsed.eargv[0],"run"))
			{
				run(parsed.eargc-1,&parsed.eargv[1]);
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
			else if(!strcmp(parsed.eargv[0],"quit"))
			{
				break;
			}
			else
			{
				printf("Error: '%s' is an unknown command\n",parsed.eargv[0]);
			}
			deallocate_parsed(parsed);
			memset(buff,0,1000*sizeof(char));
		}
	}
	return 0;
}