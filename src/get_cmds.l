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
	sep = ';';
	add_arg(NULL); /* to secure NULL termination */
	add_cmd();
}

\n {	/* end of line */
	if (argc > 0) {
		sep = '\n';
		add_arg(NULL); /* to secure NULL termination */
		add_cmd();
	}
}

\| {			/* pipe */
	sep = '|';
	add_arg(NULL); /* to secure NULL termination */
	add_cmd();
}

#.* {}     /* comment -> ignore to the $ */

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

. {}      /* bad char */
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
