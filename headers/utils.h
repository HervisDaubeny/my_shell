#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

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
	free(ptr);\
	ptr = NULL;\
}\
while(0)

/*
 * endsource
 */

struct command {
	/* if fields input, output == null: default I/O is used */
	char** value;
	char* input;
	char* output;
	char sep;
	int argc; /* null terminating included */
};

#endif
