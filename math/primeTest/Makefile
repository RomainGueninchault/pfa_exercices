prepare: test.o

test:
	gcc -Wall -Werror -c skeleton.c -o skeleton.o
	gcc test.o skeleton.o -o binaire_test
	./binaire_test

test.o: test.c
	gcc -Wall -Werror -c test.c -o test.o

clean:
	rm -f *.o binaire_test
