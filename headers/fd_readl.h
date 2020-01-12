#ifndef FD_READL_H
#define FD_READL_H

int fd_open(char*);
int fd_close(int);
char fd_getc(int);
char* fd_getl(int, int*);

#endif
