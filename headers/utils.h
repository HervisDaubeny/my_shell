#ifndef UTILS_H
#define UTILS_H
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * source: https://gist.github.com/maestrelli/1191287
 */

#define MALLOC(ptr, size) do {\
	ptr = malloc(size);\
	if(ptr == NULL) {\
		printf("malloc() error, NULL returned\n");\
		exit(1);\
	}\
}\
while(0)

#define RELLOC(ptr, size) do {\
	void* check = (void*)ptr;\
	check = realloc(ptr, size);\
	if(check == NULL) {\
		printf("realloc() error, NULL returned\n");\
		exit(1);\
	}\
}\
while(0)

#define FREE(ptr) do {\
	if(ptr != NULL) {\
		free(ptr);\
		ptr = NULL;\
	}\
}\
while(0)

/*
 * endsource
 */

#define INT 0
#define STRING 1

#define PRINT_ERR(message, argument, type) do {\
	char* buffer;\
	int size = 0;\
	if(type) {\
		size = asprintf(&buffer, "%s %s\n", message, (char*) argument);\
	}\
	else {\
		size = asprintf(&buffer, "%s %d\n", message, *((int*) argument));\
	}\
	if(size < 0) {\
		FREE(buffer);\
		exit(1);\
	}\
	write(STDERR_FILENO, buffer, strlen(buffer));\
	FREE(buffer);\
}\
while(0)

struct command {
	/* if fields input, output == null: default I/O is used */
	/* if oout == 1 output file is to be overriden, else appended to */
	char** value;
	char* input;
	char* output;
	char sep;
	int argc; /* null terminating included */
	int oout;
};

#endif
