# Project purpose

Shell is created as credit for *Unix/Linux Programming in C (NSWI015)* at Faculty of Math and Physics, Charles University, Prague.

Task requirements:
https://devnull-cz.github.io/unix-linux-prog-in-c/class-assignments/labs-assignment-2019.txt

## State [unstable]

Phase 2 is now being implemented.

## TODO:

[ ] extend `struct command` of `char*` fields `input` `output`

[ ] make sure `struct command` is alocated and disposed correctly after the change

[ ] reimplement `get_cmds.l` to use updated `struct command` and correctly match I/O redirections

[ ] reimplement `exec_bin.c` to suport I/O redirections and pipeline creation and execution

[ ] optimize the memmory leaks

[ ] pass phase 2 tests
