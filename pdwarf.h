#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#define BUFF_SIZE 256

enum TYPE
{
    FCT,
    PAR,
    VAR
};

typedef struct parsed_str
{
    int eargc;
    char ** eargv;
} 
arg_struct;

//Liste chainé qui stock les paramètres d'une fonction
typedef struct params
{
    char name[256];
    char decl_file[512];
    char type[64];
    char decl_line[10];
    struct params *next;
} PARAMETERS;

//Liste chainé qui stock les variables donctenu dans une fonction
typedef struct vars
{
    char name[256];
    char decl_file[512];
    char type[64];
    char decl_line[10];
    struct vars *next;
} VARIABLES;

//Liste chainé qui stock les fonctions ainsi que des informations sur ces dernères
typedef struct subprogram
{
    char name[256];
    char decl_file[512];
    char type[64];
    char low_pc[19];
    char high_pc[19];
    char decl_line[10];
    PARAMETERS *params;
    VARIABLES *vars;
    struct subprogram *next;
} SUBPROGRAM;

//Pointeur vers la liste des fonctions
SUBPROGRAM *fcts_l;

//Pointeur vers la dernière fonction de la liste
SUBPROGRAM *last_fct;

//Pointeur vers la dernière variable de la liste
VARIABLES *last_vars;

//Pointeur vers le derniers paramètres de la liste
PARAMETERS *last_params;

//Permet d'afficher le contenu de la liste des paramètres d'une fonction
void print_param(PARAMETERS *param);

//Permet d'afficher le contenu de la liste des variable d'une fonction
void print_vars(VARIABLES *vars);

//Permet d'afficher le contenu de la liste des des fonctions
void print_subprogram(SUBPROGRAM *sub);

//Permet de parser une string et de renvoyer une tableau avec chaque case qui 
//contient un mot de la string. utilisé pour parser les commandes entrées ainsi 
//que le dwarf
arg_struct parse_str(char *buff);

//Permet d'intialisé la struc PARAMETERS
PARAMETERS *init_params(PARAMETERS *params);

//Permet d'intialisé la struc VARIABLES
VARIABLES *init_vars(VARIABLES *vars);

//Permet d'intialisé la struc SUBPROGRAM
SUBPROGRAM *init_fcts(SUBPROGRAM *fcts);

//Permet d'ajouter une fonction a la liste des fonctions
void add_fcts(SUBPROGRAM * curr);

//Permet d'ajouter un paramètre a la liste des paramètres
void add_params(PARAMETERS * curr);

//Permet d'ajouter une variable a la liste des variables
void add_vars(VARIABLES * curr);

//Fontion principale qui permet de parser le dwarf de notre bianaire et de
//remplir les structs présenter précedemment
int parse_dwarf(char *prog_loc);