#ifndef GLOBALS_H
#define GLOBALS_H

#define rv (*get_rv())
#define cwd (*get_cwd())
#define lwd (*get_lwd())
#define hostname (*get_hostname())
#define login (*get_login())
#define cmdc (*get_cmdc())

int* rv(void);
char** get_cwd(void);
char** get_lwd(void);
char** get_hostname(void);
char** get_login(void);
int* get_cmdc(void);

void set_globals(void);

#endif
