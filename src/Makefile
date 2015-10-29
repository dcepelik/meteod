.PHONY: clean

CC=gcc
CFLAGS=-c -Wall -std=c11 `pkg-config --cflags hidapi-hidraw`
LDFLAGS=-Wall `pkg-config --libs hidapi-hidraw`
PROG=logger
OBJS=logger.o

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} $^

%.o: %.c
	${CC} ${CFLAGS} -o $@ $<

clean:
	rm -rf -- ${PROG} *.o
