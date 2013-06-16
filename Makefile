CC=clang
CFLAGS=-W -Wall -pedantic -std=gnu99 -g
LDFLAGS=-lm -lpthread

BINARIES=heateq

.PHONY: all clean debug plot

all: $(BINARIES)

heateq: heateq.c

debug:
	$(CC) $(CFLAGS) -D_DEBUG -o heateq heateq.c $(LDFLAGS)

plot:
	@echo "Press [Enter] to finish"
	gnuplot -e "splot 'final.dat' matrix; pause -1"

clean:
	rm -f $(BINARIES)
