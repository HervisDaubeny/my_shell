#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "exit.h"
#include "utils.h"

int ext(struct command* cmd, int lret) {
	/* check argc */
	if(cmd->argc > 3) {
		char* mess;
		mess = "Shelly: exit: too many arguments";

		PRINT_ERR(mess, "", STRING);
		return 1;
	}
	else if(cmd->argc == 3) {
		/* check numeric argument */
		char** endn;
		MALLOC(endn, sizeof(char**));
		int rval = (int) strtol(*(cmd->value + 1), endn, 10);

		if(**endn == '\0') {
			exit(rval);
		}
		else {
			char* mess;
			mess = "Shelly: exit: numeric argument required:";

			PRINT_ERR(mess, *(cmd->value + 1), STRING);
			exit(2);
		}
	}
	else if(cmd->argc < 3) {
		exit(lret);
	}

	return lret;
}
