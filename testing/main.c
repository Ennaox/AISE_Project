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

    	printf("on essaye\n");

    	int result = 0;
    	ptrace(PTRACE_GETSIGINFO, child, 0, &result);
		printf("erreur = %d\n",result);

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