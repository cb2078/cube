CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-function -Wno-unused-variable -DTHREADS=$(shell nproc)
LDFLAGS := -lSDL3 -lGL -lm
MAKEFLAGS := $(MAKEFLAGS) --jobs=$(shell nproc)
EXE := main test
DEP := $(EXE:%=%.d)

all: debug

debug: CFLAGS := $(CFLAGS) -g3 -fsanitize=undefined -DDEBUG
debug: main

release: CFLAGS := $(CFLAGS) -O3
release: main

test: CFLAGS := $(CFLAGS) -g3 -fsanitize=undefined -DDEBUG

clean:
	rm -f $(EXE) $(wildcard *.bin)

$(EXE): %: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -MMD -o $@ $<

.PHONY: all clean debug release

-include $(DEP)
