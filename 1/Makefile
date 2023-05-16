CFLAGS=-std=c11 -g -static

test: isakacc
	./test.sh

isakacc: isakacc.c

clean:
	rm -f isakacc *.o *~ tmp*

.PHONY: test clean