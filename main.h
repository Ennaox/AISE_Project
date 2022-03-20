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
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
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

//fonction qui initialise tous les pointeurs et variables utilise par libunwind
//pour faire un backtrace
int init_backtrace(pid_t child);

//fonction qui permet au debugger de s'attacher au programme a vérifier
int attach(pid_t child);

//fonction qui permet de se détacher du programme une fois que l'on a terminé
int detach(pid_t child);

//fonction nous permettant de récupérer la valeur de tout les registres indiqué
//dans la fonction
void get_reg();
void get_freg();

//fonction qui lance le backtrace (regarder dans la pile les fonctions qui s'y trouvent)
//cette fonction récupère toutes les fonctions de la pile
void backtrace();

//fonction qui permet de libérer les pointeurs utilisé par libunwind
void end_backtrace();

//non fonctionnelle
void breakpoint(unsigned addr);

//non fonctionnelle
void read_elf(char *name);

//fonction qui nous permet de lancé le programme a débugger dans un processus fils,
//qui s'attache a ce dernier et récupère le signal retourner
int run(arg_struct arg);

//fonction qui vérifie si le programme fils fonctionne avant de lancé le backtrace
void backtrace_fct();

//fonction qui nous permet de récupérer les registres si le programme est lancé
void reg();

//fonction qui nous permet de récupérer les registres flotant si le programme est lancé
void prev();

//fonction qui nous permet de nous deplacé pas à pas dans la pile si le programme est lancé
void step();

//cette fonction n'est pas encore implémenté
arg_struct attach_funct(int eargc, char ** eargv);

//fonction qui permet de libérer la memoire aloué par le parser
void deallocate_parsed(arg_struct parsed);

//fonction qui détache notre debugger et qui termine le fils si ce dernier
//ne s'est pas terminé
void end_process();

//fonction qui affiche les info sur le programme qui est lancé
void info(char * prog_loc);

//fonction qui gère l'affichage de notre interface (menu d'aide affiché automatiquement)
void interface_affic();