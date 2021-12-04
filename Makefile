BINDIR=bin
CC=gcc
CCLAGC=
INCDIR=inc
LDFLAGS=
SRCDIR=src

all: $(BINDIR)
	$(CC) $(CCLAGC) -Wall main.c -o $(BINDIR)/main $(LDFLAGS)

$(BINDIR):
	mkdir $(BINDIR)

clean:
	rm -rf bin/*
