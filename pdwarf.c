#include "pdwarf.h"

//Pointeur vers la liste des fonctions
SUBPROGRAM *fcts_l;

//Pointeur vers la dernière fonction de la liste
SUBPROGRAM *last_fct;

//Pointeur vers la dernière variable de la liste
VARIABLES *last_vars;

//Pointeur vers le derniers paramètres de la liste
PARAMETERS *last_params;

//Permet d'afficher le contenu de la liste des paramètres d'une fonction
void print_param(PARAMETERS *param)
{
	if(param != NULL)
	{
		printf("\n\033[0;34mPARAMETERS:\033[0m\n");
		printf("\t\033[0;32mname: \033[0;33m%s\033[0m\n",param->name);
		printf("\t\033[0;32mdeclared in: \033[0;33m%s\033[0m\n",param->decl_file);
		printf("\t\033[0;32mtype: \033[0;33m%s\033[0m\n",param->type);
		printf("\t\033[0;32mdeclaration line: \033[0;33m%s\033[0m\n",param->decl_line);
		if(param->next != NULL)
		{
			print_param(param->next);
		}
	}
	else
	{
		printf("\033[1;31mPARAMETERS pointer is NULL\033[1m\n");
	}
}

//Permet d'afficher le contenu de la liste des variable d'une fonction
void print_vars(VARIABLES *vars)
{
	if(vars != NULL)
	{
		printf("\n\033[0;34mVARIABLES:\033[0m\n");
		printf("\t\033[0;32mname: \033[0;33m%s\033[0m\n",vars->name);
		printf("\t\033[0;32mdeclared in: \033[0;33m%s\033[0m\n",vars->decl_file);
		printf("\t\033[0;32mtype: \033[0;33m%s\033[0m\n",vars->type);
		printf("\t\033[0;32mdeclaration line: \033[0;33m%s\033[0m\n",vars->decl_line);
		if(vars->next != NULL)
		{
			print_vars(vars->next);
		}
	}
	else
	{
		printf("\033[1;31mVARIABLES pointer is NULL\033[1m\n");
	}
}

//Permet d'afficher le contenu de la liste des des fonctions
void print_subprogram(SUBPROGRAM *sub)
{
	if(sub != NULL)
	{
		printf("\n\033[0;34mSUBPROGRAM:\033[0m\n");
		printf("\t\033[0;32mname: \033[0;33m%s\033[0m\n",sub->name);
		printf("\t\033[0;32mdeclared in: \033[0;33m%s\033[0m\n",sub->decl_file);
		printf("\t\033[0;32mtype: \033[0;33m%s\033[0m\n",sub->type);
		printf("\t\033[0;32mlow_pc: \033[0;33m%s\033[0m\n",sub->low_pc);
		printf("\t\033[0;32mhigh_pc: \033[0;33m%s\033[0m\n",sub->high_pc);
		printf("\t\033[0;32mdeclaration line: \033[0;33m%s\033[0m\n",sub->decl_line);

		if(sub->params != NULL)
		{
			print_param(sub->params);
		}

		if(sub->vars != NULL)
		{
			print_vars(sub->vars);
		}
		
		if(sub->next != NULL)
		{
			print_subprogram(sub->next);
		
		}

	}
	else
	{
		printf("\033[1;31mSUBPROGRAM pointer is NULL\033[1m\n");
	}

}

//Permet de parser une string et de renvoyer une tableau avec chaque case qui 
//contient un mot de la string. utilisé pour parser les commandes entrées ainsi 
//que le dwarf
arg_struct parse_str(char *buff)
{
	arg_struct parsed;
	parsed.eargc = 1;
	int inparenthese = 0;
	for(int i = 0; i<BUFF_SIZE;i++)
	{
		if(buff[i] == '(')
		{
			inparenthese = 1;
		}
		if (buff[i]==')')
		{
			inparenthese = 0;
		}
		if((buff[i] == ' ' || buff[i] == '\t') && inparenthese == 0)
		{
			parsed.eargc ++;
		}
		if(buff[i] == '\0')
			break;
		if(buff[i] == '\n')
		{
			buff[i] = '\0';
			break;
		}
	}
	parsed.eargv = malloc((parsed.eargc+1)*sizeof(char*));
	for(int i = 0; i<parsed.eargc;i++)
	{
		parsed.eargv[i] = malloc(BUFF_SIZE*sizeof(char));
		memset(parsed.eargv[i],0,BUFF_SIZE*sizeof(char));
	}
	parsed.eargv[parsed.eargc] = NULL;
	int len = strlen(buff);
	int j = 0, k=0;
	for(int i = 0;i<len;i++)
	{
		if(buff[i] == '(')
		{
			inparenthese = 1;
		}
		if (buff[i]==')')
		{
			inparenthese = 0;
		}
		if((buff[i] == ' ' || buff[i] == '\t') && inparenthese == 0)
		{
			j++;
			parsed.eargv[j][k] = '\0';
			k = 0;
		}
		else
		{
			parsed.eargv[j][k] = buff[i];
			k++;
		}
	}
	return parsed;
}

//Permet d'intialisé la struc PARAMETERS
PARAMETERS *init_params(PARAMETERS *params)
{
	memset(params->name,0,256*sizeof(char));
	memset(params->decl_file,0,512*sizeof(char));
	memset(params->type,0,64*sizeof(char));
	memset(params->decl_line,0,10*sizeof(char));
	params->next = NULL;
	return params;
}

//Permet d'intialisé la struc VARIABLES
VARIABLES *init_vars(VARIABLES *vars)
{
	memset(vars->name,0,256*sizeof(char));
	memset(vars->decl_file,0,512*sizeof(char));
	memset(vars->type,0,64*sizeof(char));
	memset(vars->decl_line,0,10*sizeof(char));
	vars->next = NULL;
	return vars;
}

//Permet d'intialisé la struc SUBPROGRAM
SUBPROGRAM *init_fcts(SUBPROGRAM *fcts)
{
	memset(fcts->name,0,256*sizeof(char));
	memset(fcts->decl_file,0,512*sizeof(char));
	memset(fcts->type,0,64*sizeof(char));
	sprintf(fcts->type,"void");
	memset(fcts->low_pc,0,19*sizeof(char));
	memset(fcts->high_pc,0,19*sizeof(char));
	memset(fcts->decl_line,0,10*sizeof(char));
	fcts->params = NULL;
	fcts->vars = NULL;
	fcts->next = NULL;
	return fcts;
}

//Permet d'ajouter une fonction a la liste des fonctions
void add_fcts(SUBPROGRAM * curr)
{
	if(fcts_l == NULL)
	{
		fcts_l = curr;
		last_fct = curr;
	}
	else
	{
		last_fct->next = curr;
		last_fct = curr;
	}
}

//Permet d'ajouter un paramètre a la liste des paramètres
void add_params(PARAMETERS * curr)
{
	PARAMETERS *tmp = last_fct->params;
	if(last_fct->params == NULL)
	{
		last_fct->params = curr;
		return;
	}
	while(tmp->next!=NULL)
	{
		tmp = tmp->next;
	}
	tmp->next = curr;
}

//Permet d'ajouter une variable a la liste des variables
void add_vars(VARIABLES * curr)
{
	VARIABLES *tmp = last_fct->vars;
	if(last_fct->vars == NULL)
	{
		last_fct->vars = curr;
		return;
	}
	while(tmp->next!=NULL)
		tmp = tmp->next;
	tmp->next = curr;
}

//Fontion principale qui permet de parser le dwarf de notre bianaire et de
//remplir les structs présenter précedemment
int parse_dwarf(char * prog_loc)
{
	char cmd[256];
	sprintf(cmd,"llvm-dwarfdump %s | grep -n 'DW_AT_name[[:blank:]]\\+(\"main\")' | cut -f1 -d:",prog_loc);
	FILE * dwarf = popen(cmd,"r");

	char nb_line_char[64];
	fgets(nb_line_char,64*sizeof(char),dwarf);
	pclose(dwarf);
	unsigned nb_line = atoi(nb_line_char) - 2;

	sprintf(cmd,"llvm-dwarfdump %s | tail -n +%u|awk '/DW_TAG_subprogram/,/NULL/'",prog_loc,nb_line);
	dwarf = popen(cmd,"r");
	char buff[1024];
	memset(buff,0,1024*sizeof(char));
	
	enum TYPE last_type;
	while(fgets(buff,1024*sizeof(char),dwarf)!= NULL)
	{
		arg_struct parsed;
		parsed = parse_str(buff);
		
		for(int i = 0;i<parsed.eargc;i++)
		{
			if(!strcmp(parsed.eargv[i],"DW_TAG_subprogram"))
			{
				
				SUBPROGRAM *curr = malloc(sizeof(SUBPROGRAM));
				curr = init_fcts(curr);
				add_fcts(curr);
				last_vars = NULL;
				last_params = NULL;
				last_type = FCT;
				break;
			}
			else if(!strcmp(parsed.eargv[i],"DW_TAG_formal_parameter"))
			{
				PARAMETERS *curr = malloc(sizeof(PARAMETERS));
				curr = init_params(curr);
				last_params = curr;
				add_params(curr);
				last_type = PAR;
				break;
			}
			else if(!strcmp(parsed.eargv[i],"DW_TAG_variable"))
			{
				VARIABLES *curr = malloc(sizeof(VARIABLES));
				curr = init_vars(curr);
				last_vars = curr;
				add_vars(curr);
				last_type = VAR;
				break;
			}
			else if(!strcmp(parsed.eargv[i],"DW_AT_name"))
			{
				if(last_type == FCT)
				{
					for(int j=i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_fct->name,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
				if(last_type == VAR)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_vars->name,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
				if(last_type == PAR)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_params->name,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
			}
			else if(!strcmp(parsed.eargv[i],"DW_AT_decl_file"))
			{
				if(last_type == FCT)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_fct->decl_file,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
				if(last_type == VAR)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_vars->decl_file,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
				if(last_type == PAR)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_params->decl_file,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
			}
			else if(!strcmp(parsed.eargv[i],"DW_AT_decl_line"))
			{
				if(last_type == FCT)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_fct->decl_line,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
				if(last_type == VAR)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_vars->decl_line,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
				if(last_type == PAR)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_params->decl_line,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
			}
			else if(!strcmp(parsed.eargv[i],"DW_AT_type"))
			{
				if(last_type == FCT)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_fct->type,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
				if(last_type == VAR)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_vars->type,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
				if(last_type == PAR)
				{
					for(int j =i+1; j<parsed.eargc;j++)
					{
						if(parsed.eargv[j][0]!='\0')
						{
							sprintf(last_params->type,"%s",parsed.eargv[j]);
							break;
						}

					}
				}
			}
			else if(!strcmp(parsed.eargv[i],"DW_AT_low_pc"))
			{
				for(int j =i+1; j<parsed.eargc;j++)
				{
					if(parsed.eargv[j][0]!='\0')
					{
						sprintf(last_fct->low_pc,"%s",parsed.eargv[j]);
						break;
					}

				}
			}
			else if(!strcmp(parsed.eargv[i],"DW_AT_low_pc"))
			{
				for(int j =i+1; j<parsed.eargc;j++)
				{
					if(parsed.eargv[j][0]!='\0')
					{
						sprintf(last_fct->high_pc,"%s",parsed.eargv[j]);
						break;
					}

				}
			}

		}

	}
	pclose(dwarf);

	return 0;
}