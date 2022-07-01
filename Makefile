ARCH=x86
CC=gcc
CFLAGS=
DEBUG=0
INCDIRS=inc
LFLAGS=
OUTDIR=_out
PROJECT=ll
SRCDIR=src
VERSION=0.0.6


# подробное отображение предупреждений компилятора
override CFLAGS:=$(CFLAGS) -Wall

ifeq ($(ARCH), arm)
# папка с инструментами кросс-компилятора
CSL_TOOL_DIR=/opt/ARM-toolchain
GCC_ARM_PREF=$(CSL_TOOL_DIR)/bin/arm-none-linux-gnueabi-
# папка с исходниками ядра linux
EZSDK=/opt/EZSDK_5_05
LINUX_DEVKIT_DIR=$(EZSDK)/linux-devkit/arm-none-linux-gnueabi/usr
# используемый компилятор
override CC=$(GCC_ARM_PREF)gcc
# включение функций расширения GNU
override CFLAGS:=$(CFLAGS) -D _GNU_SOURCE
# архитектура цели сборки
override CFLAGS:=$(CFLAGS) -march=armv7-a
# центральный процессор цели сборки
override CFLAGS:=$(CFLAGS) -mcpu=cortex-a8
# математический сопроцессор цели сборки
override CFLAGS:=$(CFLAGS) -mfpu=neon
# генерарция кода с использованием аппаратных инструкций с плавающей точкой, используя соглашение о вызовах soft-float
override CFLAGS:=$(CFLAGS) -mfloat-abi=softfp
# отключение поиска системных заголовочных файлов
override CFLAGS:=$(CFLAGS) -nostdinc
# определение в директивах препроцессора архитектуры ARMs
override CFLAGS:=$(CFLAGS) -D ARM
# заголовочные файлы компилятора
override INCDIRS:=$(INCDIRS) $(CSL_TOOL_DIR)/lib/gcc/arm-none-linux-gnueabi/4.3.3/include
# общие заголовочные файлы С (EZSDK)
override INCDIRS:=$(INCDIRS) $(LINUX_DEVKIT_DIR)/include
# специальные заголовочные файлы (EZSDK)
override INCDIRS:=$(INCDIRS) $(LINUX_DEVKIT_DIR)/src/linux-2.6.37-psp04.04.00.01-headers/include
# трассировка программы?
override LFLAGS:=$(LFLAGS) -rdynamic
# компоновка с POSIX библиотекой pthread
override LFLAGS:=$(LFLAGS) -pthread
# компоновка с библиотекой librt (работа с таймерами)
override LFLAGS:=$(LFLAGS) -l rt
endif

ifeq ($(DEBUG), 1)
override CFLAGS :=-D DEBUG $(CFLAGS)
endif

# добавление списка заголовочных файлов в список опции компилятора
INC=$(addprefix -I , $(INCDIRS))
override CFLAGS:=$(CFLAGS) $(INC)


all: bin asm
	@echo 'All targets is done...'

bin: $(OUTDIR) version
	@echo 'Creating bin...DEBUG=$(DEBUG), ARCH=$(ARCH)'
	$(CC) $(CFLAGS) $(SRCDIR)/$(PROJECT).c -o $(OUTDIR)/$(PROJECT)_$(ARCH) $(LFLAGS)

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
