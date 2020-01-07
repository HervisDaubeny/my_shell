#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "ch_dir.h"

int cd(char** path, int argc, char* lastd) {
	if(argc > 3) {
		char* mess = "Shelly: cd: too many arguments\n";
		write(STDERR_FILENO, mess, strlen(mess));

		return 1;
	}
	else if(argc < 2) {
		printf("cd was given a not NULL terminated array!\n");
		exit(1);
	}

	if(strcmp(*(path + 1), "~") == 0 || strcmp(*(path + 1), "") == 0) {
		*(path + 1) = getenv("HOME");
	}
	else if(strcmp(*(path + 1), "-") == 0) {
		*path = lastd;
	}
	if(chdir(*(path + 1)) < 0) {
		if(errno == 2) {
			char* buff = malloc(254);
			char* mess = "Shelly: cd: no such file or directory:";
			sprintf(buff, "%s %s\n", mess, *(path + 1));
			write(STDERR_FILENO, buff, strlen(buff));

			free(buff);
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
	char* lwd = malloc(128);
	char* tmp = malloc(128);
	char* cwd = malloc(128);
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

	free(cwd);
	free(tmp);
	free(lwd);

	return 0;
}
#endif
