# Project purpose

Shell is created as credit for *Unix/Linux Programming in C (NSWI015)* at Faculty of Math and Physics, Charles University, Prague.

Task requirements:
https://devnull-cz.github.io/unix-linux-prog-in-c/class-assignments/labs-assignment-2019.txt

## State [stable]

Shell now complies to phase 2 requirements.

## TODO:

- [x] extend `struct command` of `char*` fields `input` `output`

- [x] make sure `struct command` is alocated and disposed correctly after the change (in `mysh.c`)

- [x] reimplement `get_cmds.l` to use updated `struct command` and correctly match I/O redirections

- [x] reimplement `exec_bin.c` to suport I/O redirections

- [x] pipeline creation and execution

- [x] optimize the memmory leaks

- [x] pass phase 2 tests

- [x] replace every print to STDERR with PRINT_ERR macro

- [x] cleanup the code
