#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "exit.h"
#include "utils.h"

int ext(struct command* cmd, int lret) {
	/* check argc */
	if(cmd->argc > 3) {
		char* mess = "Shelly: exit: too many arguments\n";
		write(STDERR_FILENO, mess, strlen(mess));

		return 1;
	}
	else if(cmd->argc == 3) {
		/* check numeric argument */
		char** endn = malloc(sizeof(char**));
		int rval = (int) strtol(*(cmd->value + 1), endn, 10);

		if(**endn == '\0') {
			exit(rval);
		}
		else {
			char* buff = malloc(128);
			char* mess = "Shelly: exit: numeric argument required:";
			sprintf(buff, "%s %s\n", mess, *(cmd->value + 1));
			write(STDERR_FILENO, buff, strlen(buff));

			free(buff);
			exit(2);
		}
	}
	else if(cmd->argc < 3) {
		exit(lret);
	}

	return lret;
}
