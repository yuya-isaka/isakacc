CFLAGS=-std=c11 -g -static

isakacc: isakacc.c

test: isakacc
	./test.sh

clean:
	rm -f tmp* *.o *~ isakacc

.PHONY: test, clean