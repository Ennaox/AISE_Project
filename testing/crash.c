#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

void salut()
{
	for(int i=0;i<10;i++)
	{
		printf("Salut je vais crash\n");
	}
	kill(getpid(),SIGSEGV);
}

void bien_et_toi()
{
	salut();
}

void comment_va()
{
	bien_et_toi();
}

void yo_mec()
{
	comment_va();
}

int main(int argc, char const *argv[])
{
	yo_mec();
	return 0;
}