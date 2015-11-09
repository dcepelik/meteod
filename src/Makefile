.PHONY: clean

CC=gcc
CFLAGS=-c -Wall -std=gnu11 -DDEBUG `pkg-config --cflags hidapi-libusb librrd` -I. --pedantic
LDFLAGS=-Wall `pkg-config --libs hidapi-libusb librrd`
OBJS=common.o wmr200.o strbuf.o loggers/file.o loggers/rrd.o loggers/server.o
BINS=wmrd wmrq


all: ${BINS}

wmrd: wmrd.o ${OBJS}
	${CC} ${LDFLAGS} -o $@ $^

wmrq: wmrq.o
	${CC} ${LDFLAGS} -o $@ $^



wmr200.o: wmr200.c wmrdata.h common.h
	${CC} ${CFLAGS} -o $@ $<

loggers/file.o: loggers/file.c loggers/file.h wmrdata.h
	${CC} ${CFLAGS} -o $@ $<

loggers/rrd.o: loggers/rrd.c loggers/rrd.h wmrdata.h strbuf.h common.h
	${CC} ${CFLAGS} -o $@ $<



%.o: %.c %.h common.h
	${CC} ${CFLAGS} -o $@ $<

loggers/%.o: loggers/%.c loggers/%.h
	${CC} ${CFLAGS} -o $@ $< 



clean:
	rm -rf -- ${BINS} ${OBJS}
