/*
	C debuger

	List of command:
	'r' or 'run' to launch the sub process and start the tracing
	'b' or 'breakpoint' to set a breakpoint
	'p' or 'prev' to unwind stack frame 
	'bt' or 'backtrace' to show the bactrace of the program
	'reg' or 'register' to show the register of the program
	'attach to attach the debugger to the binary we need to debug'
	'reset' to reset the cursor to the top of the stack
	'clear' to cleat the terminal
	'quit' to quit the program
*/

#include <sys/ioctl.h>
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

//structure nous permettant de stocker les arguments et le programme a debugger
typedef struct parsed_str
{
	int eargc;
	char ** eargv;
} 
arg_struct;

//variable global necessaire a l'utilisation de libunwind et ptrace 
unw_addr_space_t as;
struct UPT_info *ui;
unw_cursor_t cursor;
unw_cursor_t BASE_cursor;

//valeur du pid du processus fils
pid_t child = 0;


//variable nous permettant de vérifier si le debugger est attaché au programme
char isAttach = 1;
//variable nous permettant de vérifier si le programme fonctionne
char isRunning = 0;

//fonction qui permet au debugger de s'attacher au programme a vérifier
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

//fonction qui permet de se détacher du programme une fois que l'on a terminé
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

//fonction qui initialise tous les pointeurs et variables utilise par libunwind
//pour faire un backtrace
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

//fonction nous permettant de récupérer la valeur de tout les registres indiqué
//dans la fonction
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
}

//fonction qui lance le backtrace (regarder dans la pile les fonctions qui s'y trouvent)
//cette fonction récupère toutes les fonctions de la pile
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
	cursor = BASE_cursor;	
}

//fonction qui permet de libérer les pointeurs utilisé par libunwind
void end_backtrace()
{
	unw_destroy_addr_space(as);
	_UPT_destroy(ui);
}

//fonction qui nous permet de lancé le programme a débugger dans un processus fils,
//qui s'attache a ce dernier et récupère le signal retourner
int run(arg_struct arg)
{
	printf("\n");
	if(child)
	{
		kill(child,SIGKILL);
	}
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
	

int break_point(int eargc,char ** eargv)
{
	printf("\n");
	printf("Calling break function with %d argument: ",eargc);
	for(int i=0;i<eargc;i++)
		printf("%s ",eargv[i]);
	printf("\n\n");
}

//fonction qui vérifie si le programme fils fonctionne avant de lancé le backtrace
int backtrace_fct(int eargc,char ** eargv)
{
	if(isRunning)
	{
		backtrace(cursor);
		printf("\n");
	}
	else
	{
		printf("Error: can't backtrace: no program are running\n");
	}
}

//fonction qui nous permet de récupérer les registres si le programme fonctionne
int reg(int eargc,char ** eargv)
{
	if(isRunning)
	{
		printf("\n");
		get_reg();	
	}
	else
	{
		printf("No stack frame to read register from\n");
	}
	printf("\n");
}

//fonction qui nous permet de nous deplacé pas à pas dans la pile
//si le programme fonctionne
int prev(int eargc,char ** eargv)
{
	if(isRunning)
	{
		int status = unw_step(&cursor);
		if(status <= 0)
		{
			printf("No more stack frame to unwind\n");
		}
	}
	else
	{
		printf("Error: Not running\n");
	}
    printf("\n");
}

arg_struct attach_funct(int eargc, char ** eargv)
{
	printf("\n");
	arg_struct arg;
	arg.eargc = eargc;

	arg.eargv = malloc((arg.eargc+1)*sizeof(char*));
	for(int i = 0; i<arg.eargc;i++)
	{
		arg.eargv[i] = malloc(BUFF_SIZE*sizeof(char));
		strcpy(arg.eargv[i],eargv[i]);
	}
	arg.eargv[arg.eargc] = NULL;
	printf("Now attach to %s",arg.eargv[0]);
	if(arg.eargc != 1)
	{
		printf(" with argument: ");
		for(int i = 1; i<arg.eargc; i++)
		{
			printf("%s ",arg.eargv[i]);
		}
	}
	else
	{
		printf("\n");
	}

	printf("\n");
	printf("\n");
	return arg;
}

//fonction qui nous permet de parser les informations donnné au debugger
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

//fonction qui permet de libérer la memoire aloué par le parser
void deallocate_parsed(arg_struct parsed)
{
	for(int i = 0; i<parsed.eargc+1;i++)
	{
		free(parsed.eargv[i]);
	}
	free(parsed.eargv);
}

//fonction qui détache notre debugger et qui termine le fils si ce dernier
//ne s'est pas terminé
void end_process()
{
	int status = 0;
	isRunning = 0;
	end_backtrace();
	status = detach(child);
	if(status)
	{
		kill(child,SIGINT);
		exit(status);
	}
}

//fonction qui gère l'affichage de notre interface (menu d'aide affiché automatiquement)
void interface_affic()
{
	int nb_caract = 12;

	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	int nb_ast = (w.ws_col - nb_caract)/2;
	for(int i = 0; i<nb_ast;i++)
	{
		printf("#");
	}
	printf(" C debuger ");
	for(int i = 0; i<=nb_ast;i++)
	{
		printf("#");
	}
	printf("\n\n");
	printf(
		"Made by DIAS Nicolas and LAPLANCHE Alexis\n\n"

		"List of command:\n"
		"\t-'r' or 'run' to launch the sub process and start the tracing\n"
		"\t-'b' or 'breakpoint' to set a breakpoint\n"
		"\t-'p' or 'prev' to unwind stack frame\n"
		"\t-'bt' or 'backtrace' to show the bactrace of the program\n"
		"\t-'reg' or 'register' to show the register of the program\n"
		"\t-'attach' to attach the debugger to the binary we need to debug\n"
		"\t-'reset' to reset the cursor to the top of the stack frame\n"
		"\t-'clear' to cleat the terminal\n"
		"\t-'quit' to quit the program\n\n"
		
		);
	for (int i = 0; i < w.ws_col; i++)
	{
		printf("#");
	}
	printf("\n\n");
}

int main(int argc, char *argv[])
{
	interface_affic();
	
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
			if(isRunning)
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
		else if(!strcmp(parsed.eargv[0],"p") || !strcmp(parsed.eargv[0],"prev"))
		{
			prev(parsed.eargc-1,&parsed.eargv[1]);
		}
		else if(!strcmp(parsed.eargv[0],"attach"))
		{
			arg = attach_funct(parsed.eargc-1,&parsed.eargv[1]);
			isAttach = 1;
		}
		else if(!strcmp(parsed.eargv[0],"reset"))
		{
			if(isRunning)
			{
				cursor = BASE_cursor;
				printf("\n");
			}
			else
			{
				printf("Error: No stack frame found\n");
			}
		}
		else if(!strcmp(parsed.eargv[0],"clear"))
		{
			system("clear");
			interface_affic();
		}
		else if(!strcmp(parsed.eargv[0],"quit"))
		{
			if(isRunning)
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