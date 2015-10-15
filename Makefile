CC     = cc
CFLAGS = -g -O2 -std=c11 -pedantic -Wall -Wextra

all: test-prescan
clean:
	-rm -f test-prescan test-prescan.o prescan.o
check: all
	./test-prescan tests/*.dat

test-prescan: test-prescan.o prescan.o
	$(CC) $(CFLAGS) test-prescan.o prescan.o -o test-prescan

prescan.o: prescan.c prescan.h
test-prescan.o: test-prescan.c prescan.h

.PHONY: all clean check
