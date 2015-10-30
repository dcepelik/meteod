.PHONY: clean

CC=gcc
CFLAGS=-c -Wall -std=c11 `pkg-config --cflags hidapi-libusb` -DDEBUG -g
LDFLAGS=-Wall `pkg-config --libs hidapi-libusb`
PROG=wmrd
OBJS=wmrd.o wmr200.o

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} $^

%.o: %.c
	${CC} ${CFLAGS} -o $@ $<

clean:
	rm -rf -- ${PROG} *.o
