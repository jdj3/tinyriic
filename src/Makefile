CFLAGS := -nostdlib -nostdinc -fno-asynchronous-unwind-tables -Os -Wl,-s

micro: start.o syslib.o micro_as.o tr_as.o parser.o bl_tail.o repl.o
	gcc $(CFLAGS) -o $@ $^

tracer: start.o syslib.o micro_as.o tr_as.o parser.o bl_tail.o tracer.o
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f *.o micro tracer
