CC := gcc
CFLAGS := -Wall -Wextra
# CFLAGS += -O3
CFLAGS += -g3 -Wno-unused-function -Wno-unused-variable -fsanitize=undefined
GEN := coord.c cube-table.c
EXE := $(GEN:%.c=gen-%) main test
DEP := $(EXE:%=%.d)

all: main

clean:
	rm -f $(EXE) $(GEN) $(wildcard *.bin)

main: LDFLAGS += -lSDL3 -lGL -lm
$(EXE): %: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -MMD -o $@

$(GEN): %.c: gen-%
	./$^

.PHONY: all clean

-include $(DEP)
