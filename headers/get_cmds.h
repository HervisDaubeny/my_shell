#ifndef GET_CMDS_H
#define GET_CMDS_H

#include "utils.h"

struct command* get_coms(char*, int*);
int add_cmd();
int add_arg();
int save_input();
int save_output();
int clean_args();
int clean_cmds();

#endif
