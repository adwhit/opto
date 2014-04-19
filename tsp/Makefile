all:
	gcc -fpic -Wall -c gene.c -o gene.o -O3 -std=c99
	gcc -shared -o libgene.so gene.o

bin:
	gcc -o gene -O3 -std=c99 gene.c

clean:
	rm -f gene.o gene libgene.so *.pyc

noopt:
	gcc -fpic -Wall -c gene.c -o gene.o -std=c99
	gcc -shared -o libgene.so gene.o

.PHONY: all bin clean
