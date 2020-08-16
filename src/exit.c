#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include "exit.h"
#include "utils.h"

int ext(struct command* cmd, int lret) {
	/* check argc */
	if(cmd->argc > 3) {
		char* msg;
		msg = "Shelly: exit: too many arguments";

		PRINT_ERR(msg, "", STRING);
		return 1;
	}
	else if(cmd->argc == 3) {
		/* check numeric argument */
		char* endn = "";
		errno = 0;
		int rval = (int) strtol(*(cmd->value + 1), &endn, 10);

		if(*endn == '\0') {
			exit(rval);
		}
		else if(errno == ERANGE && (rval == INT_MAX || rval == INT_MIN)) {
			char* msg;
			msg = "Shelly: strtol errno:";

			PRINT_ERR(msg, &errno, INT);
      exit(2);
    }
		else {
			char* msg;
			msg = "Shelly: exit: numeric argument required:";

			PRINT_ERR(msg, *(cmd->value + 1), STRING);
			exit(2);
		}
	}
	else if(cmd->argc < 3) {
		exit(lret);
	}

	return lret;
}
