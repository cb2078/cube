CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-function -Wno-unused-variable
# CFLAGS += -g3 -fsanitize=undefined
CFLAGS += -O3
GEN := coord.c data.c
EXE := $(GEN:%.c=gen-%) main test
DEP := $(EXE:%=%.d)

all: main

clean:
	rm -f $(EXE) $(GEN) $(wildcard *.bin)

main: LDFLAGS += -lSDL3 -lGL -lm

$(EXE): %: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -MMD -o $@ $<

$(GEN): %.c: gen-%
	./$^

.PHONY: all clean

-include $(DEP)
