.PHONY: all clean

all: zing_main_outcome

CC = gcc
CFLAGS = -Wall -fprofile-arcs -ftest-coverage
LDFLAGS = -fprofile-arcs -ftest-coverage
TESTPATH = ../../../test/

zing_main_outcome: main.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

test:
	@$(TESTPATH)test.pl

clean:
	rm zing_main_outcome main.o *.gcno *.gcda *.gcov
