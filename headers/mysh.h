#ifndef MYSH_H
#define MYSH_H

#include "utils.h"

void set_env(void);
void free_env(void);
void run_script(char*);
void interactive_run(void);
void execute_line(char*);
void int_handler(int);
char* get_prompt(void);
void free_commands(struct command*, int);
int call_cd(struct command*);
int call_exit(struct command*);

#endif
