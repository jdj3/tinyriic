full: full_as.o tr_as.o parser.o bedlisp.o repl.o
	gcc -o $@ $^

micro: micro_as.o tr_as.o parser.o bedlisp.o repl.o
	gcc -o $@ $^

tracer: micro_as.o tr_as.o parser.o bedlisp.o tracer.o
	gcc -o $@ $^

clean:
	rm -f *.o full micro tracer
