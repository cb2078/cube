CC = gcc
CFLAGS = -Wall -Wextra
SRC = cube.c coord.c moves.c util.c table.c

RELEASE = 0
ifeq ($(RELEASE), 1)
	CFLAGS += -O3
else
	CFLAGS += -g3 -Wno-unused-function -Wno-unused-variable -fsanitize=undefined
endif

cube: CFLAGS += -lSDL3 -lGL -lm
cube: $(SRC) gui.c main.c

meta: meta.c util.c
coord.c: meta
	./meta

test: $(SRC)

clean:
	rm -f coord.c coord.h cube *.bin meta test

.PHONY: clean
