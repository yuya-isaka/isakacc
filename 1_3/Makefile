CFLGAS=-std=c11 -g -fno-common

isakacc: isakacc.o
	$(CC) -o isakacc isakacc.o $(LDFLAGS)

test: isakacc
	./test.sh

clean:
	rm -f isakacc *~ tmp* *.o

.PHONY: test, clean