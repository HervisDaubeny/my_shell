#define _XOPEN_SOURCE 700
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
#include "fd_readl.h"

static int rval;
static int cmdc;
static char* cwd;
static char* lwd;

int main(int argc, char* const* argv) {
	rval = 0;
	cmdc = 0;

	int opt;
	while((opt = getopt(argc, argv, ":c:")) != -1) {
		char* buff;
		char* mess;
		
		switch(opt){
			case 'c':
				execute_line(optarg);
				return rval;
			case ':':
				MALLOC(buff, 128);
				mess = "Shelly: option requires argument\
					      :";
				sprintf(buff, "%s %c\n", mess, optopt);
				write(STDERR_FILENO, buff, strlen(buff));

				FREE(buff);
				return 2;
			case '?':
				MALLOC(buff, 128);
				mess = "Shelly: invalid option:";
				sprintf(buff, "%s %c\n", mess, optopt);
				write(STDERR_FILENO, buff, strlen(buff));

				FREE(buff);
				return 2;
		}
	}
	
	set_env();
	
	if(optind < argc) {
		run_script(argv[1]);
	}
	else {
		interactive_run();
	}

	return rval;
}

void run_script(char* file) {
	int eof = 0;
	int fd = fd_open(file);

	while(1) {
		char* line = fd_getl(fd, &eof);
		execute_line(line);

		FREE(line);
		if(eof || rval) {
			break;
		}
	}

	fd_close(fd);
}

void interactive_run() {
	char* line = NULL;
	char* prompt;

    struct sigaction signalAction;
    signalAction.sa_handler = sig_handler;
    
	while(1) {
	
		sigaction(SIGINT, &signalAction, NULL);
		prompt = get_prompt();
		line = readline(prompt);
		if(line == NULL) {
			printf("exit\n");

			FREE(prompt);
			break;
		}

		add_history(line);
		execute_line(line);

		FREE(prompt);
		FREE(line);
	}
}

void execute_line(char* line) {
	struct command* commands = get_coms(line, &cmdc);
	int cc = 1;

	for(int i = 0; i < cmdc; i++) {
		if((commands + i)->argc == 1) {
			if(i + 1 < cmdc || *((commands + i)->value) == NULL) {
				char* buff;
				char* msg;
				
				MALLOC(buff, 128);
				msg =\
				"Shelly: syntax error near unexpected token:";
				
				sprintf(buff, "%s '%c'\n", msg,\
				(commands + i)->sep);
				write(STDERR_FILENO, buff, strlen(buff));

				FREE(buff);
				rval = 2;
				cc = 0;

				break;
			}
			else {
				break;
			}
		}
	}

	if(cc) {
		for(int i = 0; i < cmdc; i++) {
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
	FREE(prompt);
}

void set_env() {
	cwd = getcwd(NULL, (size_t)0);
	if((lwd = getenv("OLDPWD")) == NULL) {
		MALLOC(lwd, strlen(cwd));
		lwd = strcpy(lwd, cwd);
	}
}

void free_commands(struct command* commands, int cmdc) {
	for(int i = 0; i < cmdc; i++) {
		for(int j = 0; j < (commands + i)->argc; j++) {
			FREE(*((commands + i)->value + j));
		}
		FREE((commands + i)->value);
	}

	FREE(commands);
}

char* get_prompt() {
	char* prompt;
	char* cuwd;
	cuwd = getcwd(NULL, (size_t)0);
	MALLOC(prompt, (strlen(cuwd) + 9));

	if(rval > 0) {
		sprintf(prompt, "%d %s $ ", rval, cuwd);
	}
	else {
		sprintf(prompt, "%s $ ", cuwd);
	}

	FREE(cuwd);
	return prompt;
}

int call_cd(struct command* cmd) {
	int ret = 0;
	char* tmp;
	
	MALLOC(tmp, 128);
	strcpy(tmp, cwd);

	ret = cd(cmd->value, cmd->argc, lwd);
	if(rval == 0) {
		strcpy(lwd, tmp);
	}
	FREE(cwd);
	FREE(tmp);

	cwd = getcwd(NULL, (size_t)0);
	
	return ret;
}

int call_exit(struct command* cmd) {
	int ret = 0;
	ret = ext(cmd, rval);
	
	return ret;
}
