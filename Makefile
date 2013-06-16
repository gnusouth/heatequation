CC=clang
CFLAGS=-W -Wall -pedantic -std=gnu99 -g
LDFLAGS=-lm -lpthread

BINARIES=heateq

.PHONY: all clean debug

all: $(BINARIES)

heateq: heateq.c

debug:
	$(CC) $(CFLAGS) -D_DEBUG -o heateq heateq.c $(LDFLAGS)

clean:
	rm -f $(BINARIES)
