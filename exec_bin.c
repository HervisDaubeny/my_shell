#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int exec_bin(**char binary) {
  int fork_val = fork()
  int exec_val = 0;

  switch (fork_val) {
    case 0:
      //In child process
      exec_val = execvp(binary);
      break;
    case -1:
      //Error while forking happend, do something appropriate
      break;
    default :
      //In parrent, handle exec_val
      break;
  }
}
