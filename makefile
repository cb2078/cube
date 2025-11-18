CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-function -Wno-unused-variable
LDFLAGS := -lSDL3 -lGL -lm
MAKEFLAGS += --jobs=$(shell nproc)
EXE := main test
DEP := $(EXE:%=%.d)

debug: CFLAGS += -g3 -fsanitize=undefined -DDEBUG
debug: all

release: CFLAGS := $(CFLAGS) -O3
release: all

all: $(EXE)

clean:
	rm -f $(EXE) $(wildcard *.bin)

$(EXE): %: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -MMD -o $@ $<

.PHONY: all clean debug release

-include $(DEP)
