NULL   =
CC     = cc -std=c11
PYTHON = python3
CYTHON = cython3 -3

all: # is the default.
include python-vars.mk

all: prescan.$M

prescan.$M: prescan.$O prescan-impl.$O
	$(CC) $(LINKER_ARGS)

clean:
	-rm -f \
	   _prescan.$M prescan.$O prescan-glue.$O prescan-glue.c \
	   python-vars.mk
	-rm -rf __pycache__

# Header dependencies
prescan.c: prescan.pyx

prescan-impl.$O: prescan-impl.c prescan.h
prescan.$O: prescan.c prescan.h

# Python boilerplate
python-vars.mk:
	$(PYTHON) get-module-compile-cmds.py $@

%.$O: %.c
	$(CC) $(COMPILER_ARGS)

%.c: %.pyx
	$(CYTHON) -I. -o $@ $<

.PHONY: all clean
