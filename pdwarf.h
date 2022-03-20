#pragma once
#ifndef pdwarf_h
#define pdwarf_h

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
    int decl_line;
    struct params *next;
} PARAMETERS;

typedef struct vars
{
    char name[256];
    char decl_file[512];
    char type[64];
    int decl_line;
    struct vars *next;
} VARIABLES;

typedef struct subprogram
{
    char name[256];
    char decl_file[512];
    char type[64];
    char low_pc[19];
    char high_pc[19];
    int decl_line;
    PARAMETERS *params;
    VARIABLES *vars;
    struct subprogram *next;
} SUBPROGRAM;

SUBPROGRAM *fcts_l = NULL;
SUBPROGRAM *last_fct = NULL;

VARIABLES *last_vars = NULL;

PARAMETERS *last_params = NULL;

char last_type[256];

void print_param(PARAMETERS *param);
void print_vars(VARIABLES *vars);
void print_subprogram(SUBPROGRAM *sub);

#endif