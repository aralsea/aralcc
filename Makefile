CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

aralcc: $(OBJS)
		$(CC) -o aralcc $(OBJS) $(LDFLAGS)

$(OBJS): aralcc.h

test: aralcc
		./test.sh

clean:
		rm -f aralcc *.o *~ tmp*

.PHONY: test clean