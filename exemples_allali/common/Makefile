.PHONY=prepare clean

prepare: test.o

clean:
	rm test.o

test.o: test.c
	gcc -c test.c
	strip --strip-unneeded test.o


