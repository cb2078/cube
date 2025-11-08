CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-function -Wno-unused-variable
GEN := coord.c cube-table.c
EXE := $(GEN:%.c=gen-%) main test
DEP := $(EXE:%=%.d)

default: debug

clean:
	rm -f $(EXE) $(GEN) $(wildcard *.bin)

debug: CFLAGS += -g3 -fsanitize=undefined
debug: main

release: CFLAGS += -O3
release: main

run:
	./main

main: LDFLAGS += -lSDL3 -lGL -lm

$(EXE): %: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -MMD -o $@

$(GEN): %.c: gen-%
	./$^

.PHONY: default clean debug release run

-include $(DEP)
