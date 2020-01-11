#ifndef UTILS_H
#define UTILS_H

struct command {
	char** value;
	char sep;
	int argc; /* null terminating included */
};
#endif
