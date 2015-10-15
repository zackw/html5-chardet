AWK    = awk
CC     = cc
CFLAGS = -g -O2 -std=c11 -pedantic -Wall -Wextra

all: test-prescan
clean:
	-rm -f test-prescan test-prescan.o prescan.o \
               uncanon-max.inc uncanon-max.incT
check: all
	./test-prescan tests/*.dat

test-prescan: test-prescan.o prescan.o
	$(CC) $(CFLAGS) test-prescan.o prescan.o -o test-prescan

prescan.o: prescan.c prescan.h encodings.inc uncanon-max.inc
test-prescan.o: test-prescan.c prescan.h

uncanon-max.inc: encodings.inc uncanon-max.awk
	$(AWK) -f uncanon-max.awk encodings.inc > uncanon-max.incT
	mv -f uncanon-max.incT uncanon-max.inc

.PHONY: all clean check
