.PHONY: all clean

all: fork-example tree-example ex2-fork ex2-signals ex2-1 ex2-2 ex2-3 ex2-4

CC = gcc
CFLAGS = -g -Wall -O2
SHELL= /bin/bash

tree-example: tree-example.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

fork-example: fork-example.o proc-common.o
	$(CC) $(CFLAGS) $^ -o $@

ex2-fork: ex2-fork.o proc-common.o
	$(CC) $(CFLAGS) $^ -o $@

ex2-signals: ex2-signals.o proc-common.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

ex2-1:	ex2-1.o proc-common.o
	$(CC) $(CFLAGS) $^ -o $@

ex2-2:	ex2-2.o proc-common.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

ex2-3: ex2-3.o proc-common.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

ex2-4: ex2-4.o proc-common.o tree.o
	$(CC) $(CFLAGS) $^ -o $@


%.s: %.c
	$(CC) $(CFLAGS) -S -fverbose-asm $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o tree-example fork-example ex2-{fork,signals,1,2,3,4}
