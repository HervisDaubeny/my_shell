#ifndef GET_CMDS_H
#define GET_CMDS_H

#include "utils.h"

struct command* get_coms(char*, int*);
int add_cmd(void);
int add_arg(char*);
int save_input(char*);
int save_output(char*);
int clean_args(void);
int clean_cmds(void);

#endif
