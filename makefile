#!/bin/bash

CC = gcc
CFLAGS = -Wall -Wextra
TFLAGS = -D TEST

PARTS = exec_bin
TTARGETS = texec_bin

.PHONY: all clean test

all: $(PARTS)

# create PARTS from item.o in PARTS
%: %.o
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

test: $(TTARGETS)

texec_bin: exec_bin.o
	$(CC) $< -o $@

%.o: %.c
	$(CC) $(TFLAGS) $(CFLAGS) -c $<

clean:
	@rm -fv *.o $(TTARGETS)
