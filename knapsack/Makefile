all:
	gcc -fpic -Wall -c knap.c -o knap.o -O3 -std=c99
	gcc -shared -o libknap.so knap.o

bin:
	gcc -o knap -O3 -std=c99 knap.c

clean:
	rm -f knap.o knap libknap.so *.pyc

.PHONY: all bin clean
