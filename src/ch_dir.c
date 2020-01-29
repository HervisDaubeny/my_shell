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
		char* mess = "Shelly: cd: too many arguments\n";
		write(STDERR_FILENO, mess, strlen(mess));

		return 1;
	}
	else if(argc == 2) {
		rval = change_dir(getenv("HOME"));

		return rval;
	}
	else if(argc < 2) {
		printf("cd was given a not NULL terminated array!\n");
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
			char* buff;
			char* mess;
			MALLOC(buff, 254);
			mess = "Shelly: cd: no such file or directory:";
			sprintf(buff, "%s %s\n", mess, target);
			write(STDERR_FILENO, buff, strlen(buff));

			FREE(buff);
		}
		else {
			printf("unknown errno: %d\n", errno);
			exit(1);
		}

		return 1;
	}

	return 0;
}

#ifdef TEST
int main() {
	char* cmda[] = {"cd", "~", NULL};
	int cmdc = 3;
	int ret = 0;
	char* lwd, tmp, cwd;
	MALLOC(lwd, 128);
	MALLOC(tmp, 128);
	MALLOC(cwd, 128);
	getcwd(cwd, (size_t)128);
	strcpy(lwd, cwd);
	
	/* backup cwd */
	strcpy(tmp, cwd);

	/* change dir ~ */
	printf("prompt > cd ~\n");
	if(cd(cmda, cmdc, lwd) == 0) {
		strcpy(lwd, tmp);
		getcwd(cwd, (size_t)128);
		printf("cwd: %s\n", cwd);
		printf("lwd: %s\n", lwd);
	}

	/* change dir back */
	printf("prompt > cd .\n");
	cmda[1] = ".";
	strcpy(tmp, cwd);
	if(cd(cmda, cmdc, lwd) == 0) {
		strcpy(lwd, tmp);
		getcwd(cwd, (size_t)128);
		printf("cwd: %s\n", cwd);
		printf("lwd: %s\n", lwd);
	}
	
	/* change dir via empty cd */
	printf("prompt > cd ..\n");
	cmda[1] = "..";
	strcpy(tmp, cwd);
	if(cd(cmda, cmdc, lwd) == 0) {
		strcpy(lwd, tmp);
		getcwd(cwd, (size_t)128);
		printf("cwd: %s\n", cwd);
		printf("lwd: %s\n", lwd);
	}

	/* change dir to nonexistent dir */
	printf("prompt > cd l\n");
	cmda[1] = "l";
	strcpy(tmp, cwd);
	ret = cd(cmda, cmdc, lwd);
	if((ret) == 0) {
		strcpy(lwd, tmp);
		getcwd(cwd, (size_t)128);
		printf("cwd: %s\n", cwd);
		printf("lwd: %s\n", lwd);
		printf("prompt >\n");
	}
	else {
		printf("%d prompt>\n", ret);
	}

	/* change dir with too many args */
	printf("prompt > cd l p\n");
	char* cmdb[] = {"cd", "l", "p", NULL};
	strcpy(tmp, cwd);
	ret = cd(cmdb, cmdc+1, lwd);
	if((ret) == 0) {
		strcpy(lwd, tmp);
		getcwd(cwd, (size_t)128);
		printf("cwd: %s\n", cwd);
		printf("lwd: %s\n", lwd);
		printf("prompt >\n");
	}
	else {
		printf("%d prompt>\n", ret);
	}

	FREE(cwd);
	FREE(tmp);
	FREE(lwd);

	return 0;
}
#endif
