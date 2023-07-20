CFLAGS=-std=c11 -g -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:*.c=*.o)

isakacc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): header.h

test: isakacc
	./test.sh

clean:
	rm -f tmp* *.o *~ isakacc

.PHONY: test, clean