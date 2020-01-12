#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "exec_bin.h"

pid_t pid;

int exec_bin(char** binary) {
	int child_exit = 253;
	signal(SIGINT, child_killer);

	pid = fork();
	if(pid == 0) {
		// In child process
	    	if(execvp(binary[0], binary) < 0) {
			if(errno == 2) {
				char* buff = malloc(1024);
				char* mess = "Shelly: command not found:";
				sprintf(buff, "%s %s\n", mess, binary[0]);
				write(STDERR_FILENO, buff, strlen(buff));
				child_exit = 127;

				free(buff);
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
			printf("\n");
			child_exit = 128 + WTERMSIG(wstatus);
		}
	}

	return child_exit;
}

void child_killer(int sig) {
	kill(pid, sig);
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
