
#include "pdwarf.h"

void print_param(PARAMETERS *param)
{
	if(param != NULL)
	{
		printf("name = %s\n",param->name);
		printf("declared in = %s\n",param->decl_file);
		printf("type = %s\n",param->type);
		printf("declaration line = %d\n",param->decl_line);
		if(param->next != NULL)
		{
			print_param(param->next);
		}
	}
	else
	{
		printf("PARAMETERS pointer is NULL\n");
	}
}

void print_vars(VARIABLES *vars)
{
	if(vars != NULL)
	{
		printf("name = %s\n",vars->name);
		printf("declared in = %s\n",vars->decl_file);
		printf("type = %s\n",vars->type);
		printf("declaration line = %d\n",vars->decl_line);
		if(vars->next != NULL)
		{
			print_vars(vars->next);
		}
	}
	else
	{
		printf("VARIABLES pointer is NULL\n");
	}
}

void print_subprogram(SUBPROGRAM *sub)
{
	if(sub != NULL)
	{
		printf("name = %s\n",sub->name);
		printf("declared in = %s\n",sub->decl_file);
		printf("type = %s\n",sub->type);
		printf("low_pc = %s\n",sub->low_pc);
		printf("high_pc = %s\n",sub->high_pc);
		printf("declaration line = %d\n",sub->decl_line);

		if(sub->next != NULL)
		{
			print_subprogram(sub->next);
		}

		if(sub->params != NULL)
		{
			print_param(sub->params);
		}

		if(sub->vars != NULL)
		{
			print_vars(sub->vars);
		}
	}
	else
	{
		printf("SUBPROGRAM pointer is NULL\n");
	}

}