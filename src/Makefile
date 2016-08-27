ARCH ?= $(shell uname -m)
TR_AS ?= micro

ARCH_PRE :=
ARCH_FLAGS :=

ifeq ($(ARCH),i386)
	ARCH_FLAGS := -m32
else ifeq ($(ARCH),arm)
	ARCH_PRE := arm-linux-gnueabi-
else ifeq ($(ARCH),mips)
	ARCH_PRE := mips-linux-gnu-
endif

CC := $(ARCH_PRE)gcc $(ARCH_FLAGS)

CFLAGS := -nostdlib -nostdinc -fno-asynchronous-unwind-tables \
	-Os -Wl,-s -I. -I$(ARCH) -DTR_ARCH_$(arch) -DTR_AS_$(TR_AS)

HEADERS := $(ARCH)/*.h *.h

ARCH_FILES := start syslib $(TR_AS)_as
AS_FILES := tr_as parser bl_tail
PROGS := repl tracer

ARCH_OBJS := $(foreach f,$(ARCH_FILES),build/$(f).$(ARCH).o)
AS_OBJS := $(foreach f,$(AS_FILES),build/$(f).$(TR_AS).$(ARCH).o)
PROG_OBJS := $(foreach f,$(PROGS),build/$(f).$(TR_AS).$(ARCH).o)

BASE_OBJS = $(ARCH_OBJS) $(AS_OBJS)

all: $(PROGS)

$(PROGS): %: bin/%_$(TR_AS)_$(ARCH)

.PHONY: clean distclean $(PROGS)
.SECONDARY: $(BASE_OBJS) $(PROG_OBJS)

bin/%_$(TR_AS)_$(ARCH): build/%.$(TR_AS).$(ARCH).o $(BASE_OBJS) $(HEADERS)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(BASE_OBJS) $<

build/start.$(ARCH).o: $(ARCH)/start.S
	$(CC) -c $(CFLAGS) -o $@ $<

build/%.$(TR_AS).$(ARCH).o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/%.$(ARCH).o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f build/*.$(ARCH).o
	rm -f bin/*_$(ARCH)

distclean:
	rm -f build/*
	rm -f bin/*
