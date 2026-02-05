src = $(shell find . -name '*.c' ! -path './bin/*')

INCLUDES = -Isrc/
LDFLAGS += -lmupdf -lpthread

CC = gcc
CFLAGS = -Wall -O3

build:
	mkdir -p bin
	$(CC) $(CFLAGS) -o bin/parse $(src) $(LDFLAGS)

run: build
	./bin/parse

clean:
	rm -r bin/
