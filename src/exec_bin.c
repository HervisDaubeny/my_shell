#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "exec_bin.h"
#include "utils.h"

pid_t pid;

int exec_bin(struct command* binary) {
	int child_exit = 253;
	struct sigaction signalAction;
    signalAction.sa_handler = child_killer;

    sigaction(SIGINT, &signalAction, NULL);

	pid = fork();
	if(pid == 0) {
		// In child process
		if(binary->input != NULL) {
			close(0);
			int input = open(binary->input, O_RDONLY);
			if(input > 0) {
				dup(input);
			}
			else {
				if (errno == 2) {
					char* buff;
					char* mess;
					MALLOC(buff, 1024);
					mess = "Shelly: No such file or directory:";
					sprintf(buff, "%s %s\n", mess, binary->input);
					write(STDERR_FILENO, buff, strlen(buff));

					FREE(buff);
					exit(1);
				}
				else if (errno == 13) {
					char* buff;
					char* mess;
					MALLOC(buff, 1024);
					mess = "Shelly: Premission denied:";
					sprintf(buff, "%s %s\n", mess, binary->input);
					write(STDERR_FILENO, buff, strlen(buff));

					FREE(buff);
					exit(1);
				}
				else {
					char* buff;
					char* mess;
					MALLOC(buff, 1024);
					mess = "Shelly: Unexpected error. Errno:";
					sprintf(buff, "%s %d\n", mess, errno);
					write(STDERR_FILENO, buff, strlen(buff));

					FREE(buff);
					exit(1);
				}
			}
		}
		if(binary->output != NULL) {
			close(1);
			int output;

			if(binary->oout) {
				output = open(binary->output, O_CREAT | O_TRUNC | O_WRONLY, 00666);
			}
			else {
				output = open(binary->output, O_APPEND | O_CREAT | O_WRONLY, 00666);
			}

			if(output > 0) {
				dup(output);
			}
			else {
				if (errno == 13) {
					char* buff;
					char* mess;
					MALLOC(buff, 1024);
					mess = "Shelly: Premission denied:";
					sprintf(buff, "%s %s\n", mess, binary->output);
					write(STDERR_FILENO, buff, strlen(buff));

					FREE(buff);
					exit(1);
				}
				else {
					char* buff;
					char* mess;
					MALLOC(buff, 1024);
					mess = "Shelly: Unexpected error. Errno:";
					sprintf(buff, "%s %d\n", mess, errno);
					write(STDERR_FILENO, buff, strlen(buff));

					FREE(buff);
					exit(1);
				}
			}
		}

	  if(execvp(*(binary->value), binary->value) < 0) {
			if(errno == 2) {
				char* buff;
				char* mess;
				MALLOC(buff, 1024);
				mess = "Shelly: command not found:";
				sprintf(buff, "%s %s\n", mess, *(binary->value));
				write(STDERR_FILENO, buff, strlen(buff));
				child_exit = 127;

				FREE(buff);
			}
			else {
				printf("Couldn't execute, errno: %d\n", errno);
			}
		}

		//If 253 is returned I'm getting a different errno.
		exit(child_exit);
	}
	else if(pid == -1) {
		/*
		 * Something bad happend while forking. It really shouldn't re-
		 * turn 254 specifically to know it was this.
		 */
		return 254;
	}
	else {
		// In parrent process
		int wstatus = 0;

		wait(&wstatus);

		if(WIFEXITED(wstatus)) {
			child_exit = WEXITSTATUS(wstatus);
		}
		if(WIFSIGNALED(wstatus)) {
			child_exit = 128 + WTERMSIG(wstatus);
		}
	}

	return child_exit;
}

void child_killer(int sig) {
	kill(pid, sig);
	printf("Killed by signal %d.\n", sig);
}

#ifdef TEST
int main() {
	 char* binary[] = {"ls", "-l", "\0"};
	 int res = 0;

	 res = exec_bin(binary);
	 printf("%d user@device /current/directory %s ...\n", (int)res, "%");

	 return res;
}
#endif
