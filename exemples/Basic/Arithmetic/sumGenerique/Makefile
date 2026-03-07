CC = gcc
CFLAGS = -g -Wall -Wextra

sumClone: testSum.o
	rm -f testSum.c

testSum.o: testSum.c
	$(CC) -Wall -Wextra -c testSum.c -o testSum.o

sumValidate:
	$(CC) $(CFLAGS) sum.c testSum.o -o testSum
	./testSum

.PHONY: sumClone sumValidate clean
