CC = gcc
CFLAGS = -Wall -Wextra
SRC = gui.c main.c cube.c coord.c moves.c util.c table.c test.c

all: debug

coord.c: meta.c util.c
	$(CC) $(CFLAGS) -o meta $^
	./meta

cube: $(SRC)
	$(CC) $(CFLAGS) -lSDL3 -lGL -lm -o $@ $^

debug: CFLAGS += -Wno-unused-function -Wno-unused-variable -g
debug: cube

release: CFLAGS += -O3
release: cube

clean:
	rm -f coord.c coord.h cube *.bin meta

run: all
	./cube

.PHONY: all clean debug release run
