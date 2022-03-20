/*
	C debuger

	List of command:
	'r' or 'run' to launch the sub process and start the tracing
	'p' or 'prev' to unwind stack frame 
	's' or 'step' to step forward the execution of the program
	'bt' or 'backtrace' to show the bactrace of the program
	'reg' or 'register' to show the register of the program
	'info' to show basic info of the program
	'info full' to show full info of the program\n"
	'attach to attach the debugger to the binary we need to debug'
	'reset' to reset the cursor to the top of the stack
	'clear' to clear the terminal
	'quit' to quit the program
*/

#include "main.h"

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

//fonction qui permet au debugger de s'attacher au programme a vérifier
int attach(pid_t child)
{
	int waitStat = 0;
    waitpid(child, &waitStat, WUNTRACED);
    ptrace(PTRACE_CONT, child, NULL,NULL);
    waitpid(child, &waitStat, WUNTRACED);
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



//fonction nous permettant de récupérer la valeur de tout les registres indiqué
//dans la fonction
void get_reg()
{
	char ** name = malloc(33*sizeof(char*));
	for(int i = 0; i < 33; i++)
	{
		name[i] = malloc(5*sizeof(char));
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
	name[16] = "rip";
	name[17] = "xmm0";
	name[18] = "xmm1";
	name[19] = "xmm2";
	name[20] = "xmm3";
	name[21] = "xmm4";
	name[22] = "xmm5";
	name[23] = "xmm6";
	name[24] = "xmm7";
	name[25] = "xmm8";
	name[26] = "xmm9";
	name[27] = "xmm10";
	name[28] = "xmm11";
	name[29] = "xmm12";
	name[30] = "xmm13";
	name[31] = "xmm14";
	name[32] = "xmm15";

	unw_word_t ip;
	for(int i = 0; i < 17; i++)
	{
		unw_get_reg(&cursor, i, &ip);
		printf("%s\t%lx\t%lu\n",name[i],ip,ip);
	}

	unw_fpreg_t reg;
	printf("\n");

	for(int i = 17; i < 33; i++)
	{
		if(unw_get_fpreg(&cursor, i, &reg)<0)
		{
			printf("Error when reading on %s\n",name[i]);
		}
		printf("%s\t%Lf\n",name[i],reg);
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

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
		printf("ip: %lx\tsp: %lx\n",ip, sp);

		unw_proc_info_t proc_info;
		unw_get_proc_name(&cursor, sym, sizeof(sym), &offset);
		unw_get_proc_info(&cursor, &proc_info);

		printf("%lx: <%s+0x%lx>\n\n",proc_info.start_ip ,sym, offset);
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
		printf("\033[0;32mStarting program: \033[0;33m%s\n\033[0;32mProgram pid: \033[0;33m%d\033[0m\n\n", BUFF,child);

		int status = 0;
		if((status = attach(child)))
		{
			kill(child,SIGINT);
			return status;
		}

  		int waitStat = 0;
    	waitpid(child, &waitStat, WUNTRACED);


  		if(WIFEXITED(waitStat))
  		{
  			printf("\033[0;35mThe program: \033[0;33m%d \033[0;35mexited normally\033[0m\n",child);
  			child = 0;
  			isRunning = 0;
  		}
  		else
  		{	
  			init_backtrace(child);;
  			
  				siginfo_t result;
		    	ptrace(PTRACE_GETSIGINFO, child, 0, &result);
				printf("\n");
				#if __GLIBC__ >=2 && __GLIBC_MINOR__>=32
				printf("\033[1;35mProgram receiv the signal Yellow \033[0;33m%d\033[1;35m: SIG\033[0;33m%s: %s\n\033[1;35mError raised at \033[0;33m0x%p\033[0m\n",result.si_signo,sigabbrev_np(result.si_signo),strsignal(result.si_signo),result.si_addr);
		    	#else
		    	printf("\033[1;35mProgram receiv the signal Yellow \033[0;33m%d\033[1;35m: SIG\033[0;33m%s: %s\n\033[1;35mError raised at \033[0;33m0x%p\033[0m\n",result.si_signo,"",strsignal(result.si_signo),result.si_addr);
		    	#endif

  		}
    	printf("\n");
	}
	else
	{
		ptrace(PTRACE_TRACEME,0,0,0);
		raise(SIGSTOP);
		kill(getppid(),SIGCONT);
		execvp(arg.eargv[0],arg.eargv+sizeof(char *));
		return 0;
	}
	return 0;
}

//fonction qui vérifie si le programme fils fonctionne avant de lancé le backtrace
void backtrace_fct()
{
	if(isRunning)
	{
		backtrace(cursor);
	}
	else
	{
		printf("Error: can't backtrace: no program are running\n");
	}

}

//fonction qui nous permet de récupérer les registres si le programme fonctionne
void reg()
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
void prev()
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

//cette fonction n'est pas encore implémenté
void step()
{
	int status = ptrace(PTRACE_SINGLESTEP,child,0,0);
	int wstatus; 
	waitpid(child,&wstatus,WUNTRACED);
	if(status && WIFEXITED(wstatus)) 
	{
		printf("The program %d exited normally\n",child);
	}
	else if(status && WIFSIGNALED(wstatus))
	{
		siginfo_t result;
		ptrace(PTRACE_GETSIGINFO, child, 0, &result);
		#if __GLIBC__ >=2 && __GLIBC_MINOR__>=32
			printf("Program receiv the signal %d: SIG%s: %s\nError raised at 0x%p\n",result.si_signo,sigabbrev_np(result.si_signo),strsignal(result.si_signo),result.si_addr);
		#else
		    printf("Program receiv the signal %d: SIG%s: %s\nError raised at 0x%p\n",result.si_signo,"",strsignal(result.si_signo),result.si_addr);
		#endif
	}
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


void info(char * prog_loc)
{
	char BUFF[BUFF_SIZE];
	realpath(prog_loc,BUFF);
	printf("\n");
	printf("\033[0;32mPID: \033[0;33m%u\t\033[0;32mPPID: \033[0;33m%u\t\033[0;32mGID: \033[0;33m%u\n",child,getpid(),getgid());
	printf("\033[0;32mProgram location: \033[0;33m%s\033[0m\n",BUFF);

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
		printf("\033[0;36m#");
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
		"\t-'p' or 'prev' to unwind stack frame\n"
		"\t-'s' or 'step' to step forward the execution of the program\n"
		"\t-'bt' or 'backtrace' to show the bactrace of the program\n"
		"\t-'reg' or 'register' to show the register of the program\n"
		"\t-'info' to show basic info of the program\n"
		"\t-'info full' to show all info of the program\n"
		"\t-'attach' to attach the debugger to the binary we need to debug\n"
		"\t-'reset' to reset the cursor to the top of the stack frame\n"
		"\t-'clear' to clear the terminal\n"
		"\t-'quit' to quit the program\n\n"
		
		);
	for (int i = 0; i < w.ws_col; i++)
	{
		printf("#");
	}
	printf("\n\n\033[0m");
}

int main(int argc, char *argv[])
{
	interface_affic();
	
	fcts_l = NULL;
	last_fct = NULL;
	last_vars = NULL;
	last_params = NULL;

	arg_struct arg;
	arg_struct parsed;

	if(argc < 2)
	{
		isAttach = 0;
		printf("\033[1;35mWARNING: The debbuger isn't attach to any binary\n\tPlease attach it to a binary with the command 'attach [MyExecutable] [List of argument]'\033[1m\n\n");
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
		printf("\033[0;36mcdbg>\033[0m");
		fgets(buff,BUFF_SIZE,stdin);
		parsed = parse_str(buff);

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
				printf("\033[1;35mWARNING: Can't run: The debuger isn't attach to a binary\033[1m\n\n");
			}
		}
		else if(!strcmp(parsed.eargv[0],"bt") || !strcmp(parsed.eargv[0],"backtrace"))
		{
			backtrace_fct();
		}
		else if(!strcmp(parsed.eargv[0],"reg") || !strcmp(parsed.eargv[0],"register"))
		{
			reg();
		}
		else if(!strcmp(parsed.eargv[0],"p") || !strcmp(parsed.eargv[0],"prev"))
		{
			prev();
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
				printf("\033[1;35mError: No stack frame found, maybe the program hasn't been run\033[1m\n\n");
			}
		}
		else if(!strcmp(parsed.eargv[0],"clear"))
		{
			system("clear");
			interface_affic();
		}
		else if(!strcmp(parsed.eargv[0],"info"))
		{
			if(isAttach)
			{
				info(arg.eargv[0]);
				if(parsed.eargc >= 2 && !strcmp(parsed.eargv[1],"full"))
				{
					parse_dwarf(arg.eargv[0]);
					print_subprogram(fcts_l);
				}
			}
			else
			{
				printf("\n\033[1;35mWARNING: Can't show info: The debuger isn't attach to a binary\033[1m\n");
			}
			printf("\n");
		}
		else if(!strcmp(parsed.eargv[0],"s") || !strcmp(parsed.eargv[0],"step"))
		{
			if(isRunning)
			{
				printf("\033[1;31mNot yet implemented\033[1m\n\n");
				step();
			}
			else
			{
				printf("\033[1;35mError: no program has been launch\033[1m\n\n");
			}
		}
		else if(!strcmp(parsed.eargv[0],"make"))
		{
			printf("\033[1;37mAhah the debugger is still running! Use 'quit' then make\033[1m\n\n");
		}
		else if(!strcmp(parsed.eargv[0],"quit"))
		{
			if(isRunning)
			{
				end_process();
				deallocate_parsed(arg);
				deallocate_parsed(parsed);
			}
			break;
		}
		else
		{
			printf("\033[1;35mError: '%s' is an unknown command\033[1m\n\n",parsed.eargv[0]);
		}
		deallocate_parsed(parsed);
		memset(buff,0,BUFF_SIZE*sizeof(char));
	}
	return 0;
}