#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "ch_dir.h"
#include "utils.h"

int cd(char** path, int argc, char* lastd) {
	int rval = 0;

	if(argc > 3) {
		char* mess;
		mess = "Shelly: cd: too many arguments\n";

		PRINT_ERR(mess, "", STRING);
		return 1;
	}
	else if(argc == 2) {
		rval = change_dir(getenv("HOME"));

		return rval;
	}
	else if(argc < 2) {
		char* mess;
		mess = "cd was given a not NULL terminated array!";

		PRINT_ERR(mess, "", STRING);
		exit(1);
	}

	if(strcmp(*(path + 1), "~") == 0) {
		rval = change_dir(getenv("HOME"));
	}
	else if(strcmp(*(path + 1), "-") == 0) {
		rval = change_dir(lastd);
		printf("%s\n", lastd);
	}
	else {
		rval = change_dir(*(path + 1));
	}

	return rval;
}

int change_dir(char* target) {
	if((chdir(target)) < 0) {
		if(errno == 2) {
			char* mess;
			mess = "Shelly: cd: no such file or directory:";

			PRINT_ERR(mess, target, STRING);
		}
		else {
			char* mess;
			mess = "unknown errno:";

			PRINT_ERR(mess, &errno, INT);
			exit(1);
		}

		return 1;
	}

	return 0;
}
