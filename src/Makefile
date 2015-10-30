.PHONY: clean

CC=gcc
CFLAGS=-c -Wall -std=gnu11 `pkg-config --cflags hidapi-libusb` -DDEBUG
LDFLAGS=-Wall `pkg-config --libs hidapi-libusb`
PROG=wmrd
OBJS=common.o wmr200.o wmrd.o stdout_handler.o

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} $^

%.o: %.c %.h
	${CC} ${CFLAGS} -o $@ $<


clean:
	rm -rf -- ${PROG} *.o
