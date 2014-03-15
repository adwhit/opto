all:
	gcc -fpic -Wall -c graph.c -o graph.o -O3 -std=c99
	gcc -shared -o libgraph.so graph.o

bin:
	gcc -o graph -O3 -std=c99 graph.c

clean:
	rm -f graph.o graph libgraph.so *.pyc

noopt:
	gcc -fpic -Wall -c graph.c -o graph.o -std=c99
	gcc -shared -o libgraph.so graph.o

.PHONY: all bin clean
