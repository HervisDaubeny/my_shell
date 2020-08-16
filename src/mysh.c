#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
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

	set_env();

	int opt;
	while((opt = getopt(argc, argv, ":c:")) != -1) {
		char* msg;

		switch(opt) {
			case 'c':
				execute_line(optarg);
				return rval;
			case ':':
				msg = "Shelly: option requires argument: -";

				PRINT_ERR(msg, &optopt, STRING);
				return 2;
			case '?':
				msg = "Shelly: invalid option: -";

				PRINT_ERR(msg, &optopt, STRING);
				return 2;
		}
	}

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

	struct sigaction promptAction = {0};
	promptAction.sa_handler = int_handler;
	if(sigaction(SIGINT, &promptAction, NULL)) {
		char* msg;
		msg = "Assigning sig handler to main process failed, errno:";

		PRINT_ERR(msg, &errno, INT);
	}

	while(1) {
		prompt = get_prompt();
		line = readline(prompt);
		if(line == NULL) {
			printf("exit\n");

			FREE(prompt);
			break;
		}

		/* block signals for main process druing a command execution */
		struct sigaction old = {0};
		promptAction.sa_handler = SIG_IGN;
		if(sigaction(SIGINT, &promptAction, &old)) {
			char* msg;
			msg = "Blocking signals in main process failed, errno:";

			PRINT_ERR(msg, &errno, INT);
		}

		add_history(line);
		execute_line(line);

		/* allow signals for main process after a command executed */
		if(sigaction(SIGINT, &old, NULL)) {
			char* msg;
			msg = "Reenabling signals in main process failed, errno:";

			PRINT_ERR(msg, &errno, INT);
		}

		FREE(prompt);
		FREE(line);
	}
}

void execute_line(char* line) {
	struct command* commands = get_coms(line, &cmdc);
	int cmd_check = 1;

	for(int i = 0; i < cmdc; i++) {
		if((commands + i)->argc == 1) {
			if(i + 1 < cmdc || *((commands + i)->value) == NULL) {
				char* msg;
				msg = "Shelly: syntax error near unexpected token:";

				PRINT_ERR(msg, &(commands + i)->sep, STRING);

				rval = 2;
				cmd_check = 0;

				break;
			}
			else {
				break;
			}
		}

		char* left = "<";
		char* right = ">";
		for(int j = 0; j < ((commands + i)->argc) - 1; j++) {
			char* contains = strstr(*((commands + i)->value + j), left);
			if(contains) {
				char* msg;
				msg = "Shelly: syntax error near unexpected token:";

				PRINT_ERR(msg, *((commands + i)->value + j), STRING);
				rval = 2;
				cmd_check = 0;

				break;
			}

			contains = strstr(*((commands + i)->value + j), right);
			if(contains) {
				char* msg;
				msg = "Shelly: syntax error near unexpected token:";

				PRINT_ERR(msg, *((commands + i)->value + j), STRING);
				rval = 2;
				cmd_check = 0;

				break;
			}
		}
	}

	if(cmd_check) {
		int pipe = 0;

		for(int i = 0; i < cmdc; i++) {
			/* take care of pipeline execution and skip what was executed */
			if((commands+i)->sep == '|') {
				if (pipe) { // already executed -> skip
					continue;
				}
				pipe = 1; // execute pipeline
				rval = exec_pipe(commands + i, cmdc);
				continue;
			}
			else if(pipe) { // last command that got pipelined input -> skip
				pipe = 0;
				continue;
			}

			/* take care of "notpiped" command execution */
			if(strcmp(*((commands + i)->value), "cd") == 0) {
				rval = call_cd(commands + i);
			}
			else if(strcmp(*((commands + i)->value), "exit") == 0) {
				rval = call_exit(commands + i);
			}
			else {
				rval = exec_bin(commands + i);
			}
		}
	}

	free_commands(commands, cmdc);
}

void int_handler(int signum) {
	write(STDOUT_FILENO, "\n130 ", strlen("\n130 "));
	/* I know SIG_INT has value of 2, so I can print 130 directly, cause this
	 * function will only ever be used as handler for SIG_INT.
	 */
	rval = 128 + signum;

	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}
void set_env() {
	if((cwd = getcwd(NULL, 0)) == NULL) {
		char* msg;
		msg = "Shelly: Current working directory couldn't be	retrieved, errno:";

		PRINT_ERR(msg, &errno, INT);
		exit(1);
	}
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
		FREE((commands + i)->input);
		FREE((commands + i)->output);
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
