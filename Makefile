CC=clang
CFLAGS=-W -Wall -pedantic -std=gnu99 -g
LDFLAGS=-lm -lpthread

BINARIES=heat

.PHONY: all clean

all: $(BINARIES)

heat: heat.c

clean:
	rm -f $(BINARIES)
