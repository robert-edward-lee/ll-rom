PROJECT = ll
VERSION = 1.1.2
CC = gcc
CFLAGS = -c
LFLAGS =
DEBUG = 0
DEFINES = _GNU_SOURCE
BUILDDIR = build
SRCDIRS = src $(wildcard src/*/)
INCDIRS = $(SRCDIRS) $(BUILDDIR)

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

DEPEND_FLAGS = -MMD -MP

CFLAGS += --std=gnu99
CFLAGS += $(OPT_FLAGS)
CFLAGS += $(WARN_FLAGS)
CFLAGS += $(DEPEND_FLAGS)
CFLAGS += $(addprefix -I, $(INCDIRS))
CFLAGS += $(addprefix -D, $(DEFINES))

VPATH = $(subst $() $(),:,$(sort $(INCDIRS) $(SRCDIRS) $(BUILDDIR)))
SOURCES = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJECTS = $(addprefix $(BUILDDIR)/, $(notdir $(patsubst %.c,%.o,$(SOURCES))))
DEPENDS = $(patsubst %.o,%.d,$(OBJECTS))
-include $(DEPENDS)

.DEFAULT_GOAL := $(PROJECT)

$(PROJECT): $(BUILDDIR) version $(OBJECTS)
	@$(CC) $(OBJECTS) -o $(BUILDDIR)/$@ $(LFLAGS)
	@echo '  LD      ' $(BUILDDIR)/$@

$(BUILDDIR)/%.o: %.c
	@$(CC) $(CFLAGS) $< -o $@
	@echo '  CC      ' $@

$(BUILDDIR):
	@echo 'Creating dir $(BUILDDIR)...'
	@mkdir -p $@

version:
	@echo 'Creating version...'
	@echo '#define VERSION "$(VERSION)"' > $(BUILDDIR)/version.h

clean:
	@rm -rf $(BUILDDIR)/*

format:
	@clang-format \
		-style=file:$(CURDIR)/.clang-format \
		-i $(wildcard $(CURDIR)/src/*.c) $(wildcard $(CURDIR)/src/*/*.h)
