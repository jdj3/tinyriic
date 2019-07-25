ARCH ?= $(shell uname -m)
TR_AS ?= micro

ARCH_PRE :=
ARCH_FLAGS :=
ARCH_DIR := $(ARCH)

ifeq ($(ARCH),i386)
	ARCH_FLAGS := -m32
else ifeq ($(ARCH),arm)
	ARCH_PRE := arm-linux-gnueabi-
else ifeq ($(ARCH),mips)
	ARCH_PRE := mips-linux-gnu-
	ARCH_FLAGS := -mips1 -msoft-float -mgp32 -mfp32
else ifeq ($(ARCH),mipsel)
	ARCH_PRE := mipsel-linux-gnu-
	ARCH_DIR := mips
	ARCH_FLAGS := -mips1 -msoft-float -mgp32 -mfp32
endif

CC := $(ARCH_PRE)gcc $(ARCH_FLAGS)

CFLAGS := -nostdlib -nostdinc -fno-asynchronous-unwind-tables \
	-Os -Wl,-s -I. -I$(ARCH_DIR) -DTR_ARCH_$(ARCH_DIR) -DTR_AS_$(TR_AS)

HEADERS := $(ARCH_DIR)/*.h *.h

SRCS := syslib $(TR_AS)_as tr_as parser bl_tail
PROGS := repl tracer

BASE_OBJS := build/start.$(ARCH).o \
	$(foreach f,$(SRCS),build/$(f).$(TR_AS).$(ARCH).o)
PROG_OBJS := $(foreach f,$(PROGS),build/$(f).$(TR_AS).$(ARCH).o)

all: $(PROGS)

$(PROGS): %: bin/%_$(TR_AS)_$(ARCH)

.PHONY: clean distclean $(PROGS)
.SECONDARY: $(BASE_OBJS) $(PROG_OBJS)

bin/%_$(TR_AS)_$(ARCH): build/%.$(TR_AS).$(ARCH).o $(BASE_OBJS) $(HEADERS)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(BASE_OBJS) $<

build/start.$(ARCH).o: $(ARCH_DIR)/start.S
	mkdir -p build
	$(CC) -c $(CFLAGS) -o $@ $<

build/%.$(TR_AS).$(ARCH).o: %.c $(HEADERS)
	mkdir -p build
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f build/*.$(ARCH).o
	rm -f bin/*_$(ARCH)

distclean:
	rm -f build/*
	rm -f bin/*
