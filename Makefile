CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

aralc: $(OBJS)
		$(CC) -o aralc $(OBJS) $(LDFLAGS)

$(OBJS): aralc.h

test: aralc
		./test.sh

clean:
		rm -f aralc *.o *~ tmp*

.PHONY: test clean