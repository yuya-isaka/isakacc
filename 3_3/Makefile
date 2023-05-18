CFLAGS=-std=c11 -g -fno-common

isakacc: main.o
	$(CC) -o isakacc main.o $(LDFLAGS)

test: isakacc
	./test.sh

clean:
	rm -f *~ tmp* *.s isakacc

.PHONY: test, clean