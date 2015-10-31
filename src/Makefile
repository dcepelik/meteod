.PHONY: clean

CC=gcc
CFLAGS=-c -Wall -std=gnu11 `pkg-config --cflags hidapi-libusb librrd` -DDEBUG
LDFLAGS=-Wall `pkg-config --libs hidapi-libusb librrd`
PROG=wmrd
OBJS=common.o wmr200.o wmrd.o file_logger.o rrd_logger.o strbuf.o

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} $^

%.o: %.c %.h
	${CC} ${CFLAGS} -o $@ $<


clean:
	rm -rf -- ${PROG} *.o
