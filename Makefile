CC ?= gcc
CFLAGS =
LFLAGS =
DEBUG = 0
DEFINES = _GNU_SOURCE
BUILDDIR = build
INCDIRS = src/utils $(BUILDDIR)
PROJECT = ll
SRCDIR = src
VERSION = 1.1.0

WARN_FLAGS = -Wall -Wextra -pedantic
ifeq ($(CC),clang)
WARN_FLAGS += -Wno-gnu-zero-variadic-macro-arguments
endif

ifeq ($(DEBUG),1)
OPT_FLAGS = -g -Og
DEFINES += DEBUG
else
OPT_FLAGS = -O2
endif
# _GNU_SOURCE
CFLAGS += --std=gnu99
CFLAGS += $(OPT_FLAGS)
CFLAGS += $(WARN_FLAGS)
CFLAGS += $(addprefix -I, $(INCDIRS))
CFLAGS += $(addprefix -D, $(DEFINES))

all: bin asm
	@echo 'All targets is done...'

bin: $(BUILDDIR) version
	@echo 'Creating bin...DEBUG=$(DEBUG)'
	$(CC) $(CFLAGS) $(SRCDIR)/$(PROJECT).c -o $(BUILDDIR)/$(PROJECT) $(LFLAGS)

asm: $(BUILDDIR) version
	@echo 'Creating asm...'
	@$(CC) -S $(CFLAGS) $(SRCDIR)/$(PROJECT).c -o $(BUILDDIR)/$(PROJECT).s $(LFLAGS)

$(BUILDDIR):
	@echo 'Creating dir $(BUILDDIR)...'
	@mkdir $@

version:
	@echo 'Creating version...'
	@echo '#define VERSION "$(VERSION)"' > $(BUILDDIR)/version.h

clean:
	@rm -rf $(BUILDDIR)/*
