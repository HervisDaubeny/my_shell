#include <unistd.h>                            
#include <stdlib.h>
#include <sys/types.h>                   
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int fd_open(char* file) {
	int fd = (int)open(file, O_RDONLY);

	return fd;
}

int fd_close(int fd) {
	return close(fd);
}

char fd_getc(int fd) {
	static int size = 10;
	static int poss = 0;
	static int chread = 0;
	static char buffer[10];
	char ret = '\0';

	if(poss >= chread) {
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

char* fd_getl(int fd, int* eof) {
	int size = 80;
	int length = 0;
	char* buffer = malloc(size);
	char* line;

	while(1) {
		char c = fd_getc(fd);
		if(c == -1) {
			/* EOF reached */
			c = '\n';
			*eof = 1;
		}

		if(length >= size - 1) {
			size *= 2;
			if((realloc(buffer, size)) == NULL) {
					printf("Error in malloc!\n");
					exit(1);
			}
		}

		buffer[length] = c;
		length ++;

		if(c == '\n' || *eof) {
			line = malloc(length + 1);

			for(int i = 0; i < length; i++) {
				line[i] = buffer[i];
			}
			line[length] = '\0';

			free(buffer);
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
		free(line);
		if(eof) {
			break;
		}
	}

	ret = fd_close(fd);

	return ret;
}
#endif
