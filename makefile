CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-function -Wno-unused-variable -march=native
MAKEFLAGS := $(MAKEFLAGS) --jobs=$(shell nproc)
EXE := main debug
DEP := $(EXE:%=%.d)

all: debug main

main: CFLAGS := $(CFLAGS) -O3

debug: CFLAGS := $(CFLAGS) -g3 -fsanitize=undefined -DDEBUG

clean:
	rm -rfv $(EXE) $(DEP) *.bin

$(EXE): %: main.c
	$(CC) $(CFLAGS) -MMD -o $@ $<

.PHONY: all clean

-include $(DEP)
