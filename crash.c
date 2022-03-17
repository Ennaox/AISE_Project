#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

void test()
{

	double a = 1.1;
	double b = 1.2;
	double c = 1.3;
	double d = 1.4;
	double e = 1.5;
	double f = 1.6;
	
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
		// "movss (%[_a]), %%xmm0;\n"
		// "movss (%[_b]), %%xmm1;\n"
		// "movss (%[_c]), %%xmm2;\n"
		// "movss (%[_d]), %%xmm3;\n"
		// "movss (%[_e]), %%xmm4;\n"
		// "movss (%[_f]), %%xmm5;\n"

		:

		:	//input
			// [_a] "r" (&a),
		 //    [_b] "r" (&b),
		 //    [_c] "r" (&c),
		 //    [_d] "r" (&d),
		 //    [_e] "r" (&e),
		 //    [_f] "r" (&f)
		:
		"cc","memory","rax","rdx","rcx","rbx","rsi","rdi","r8","r9","r10",
		"r11","r12","r13","r14","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5"
	);
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