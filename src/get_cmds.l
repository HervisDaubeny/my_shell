%option noyywrap

%{
#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>

#include "get_cmds.h"
#include "utils.h"

int cmdc = 0;
int argc = 0;
int read_in = 0;
int read_out = 0;
int over_out = 0;
char sep = '\0';
char* saved_input = NULL;
char* saved_output = NULL;

struct cmd {
	char** binary;
	char* input;
	char* output;
	int over_out;
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

\| {			/* pipe */
	#ifdef TEST
	printf("%s %s\n", yytext, "pipe");
	#endif
	sep = '|';
	add_arg(NULL); /* to secure NULL termination */
	add_cmd();
}

#.* {     /* comment -> ignore to the $ */
	#ifdef TEST
	printf("%s %s\n",yytext, "comment");
	#endif
}

\>\> {		/* soft output redirection */
	if(read_in || read_out) {
		add_arg(yytext);
	}
	over_out = 0;
	read_out = 1;
}

\< {			/* input redirection */
	if(read_in || read_out) {
		add_arg(yytext);
	}
	read_in = 1;
}

\> {			/* hard output redirection */
	if(read_in || read_out) {
		add_arg(yytext);
	}
	over_out = 1;
	read_out = 1;
}

[^\ \t\n\r;<\|>]+ {			/* good word */
	#ifdef TEST
	printf("%s %s\n",yytext, "good word");
	#endif
	if(read_in) {
		save_input(yytext);
		read_in = 0;
	}
	else if(read_out) {
		save_output(yytext);
		read_out = 0;
	}
	else {
		add_arg(yytext);
	}
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

	MALLOC(result, (cmdc * sizeof(struct command)));
	TAILQ_FOREACH(iterate, &cmds, nextc) {
		(result + i)->value = iterate->binary;
		(result + i)->argc = iterate->argc;
		(result + i)->sep = iterate->sep;
		(result + i)->input = iterate->input;
		(result + i)->output = iterate->output;
		(result + i)->oout = iterate->over_out;
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
	yylex_destroy();

	return result;
}

int add_arg(char* text) {
	struct arg* new_arg;
	char* value;

	MALLOC(new_arg, (sizeof(struct arg)));

	if(text != NULL){
		MALLOC(value, (strlen(text) + 1));
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
	struct cmd* new_cmd;
	char** cmd_args;
	struct arg* iterate;
	char* in = NULL;
	char* out = NULL;

	MALLOC(new_cmd, (sizeof(struct cmd)));
	MALLOC(cmd_args, (sizeof(char *) * argc));

	if (saved_input != NULL) {
		MALLOC(in, (strlen(saved_input) + 1));
		strcpy(in, saved_input);
	}

	if (saved_output != NULL) {
		MALLOC(out, (strlen(saved_output) + 1));
		strcpy(out, saved_output);
	}

	int i = 0;
	TAILQ_FOREACH(iterate, &args, nexta) {
		*(cmd_args + i) = iterate->value;
		i++;
	}


	new_cmd->binary = cmd_args;
	new_cmd->argc = argc;
	new_cmd->sep = sep;
	new_cmd->input = in;
	new_cmd->output = out;
	new_cmd->over_out = over_out;
	TAILQ_INSERT_TAIL(&cmds, new_cmd, nextc);

	cmdc++;
	sep = '\0';
	over_out = 0;

	FREE(saved_input);
	FREE(saved_output);

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

int save_input(char* text) {
	MALLOC(saved_input, (strlen(text) + 1));
	strcpy(saved_input, text);
}

int save_output(char* text) {
	MALLOC(saved_output, (strlen(text) + 1));
	strcpy(saved_output, text);
}

int clean_args() {
	struct arg* argument;

	while(!TAILQ_EMPTY(&args)) {
		argument = TAILQ_FIRST(&args);
		TAILQ_REMOVE(&args, argument, nexta);
		FREE(argument);
	}
	argc = 0;

	return 0;
}

int clean_cmds() {
	struct cmd* command;

	while(!TAILQ_EMPTY(&cmds)) {
		command = TAILQ_FIRST(&cmds);
		TAILQ_REMOVE(&cmds, command, nextc);
		FREE(command);
	}
	cmdc = 0;

	return 0;
}

#ifdef TEST
int main() {
	char* err = "";
	int a = 0;
	struct command* result = get_coms("ls -l -a; echo 'A'\n", &a);

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
