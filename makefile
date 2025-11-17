CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-function -Wno-unused-variable
LDFLAGS := -lSDL3 -lGL -lm

GEN := coord.c data.c
EXE := $(GEN:%.c=gen-%) main test
DEP := $(EXE:%=%.d)

debug: CFLAGS += -g3 -fsanitize=undefined -DDEBUG
debug: all

release: CFLAGS := $(CFLAGS) -O3
release: all

all: main test

clean:
	rm -f $(EXE) $(GEN) $(wildcard *.bin)

$(EXE): %: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -MMD -o $@ $<

$(GEN): %.c: gen-%
	./$^

.PHONY: all clean debug release

-include $(DEP)
