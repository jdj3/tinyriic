ARCH ?= $(shell uname -m)
TR_AS ?= micro

CFLAGS := -nostdlib -nostdinc -fno-asynchronous-unwind-tables \
	-Os -Wl,-s -I. -I$(ARCH)

HEADERS := $(ARCH)/*.h *.h

ARCH_FILES := start syslib $(TR_AS)_as
AS_FILES := tr_as parser bl_tail
PROGS := repl tracer

ARCH_OBJS := $(foreach f,$(ARCH_FILES),build/$(f).$(ARCH).o)
AS_OBJS := $(foreach f,$(AS_FILES),build/$(f).$(TR_AS).$(ARCH).o)
PROG_OBJS := $(foreach f,$(PROGS),build/$(f).$(TR_AS).$(ARCH).o)

BASE_OBJS = $(ARCH_OBJS) $(AS_OBJS)

all: $(PROGS)

$(PROGS): %: bin/$(TR_AS)_%_$(ARCH)

.PHONY: clean distclean $(PROGS)
.SECONDARY: $(BASE_OBJS) $(PROG_OBJS)

bin/$(TR_AS)_%_$(ARCH): build/%.$(TR_AS).$(ARCH).o $(BASE_OBJS) $(HEADERS)
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
