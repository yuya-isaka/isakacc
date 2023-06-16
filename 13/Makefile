CFLGAS=-std=c11 -g -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:*.c=*.o)

isakacc: $(OBJS)
	$(CC) $(CFLGAS) -o $@ $^ $(LDFLAGS)

$(OBJS): header.h

test: isakacc
	./test.sh

clean:
	rm -f isakacc tmp* *.o *~

.PHONY: test, clean