BINDIR=bin
CC=gcc
CFLAGS=-Wall -Iinc
INCDIR=inc
LFLAGS=
SRCDIR=src

override $(CFLAGS):=$(CFLAGS) $(addprefix -I, $(INCDIR))

# VPATH=$(SRCDIR):$(BINDIR)

all: $(BINDIR)
	$(CC) $(CFLAGS) $(SRCDIR)/main.c -o $(BINDIR)/main $(LFLAGS)

$(BINDIR):
	mkdir $(BINDIR)

clean:
	rm -rf bin/*
