#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATH "/bin:/usr/bin"

int exec_bin(char** binary) {
	pid_t pid = fork();
	int exec_val = 0;

	if(pid == 0) {
		//In child process
	    	if (execvp(binary[0], binary) < 0) {
			if (errno == 2) {
				//TODO: cover known errnos
			}

			printf("Couldn't execute, errno: %d\n", errno);
		}
		exit(0);
	}
	else if (pid == -1) {
		return 1;
	}
	else {
		wait(NULL);
	}

	return exec_val;
}

int main() {
	 char* binary[] = {"s", "-l", "-a", NULL};
	 int res = 0;

	 res = exec_bin(binary);

	 return 0;
}
