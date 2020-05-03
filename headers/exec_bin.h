#ifndef EXEC_BIN_H
#define EXEC_BIN_H

#include "utils.h"

int exec_bin(struct command*);
int exec_pipe(struct command*, int);
void child_killer(int);
void execute_child(struct command*);
void redirect(char*, char*, int);
void kill_childern(pid_t*, size_t);

#endif
