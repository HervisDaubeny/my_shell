CC = gcc
F = flex
S = src/
CFLAGS = -I headers -Wall -Wextra -ggdb3
TFLAGS = -D TEST

TTARGETS = texec_bin tget_cmd
MYSH_OBJECTS = mysh.o ch_dir.o exit.o exec_bin.o lex.yy.o fd_readl.o
REMOVE = mysh lex.yy.c val

.PHONY: all clean test

all: mysh

mysh: $(MYSH_OBJECTS)
	$(CC) -o mysh $(MYSH_OBJECTS) -l readline

mysh.o: $(S)mysh.c
	$(CC) $(CFLAGS) -c $<

ch_dir.o: $(S)ch_dir.c
	$(CC) $(CFLAGS) -c $<

exit.o: $(S)exit.c
	$(CC) $(CFLAGS) -c $<

exec_bin.o: $(S)exec_bin.c
	$(CC) $(CFLAGS) -c $<

lex.yy.o: lex.yy.c 	#replaces get_cmds.o
	$(CC) -I headers -w -c $<
	# -w here is to ignore flex's unused variables, not mine warnings

fd_readl.o: $(S)fd_readl.c
	$(CC) $(CFLAGS) -c $<

lex.yy.c: $(S)get_cmds.l
	$(F) $<

clean:
	@rm -fv *.o $(TTARGETS) $(REMOVE)
