#define _GNU_SOURCE
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
		char* msg;
		msg = "Shelly: error while opening file:";

		PRINT_ERR(msg, &errno, INT);
		exit(1);
	}

	return fd;
}

int fd_close(int fd) {
	int result = close(fd);
	if (result == -1) {
		char* msg;
		msg = "Shelly: error while closing file:";

		PRINT_ERR(msg, &errno, INT);
		exit(1);
	}

	return result;
}

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
				line[i] = buffer[i];
			}
			line[length] = '\0';

			FREE(buffer);
			return line;
		}
	}
}
