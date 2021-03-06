#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "exec_bin.h"
#include "utils.h"

pid_t pid;

int exec_bin(struct command* binary) {
	int child_exited = -1;

	pid = fork();
	if(pid == 0) {
		/* In child process */

		// Handle signals
		struct sigaction signalAction = {0};
	  signalAction.sa_handler = child_killer;
	  if(sigaction(SIGINT, &signalAction, NULL)) {
			char* msg;
			msg = "assigning signal handler to child process failed, errno:";

			PRINT_ERR(msg, &errno, INT);
		}

		// Handle process execution
		redirect(binary->input, binary->output, binary->oout);
		execute_child(binary);
	}
	else if(pid == -1) {
		/* fork() failed */
		char* msg;
		msg = "Shelly: unable to fork, errno:";

		PRINT_ERR(msg, &errno, INT);
		exit(1);
	}
	else {
		/* In parrent process */
		int wstatus = 0;

		wait(&wstatus);
		if(WIFEXITED(wstatus)) {
			child_exited = WEXITSTATUS(wstatus);
		}
		if(WIFSIGNALED(wstatus)) {
			child_exited = 128 + WTERMSIG(wstatus);
		}
	}

	return child_exited;
}

int exec_pipe(struct command* commands, int cmdc) {
	int NO_ITEMS = cmdc;
	int pipe_length = 1; // 1 for last item of pipe without sep == '|'

	// get pipe length
	for(int i = 0; i < NO_ITEMS; i++) {
	  if(((commands + i)->sep) == '|') {
			pipe_length++;
	  }
	  else {
			break;
	  }
	}

	// create array to store pid of all childern
	pid_t childern[pipe_length];
	for(int i = 0; i < pipe_length; i++) {
	  childern[i] = 0;
	}

	// create two pipes
	int r_pipe[2];
	int w_pipe[2];
	if(pipe(r_pipe)) {
		char* msg;
		msg = "creating reading pipe failed";

		PRINT_ERR(msg, "", STRING);
		exit(1);
	}
	if(pipe(w_pipe)) {
		char* msg;
		msg = "creating writing pipe failed";

		PRINT_ERR(msg, "", STRING);
		exit(1);
	}

	// execute pipeline
	for(int index = 0; index < pipe_length; index++) {
		pid_t pid = fork();

		if(pid == 0) {
			/* in child */
			struct sigaction childHandler = {0};
			childHandler.sa_handler = child_killer;
			if(sigaction(SIGINT, &childHandler, NULL)) {
				char* msg;
				msg = "assigning signal handler to child process failed, errno:";

				PRINT_ERR(msg, &errno, INT);
			}

			// manage the pipes
			if(index == 0) {
				close(1);
				dup(w_pipe[1]);
			}
			else if(index ==  pipe_length - 1) {
				close(0);
				dup(r_pipe[0]);
			}
			else {
				close(0);
				dup(r_pipe[0]);
				close(1);
				dup(w_pipe[1]);
			}
			close(r_pipe[0]);
			close(r_pipe[1]);
			close(w_pipe[0]);
			close(w_pipe[1]);

			// execute actual pipe command
			redirect((commands + index)->input, (commands + index)->output, (commands + index)-> oout);
			execute_child(commands + index);

			/* this code runs only if execute_child() failed -> stop the cycle */
			break;
		}
		else if (pid == -1) {
			/* fork() failed */
			char* msg;
			msg = "Shelly: unable to fork, errno:";

			PRINT_ERR(msg, &errno, INT);
			exit(1);
		}
		else {
			childern[index] = pid;

			close(r_pipe[0]);
			close(r_pipe[1]);
			r_pipe[0] = w_pipe[0];
			r_pipe[1] = w_pipe[1];

			if(index < pipe_length - 1) {
				if(pipe(w_pipe)) {
					char* msg;
					msg = "creating writing pipe failed";

					PRINT_ERR(msg, "", STRING);
					exit(1);
				}
			}
			else {
				close(w_pipe[0]);
				close(w_pipe[1]);
			}
		}
	}
	int wstatus = 0;
	int child_exited = -1;

	for(int i = 0; i < pipe_length; i++) {
		waitpid(childern[i], &wstatus, 0);
		if(WIFEXITED(wstatus)) {
			child_exited = WEXITSTATUS(wstatus);
		}
		if(WIFSIGNALED(wstatus)) {
			child_exited = 128 + WTERMSIG(wstatus);
			kill_childern(childern, sizeof(childern));
			break;
		}
	}

	return child_exited;
}

void execute_child(struct command* command) {
/* this method can only be called from child process! -> it's fine to exit() */
	int ret = 0;

	ret = execvp(*(command->value), command->value);

	/* this code runs only if exec() failed */
	if(ret) {
		if(errno == 2) {
			char* msg;
			msg = "Shelly: command not found:";

			PRINT_ERR(msg, *(command->value), STRING);
			exit(127);
		}
		else {
			char* msg;
			msg = "Shelly: couldn't execute, errno:";

			PRINT_ERR(msg, &errno, INT);
			exit(1);
		}
	}
}

void redirect(char* input, char* output, int override) {
	/* check input redirection */
	if(input != NULL) {
		close(0);
		int input_fd = open(input, O_RDONLY);
		if(input_fd) {
			dup(input_fd);
		}
		else {
			/* expected error */
			if (errno == ENOENT) {
				char* msg;
				msg = "Shelly: No such file or directory:";

				PRINT_ERR(msg, input, STRING);
				exit(1);
			}
			else if (errno == EACCES) {
				char* msg;
				msg = "Shelly: Premission denied:";

				PRINT_ERR(msg, input, STRING);
				exit(1);
			}

			/* unexpected error */
			if (errno > 0) {
				char* msg;
				msg = "Shelly: unexpected error, errno:";

				PRINT_ERR(msg, &errno, INT);
				exit(1);
			}
		}
	}

	/* check output redirection */
	if(output != NULL) {
		close(1);
		int output_fd;

		if(override) {
			output_fd = open(output, O_CREAT | O_TRUNC | O_WRONLY, 00666);
		}
		else {
			output_fd = open(output, O_APPEND | O_CREAT | O_WRONLY, 00666);
		}

		if(output_fd) {
			dup(output_fd);
		}
		else {
			if (errno == 13) {
				char* msg;
				msg = "Shelly: Premission denied:";

				PRINT_ERR(msg, output, STRING);
				exit(1);
			}

			if (errno > 0) {
				char* msg;
				msg = "Shelly: Unexpected error. Errno:";

				PRINT_ERR(msg, &errno, INT);
				exit(1);
			}
		}
	}
}

void child_killer(int sig) {
	kill(getpid(), sig);
}

void kill_childern(pid_t* childern, size_t count) {
	for(size_t i = 0; i < count; i++) {
		if(childern[i]) {
			kill(childern[i], SIGINT);
		}
		else {
			break;
		}
	}
}
