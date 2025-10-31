CC = gcc
CFLAGS = -Wall -Wextra
SRC = cube.c cube-table.c coord.c moves.c util.c table.c

RELEASE = 0
ifeq ($(RELEASE), 1)
	CFLAGS += -O3
else
	CFLAGS += -g3 -Wno-unused-function -Wno-unused-variable -fsanitize=undefined
endif

cube: CFLAGS += -lSDL3 -lGL -lm
cube: $(SRC) gui.c main.c

test: $(SRC)

gen-coord: gen-coord.c util.c
coord.c: gen-coord
	./gen-coord

gen-cube-table: gen-cube-table.c cube.c util.c table.c moves.c coord.c
cube-table.c: gen-cube-table
	./gen-cube-table

clean:
	rm -f *bin
	rm -f coord.c coord.h cube-table.c
	rm -f cube gen-coord gen-cube-table test

.PHONY: clean
