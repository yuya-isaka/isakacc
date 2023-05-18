CFLAGS=-std=c11 -g -static

isakacc: isakacc.c

test: isakacc
	./test.sh

clean:
	rm -rf tmp* *.s *.o isakacc

.PHONY: test, clean