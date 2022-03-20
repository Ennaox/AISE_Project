#pragma once
#define _GNU_SOURCE
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
#include <errno.h>
#include "pdwarf.h"

#define BUFF_SIZE 256

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

int init_backtrace(pid_t child);
int attach(pid_t child);
int detach(pid_t child);
void get_reg();
void backtrace();
void end_backtrace();
int run(arg_struct arg);
void backtrace_fct();
void reg();
void prev();
void step();
arg_struct attach_funct(int eargc, char ** eargv);
arg_struct parse_str(char *buff);
void deallocate_parsed(arg_struct parsed);
void end_process();
void info(char * prog_loc);
void interface_affic();