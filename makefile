CC := gcc
CFLAGS := -MMD -Werror -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-parentheses -march=native
MAKEFLAGS := $(MAKEFLAGS) --jobs=$(shell nproc)
EXE := main debug meta
DEP := $(EXE:%=%.d)

all: debug main

main: CFLAGS := $(CFLAGS) -O3
debug meta: CFLAGS := $(CFLAGS) -g3 -fsanitize=undefined -fsanitize-trap -DDEBUG

clean:
	rm -rfv $(EXE) $(DEP) *.bin

test: main
	./main -v -t -j$(shell nproc)

meta: meta.c
	$(CC) $(CFLAGS) -o $@ $<
	./meta

main debug: main.c meta
	$(CC) $(CFLAGS) -o $@ $<

phase2-prune: main
	date
	rm -f phase2.bin
	time ./main -v -j$(shell nproc) < /dev/null

.PHONY: all clean test phase2-prune

-include $(DEP)
