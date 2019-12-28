#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <err.h>
#include <stdarg.h>
#include <string.h>

/* Function that executes binary given as argument. */
int exec_bin(char** binary) {
	pid_t pid = fork();
	int child_exit = 253;
	int wstatus = 0;

	if(pid == 0) {
		// In child process
	    	if (execvp(binary[0], binary) < 0) {
			if (errno == 2) {
				char buf[1024] = "hdsh: command not found: ";
				strcat(buf, binary[0]);
				strcat(buf, "\n");
				write(STDERR_FILENO, buf, sizeof(buf) - 1);
				child_exit = 127;
			}
			#ifdef TEST
			else {
				printf("Couldn't execute, errno: %d\n", errno);
			}
			#endif
		}

		//If 253 is returned I'm getting a different errno.
		exit(child_exit);
	}
	else if (pid == -1) {
		/*
		 * Something bad happend while forking. It really shouldn't re-
		 * turn 254 specifically to know it was this.
		 */
		return 254;
	}
	else {
		// In parrent process
		wait(&wstatus);

		if (!WIFEXITED(wstatus)) {
			/*
			 * perhaps signals will lead here, for now just print
			 * and return specifically 255
			 */
			printf("Child did not exit normally!\n");
			return 255;
		}
		child_exit = WEXITSTATUS(wstatus);
	}

	return child_exit;
}

#ifdef TEST
int main() {
	 char* binary[] = {"s", "l",  NULL};
	 int res = 0;

	 res = exec_bin(binary);
	 printf("%d user@device /current/directory %s ...\n", (int)res, "%");

	 return res;
}
#endif
