#ifndef MYSH_H
#define MYSH_H

#include "utils.h"

void set_env();
void free_env();
void run_script(char*);
void interactive_run();
void execute_line();
void sig_handler(int);
char* get_prompt();
void free_commands(struct command*, int);
int call_cd(struct command*);
int call_exit(struct command*);

#endif
