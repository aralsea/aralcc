CFLAGS=-std=c11 -g -static

9cc: aralc.c

test: aralc
		./test.sh

clean:
		rm -f aralc *.o *~ tmp*

.PHONY: test clean