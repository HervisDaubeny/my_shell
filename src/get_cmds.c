%option noyywrap                   

%{                                       
#include <stdlib.h>
#include <stdio.h>                
#include <sys/queue.h>            
#include <sys/types.h>
#include <string.h>                                
#include <stdbool.h>

#include "get_cmds.h"

int cmdc = 0;
int argc = 0;
char sep = '\0';

struct cmd {
	char** binary;
	char sep;
	int argc;
	TAILQ_ENTRY(cmd) nextc;
};

struct arg {
	 char* value;
	 TAILQ_ENTRY(arg) nexta;
};

TAILQ_HEAD(cmdhead, cmd) cmds;
TAILQ_HEAD(arghead, arg) args;

%}

%%
; {      /* split */
	#ifdef TEST
	printf("%s %s\n",yytext, "semicol");
	#endif
	sep = ';';
	add_arg(NULL); /* to secure NULL termination */
	add_cmd();
}

\n {	/* end of line */
	#ifdef TEST
	printf("%s %s\n",yytext, "eol");
	#endif
	if (argc > 0) {
		sep = '\n';
		add_arg(NULL); /* to secure NULL termination */
		add_cmd();
	}
}

#* {     /* ignore to the $ */
	#ifdef TEST
	printf("%s %s\n",yytext, "comment");
	#endif
}

[^\ \t\n\r;]+ { /* good word */
	#ifdef TEST
	printf("%s %s\n",yytext, "good word");
	#endif
	add_arg(yytext);
}

. {      /* bad char */
	#ifdef TEST
	printf("'%s' %s\n",yytext, "bad word");
	#endif
}
%%

struct command* get_coms(char* line, int* command_count) {
	TAILQ_INIT(&cmds);
	TAILQ_INIT(&args);
	struct command* result;
	struct cmd* iterate;
	int i = 0;

	yy_scan_string(line);
	yylex();

	if(argc > 0) {
		add_arg(NULL);
		add_cmd();
	}

	result = malloc(cmdc * sizeof(struct command));
	if (result == NULL) {
		printf("Error in mallock of result\n");
		exit(1);
	}

	TAILQ_FOREACH(iterate, &cmds, nextc) {
		(result + i)->value = iterate->binary;
		(result + i)->argc = iterate->argc;
		(result + i)->sep = iterate->sep;
		#ifdef TEST
		printf("result value at: %p\n", (result + i)->value);
		for (int j = 0; j < (result + i)->argc; j++) {
			printf("%s at %p\n", *((result + i)->value + j), (result + i)->value + j);
		}
		#endif
		i++;
	}
	*command_count = cmdc;
	clean_cmds();

	return result;
}

int add_arg(char* text) {
	struct arg* new_arg = malloc(sizeof(struct arg));
	char* value;

	if(text != NULL){
		value = malloc(strlen(text));

		if (new_arg == NULL || value == NULL) {
			printf("%s\n", "Error during malloc for arg!");
			exit(1);
		}
		strcpy(value, text);
		new_arg->value = value;
	}
	else {
		new_arg->value = NULL;
	}

	TAILQ_INSERT_TAIL(&args, new_arg, nexta);

	argc++;

	return 0;
}

int add_cmd() {
	struct cmd* new_cmd = malloc(sizeof(struct cmd));
	char** cmd_args = malloc(sizeof(char*) * argc);
	struct arg* iterate;

	int i = 0;

	if (new_cmd == NULL || cmd_args == NULL) {
		printf("%s\n", "Error during malloc for cmd!");
		exit(1);
	}
	
	TAILQ_FOREACH(iterate, &args, nexta) {
		*(cmd_args + i) = iterate->value;
		i++;
	}
	
	new_cmd->binary = cmd_args;
	new_cmd->argc = argc;
	new_cmd->sep = sep;
	TAILQ_INSERT_TAIL(&cmds, new_cmd, nextc);

	cmdc++;
	sep = '\0';
	clean_args();

	#ifdef TEST
	struct cmd* test;
	TAILQ_FOREACH(test, &cmds, nextc) {
		for (int i = 0; i < test->argc; i++) {
			printf("%s at: %p \n", *(test->binary+i), test->binary+i);
		}
		printf("arg count: %d\n", test->argc);
	}
	#endif

	return 0;
}

int clean_args() {
	struct arg* argument;

	while(!TAILQ_EMPTY(&args)) {
		argument = TAILQ_FIRST(&args);
		TAILQ_REMOVE(&args, argument, nexta);
		free(argument);
		argument = NULL;
	}
	argc = 0;

	return 0;
}

int clean_cmds() {
	struct cmd* command;

	while(!TAILQ_EMPTY(&cmds)) {
		command = TAILQ_FIRST(&cmds);
		TAILQ_REMOVE(&cmds, command, nextc);
		free(command);
		command = NULL;
	}
	cmdc = 0;

	return 0;
}

#ifdef TEST
int main() {
	char* err = "";
	struct command* result = get_coms("ls -l -a; echo 'A'\n");

	if (argc > 0) {
		err = " - ended in the middle of parsing!";
	}
	printf("argc: %d%s cmdc: %d\n", argc, err, cmdc);
	printf("parsed commands are:\n");

	for (int i = 0; i < cmdc; i++) {
		int arg_c = (result + i)->argc;
		for (int j = 0; j < arg_c; j++) {
			printf("%s ", *((result + i)->value + j));
		}
		printf("\n");
	}
	return 0;
}
#endif
