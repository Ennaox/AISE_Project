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

typedef struct params
{
    char name[256];
    char decl_file[512];
    char type[64];
    char decl_line[10];
    struct params *next;
} PARAMETERS;

typedef struct vars
{
    char name[256];
    char decl_file[512];
    char type[64];
    char decl_line[10];
    struct vars *next;
} VARIABLES;

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

SUBPROGRAM *fcts_l;
SUBPROGRAM *last_fct;

VARIABLES *last_vars;

PARAMETERS *last_params;

char last_type[256];

void print_param(PARAMETERS *param);
void print_vars(VARIABLES *vars);
void print_subprogram(SUBPROGRAM *sub);
arg_struct parse_str(char *buff);
PARAMETERS *init_params(PARAMETERS *params);
VARIABLES *init_vars(VARIABLES *vars);
SUBPROGRAM *init_fcts(SUBPROGRAM *fcts);
void add_fcts(SUBPROGRAM * curr);
void add_params(PARAMETERS * curr);
void add_vars(VARIABLES * curr);
int parse_dwarf(char *prog_loc);