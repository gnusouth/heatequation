CC=clang
CFLAGS=-W -Wall -pedantic -O2 -std=gnu99
LDFLAGS=-lm -lpthread

BINARIES=heateq

.PHONY: all clean debug sequential slow plot

all: $(BINARIES)

heateq: heateq.c
	$(CC) $(CFLAGS) -g -o heateq heateq.c $(LDFLAGS)

debug:
	$(CC) $(CFLAGS) -D_DEBUG -o heateq heateq.c $(LDFLAGS)

sequential:
	$(CC) $(CFLAGS) -g -D_SEQUENTIAL -o heateq heateq.c

slow:
	$(CC) -W -Wall -pedantic -O0 -std=gnu99 -o heateq heateq.c

plot:
	@echo "Press [Enter] to finish"
	gnuplot -e "splot 'final.dat' matrix; pause -1"

clean:
	rm -f $(BINARIES)
