#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "mysh.h"
#include "exit.h"
#include "ch_dir.h"
#include "get_cmds.h"
#include "exec_bin.h"
#include "utils.h"

static int rval;
static int cmdc;
static char* cwd;
static char* lwd;

int main(int argc, char* const* argv) {
	rval = 0;
	cmdc = 0;
	cwd = malloc(128);
	lwd = malloc(128);
	char* line;
	char* prompt;

	int opt;
	while((opt = getopt(argc, argv, ":c:")) != -1) {
		char* buff;
		char* mess;
		switch(opt){
			case 'c':
				execute_line(optarg);
				return rval;
			case ':':
				buff = malloc(128);
				mess = "Shelly: option requires argument\
					      :";
				sprintf(buff, "%s %c\n", mess, optopt);
				write(STDERR_FILENO, buff, strlen(buff));

				free(buff);
				return 2;
			case '?':
				buff = malloc(128);
				mess = "Shelly: invalid option:";
				sprintf(buff, "%s %c\n", mess, optopt);
				write(STDERR_FILENO, buff, strlen(buff));

				free(buff);
				return 2;
		}
	}

	set_env();
	if(optind < argc) {
		/* there is file to be read.. and possibly filthy hobitsses */

		//return ...
	}

	interactive_run();

	return rval;
}

void interactive_run() {
	char* line = NULL;
	char* prompt;

	while(1) {
		signal(SIGINT, sig_handler);
		prompt = get_prompt();
		line = readline(prompt);
		if(line == NULL) {
			printf("exit\n");
			break;
		}

		add_history(line);
		execute_line(line);

		free(prompt);
		free(line);
		line = NULL;
	}
}

void execute_line(char* line) {
	struct command* commands = get_coms(line, &cmdc);

	for(int i = 0; i < cmdc; i++){
		if((commands + i)->argc == 1) {
			if(i + 1 < cmdc) {
				char* buff = malloc(128);
				char* msg =\
				"Shelly: syntax error near unexpected token";
				sprintf(buff, "%s: '%c'\n", msg,\
				(commands + 1)->sep);
				write(STDERR_FILENO, buff, strlen(buff));

				free(buff);
				rval = 2;

				break;
			}
			else {
				break;
			}
		}
		if(strcmp(*((commands + i)->value), "cd") == 0) {
			rval = call_cd(commands + i);
		}
		else if(strcmp(*((commands + i)->value), "exit") == 0) {
			rval = call_exit(commands + i);
		}
		else {
			rval = exec_bin((commands + i)->value);
		}
	}

	free_commands(commands, cmdc);
}

void sig_handler(int sig) {
	printf("\n");
	rval = 128 + sig;
	char* prompt = get_prompt();

	rl_set_prompt(prompt);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();

	signal(SIGINT, sig_handler);
	free(prompt);
}

void set_env() {
	cwd = getcwd(NULL, (size_t)0);
	if((lwd = getenv("OLDPWD")) == NULL) {
		lwd = malloc(strlen(cwd));
		lwd = strcpy(lwd, cwd);
	}
}

void free_commands(struct command* commands, int cmdc) {
	for(int i = 0; i < cmdc; i++) {
		for(int j = 0; j < (commands + i)->argc; j++) {
			free(*((commands + i)->value + j));
		}
		free((commands + i)->value);
	}

	free(commands);
}

char* get_prompt() {
	char* prompt;
	char* cwd;
	cwd = getcwd(NULL, (size_t)0);
	prompt = malloc(strlen(cwd) + 4);

	if(rval > 0) {
		sprintf(prompt, "%d %s $ ", rval, cwd);
	}
	else {

		sprintf(prompt, "%s $ ", cwd);
	}

	free(cwd);

	return prompt;
}

int call_cd(struct command* cmd) {
	int ret = 0;
	char* tmp = malloc(128);
	strcpy(tmp, cwd);

	ret = cd(cmd->value, cmd->argc, lwd);
	if(rval == 0) {
		strcpy(lwd, tmp);
	}
	free(tmp);

	return ret;
}

int call_exit(struct command* cmd) {
	int ret = 0;
	ret = ext(cmd, rval);

	return ret;
}
