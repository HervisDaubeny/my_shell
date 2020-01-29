#include <unistd.h>                            
#include <stdlib.h>
#include <sys/types.h>                   
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

int fd_open(char* file) {
	int fd = open(file, O_RDONLY);
	if (fd == -1) {
		char* buff;
		char* mess;
		
		MALLOC(buff, 128);
		mess = "Shelly: error while opening file:";
		sprintf(buff, "%s %d\n", mess, errno);
		write(STDERR_FILENO, buff, strlen(buff));

		FREE(buff);
		exit(1);
	}

	return fd;
}

int fd_close(int fd) {
	int result = close(fd);
	if (result == -1) {
		char* buff;
		char* mess;
		
		MALLOC(buff, 128);
		mess = "Shelly: error while closing file:";
		sprintf(buff, "%s %d\n", mess, errno);
		write(STDERR_FILENO, buff, strlen(buff));

		FREE(buff);
		exit(1);
	}

	return result;
}

/*
char fd_getc(int fd) {
	static int size = 10;
	static int poss = 0;
	static int chread = 0;
	static char buffer[10];
	char ret = '\0';

	if(poss >= chread) {
		chread = 0;
		for(int i = 0; i < size; i++) {
			buffer[i] = '\0';
		}
		chread = (int)read(fd, buffer, size);
		poss = 0;

		if(chread == 0) {

			return -1;
		}
	}
	
	ret = buffer[poss];
	poss++;

	return ret;
}
*/

char fd_getc(int fd) {
	char buffer[1];
	ssize_t red = 0;
	ssize_t eof = 0;
	const int size = 1;

	red = read(fd, buffer, size);
	if(red == eof) {
		return -1;
	}

	return *buffer;
}
char* fd_getl(int fd, int* eof) {
	size_t size = 80;
	int length = 0;
	char* buffer; 
	char* line;
	MALLOC(buffer, size);

	while(1) {
		char c = fd_getc(fd);
		if(c == -1) {
			/* EOF reached */
			c = '\n';
			*eof = 1;
		}

		if(length >= (int)size - 1) {
			size *= 2;
			RELLOC(buffer, size);
		}

		buffer[length] = c;
		length ++;

		if(c == '\n' || *eof) {
			MALLOC(line, (length + 1));

			for(int i = 0; i < length; i++) {
				line [i] = buffer[i];
			}
			line[length] = '\0';

			FREE(buffer);
			return line;
		}
	}
	
}
#ifdef TEST
#include <stdio.h>

int main(int argc, char* argv[]) {
	int ret;
	int eof = 0;

	if(argc < 2) {
		return 1;
	}
	int fd = fd_open(argv[1]);

	while(1) {
		char* line = fd_getl(fd, &eof);
		printf("%s", line);
		FREE(line);
		if(eof) {
			break;
		}
	}

	ret = fd_close(fd);

	return ret;
}
#endif
