CC=gcc
CFLAGS=-Wall -Iinc
DEBUG=0
INCDIR=inc
LFLAGS=
OUTDIR=_out
PROJECT=ll
SRCDIR=src
VERSION=0.0.6

override $(CFLAGS):=$(CFLAGS) $(addprefix -I, $(INCDIR))

ifeq ($(DEBUG), 1)
	override $(CFLAGS) :=-D DEBUG $(CFLAGS)
endif

# VPATH=$(SRCDIR):$(BINDIR)

all: bin asm
	@echo 'All targets is done...'

bin: $(OUTDIR) version
	@echo 'Creating bin...DEBUG=$(DEBUG)'
	$(CC) $(CFLAGS) $(SRCDIR)/$(PROJECT).c -o $(OUTDIR)/$(PROJECT) $(LFLAGS)

asm: $(OUTDIR) version
	@echo 'Creating asm...'
	@$(CC) -S $(CFLAGS) $(SRCDIR)/$(PROJECT).c -o $(OUTDIR)/$(PROJECT).s $(LFLAGS)

$(OUTDIR):
	@echo 'Creating dir $(OUTDIR)...'
	@mkdir $@

version:
	@echo 'Creating version...'
	@echo '#define VERSION "$(VERSION)"' > inc/version.h

clean:
	@rm -rf $(OUTDIR)/*
