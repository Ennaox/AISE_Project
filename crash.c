#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

void test()
{
	for(int i=0;i<10;i++)
	{
		//sleep(1);
		printf("Salut je vais crash\n");
	}
	raise(SIGSEGV);

}

void salut()
{
	test();
}

void bien_et_toi(char const *argv[])
{
	salut(argv);
}

void comment_va(char const *argv[])
{
	bien_et_toi(argv);
}

void yo_mec(char const *argv[])
{
	comment_va(argv);
}

int main(int argc, char const *argv[])
{
	yo_mec(argv);
	return 0;
}