CC := gcc
CFLAGS := -MMD -Werror -Wall -Wextra -Wno-unused-function -Wno-unused-variable -march=native
MAKEFLAGS := $(MAKEFLAGS) --jobs=$(shell nproc)
EXE := main debug meta
DEP := $(EXE:%=%.d)

all: debug main

main: CFLAGS := $(CFLAGS) -O3
debug meta: CFLAGS := $(CFLAGS) -g3 -fsanitize=undefined -fsanitize-trap -DDEBUG

clean:
	rm -rfv $(EXE) $(DEP) *.bin

meta: meta.c
	$(CC) $(CFLAGS) -o $@ $<
	./meta

main debug: main.c meta
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: all clean

-include $(DEP)
