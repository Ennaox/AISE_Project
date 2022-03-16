#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

void test()
{
	__asm__ volatile
	(
		"mov $1, %%rax;\n"
		"mov $2, %%rdx;\n"
		"mov $3, %%rcx;\n"
		"mov $4, %%rbx;\n"
		"mov $5, %%rsi;\n"
		"mov $6, %%rdi;\n"
		"mov $9, %%r8;\n"
		"mov $10, %%r9;\n"
		"mov $11, %%r10;\n"
		"mov $12, %%r11;\n"
		"mov $13, %%r12;\n"
		"mov $14, %%r13;\n"
		"mov $15, %%r14;\n"
		"mov $16, %%r15;\n"

		:

		:

		:
		"cc","memory","rax","rdx","rcx","rbx","rsi","rdi","r8","r9","r10",
		"r11","r12","r13","r14","r15"
	);
	raise(SIGSEGV);
}

void salut()
{
	test();
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