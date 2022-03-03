/*
	C debuger

	List of command:
	'r' or 'run' to launch the sub process and start the tracing
	'b' or 'breakpoint' to set a breakpoint
	'bt' or 'backtrace' to show the bactrace of the program
	'reg' or 'register' to show the register of the program
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

int run()
{
	printf("Calling run function");
}

parsed_str parse_str(char *buff)
{
	parsed_str parsed;
	parsed.eargc = 1;
	printf("%s",buff);
	for(int i = 0; i<BUFF_SIZE;i++)
	{
		if(buff[i] == ' ')
		{
			parsed.eargc ++;
		}
		if(buff[i] == '\0')
			break;
	}
	printf("%d\n",parsed.eargc);
	
	parsed.eargv = malloc(parsed.eargc*sizeof(char*));
	for(int i = 0; i<parsed.eargc;i++)
	{
		parsed.eargv[i] = malloc(BUFF_SIZE*sizeof(char));
	}
	
	strcpy(strtok(buff," "),parsed.eargv[0]);
	for(int i = 1; i<parsed.eargc; i++)
	{
		strcpy(strtok(NULL," "),parsed.eargv[i]);
		printf("%s\n",parsed.eargv[i]);
	}
	return parsed;
}

int main(int argc, char *argv[])
{
	as = unw_create_addr_space(&_UPT_accessors,0);
	if (!as) {
        printf("unw_create_addr_space failed\n");
        return 10;
    }
	
	pid_t child = 0;

	child = 1;
	//child = fork();
	
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
		char * eargv;
		char * eargc;
		while(1)
		{
			fgets(buff,BUFF_SIZE,stdin);
			parsed_str parsed = parse_str(buff);
			if(!strcmp(buff,"r") || !strcmp(buff,"run"))
			{
				run();
			}
			else if(!strcmp(buff,"b") || !strcmp(buff,"breakpoint"))
			{
				
			}
		/* int status = 0;
		if(status = attach(child))
		{
			kill(child,SIGINT);
			return status;
		}

		unw_context_t context;
  		char buf[1024];
  		unw_getcontext(&context);
  		//unw_init_local(&cursor, &context);
  		unw_init_remote(&cursor,as,ui);

  		while (unw_step(&cursor) > 0) {
  			printf("jecherche\n");

			unw_word_t offset, pc;
			char sym[64];

			if (unw_get_reg(&cursor, UNW_REG_IP, &pc))
			{
				printf("ERROR: cannot read program counter\n");
			}

			printf("0x%lx: ", pc);

			sym[0] = '\0';

			(void) unw_get_proc_name(&cursor, sym, sizeof(sym), &offset);
			printf("(%s+0x%lx)\n", sym, offset);
		}

  		wait(NULL);

	    int result = 0;
    	ptrace(PTRACE_GETSIGINFO, child, 0, &result);
		printf("erreur = %d\n",result);

    	status = detach(child);
    	if(status)
    	{
    		kill(child,SIGINT);
    		return status;
    	}*/
			memset(buff,0,1000*sizeof(char));
		}
	}
	else
	{
		execvp(argv[1],argv+sizeof(char *));
		return 0;
	}
	return 0;
}