#ifndef MYSH_H
#define MYSH_H

#include "utils.h"

void set_env();
int interactive_run();
int execute_line();
char* get_prompt();
void free_commands(struct command*, int);
int call_cd(struct command*);
int call_exit(struct command*);

#endif
