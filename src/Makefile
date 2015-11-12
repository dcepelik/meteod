.PHONY: clean

CC=gcc

CFLAGS=-c \
	-std=gnu11 \
	`pkg-config --cflags hidapi-libusb librrd` \
	-Wall --pedantic -g \
	-Wno-unused-function \
	-I.

LDFLAGS=`pkg-config --libs hidapi-libusb librrd` \
	-lpthread \
	-Wall

OBJS=common.o wmr200.o strbuf.o serialize.o daemon.o \
	loggers/file.o loggers/rrd.o loggers/server.o

BINS=wmrd wmrq


all: ${BINS}

wmrd: wmrd.o ${OBJS}
	${CC} ${LDFLAGS} -o $@ $^

wmrq: wmrq.o common.o serialize.o loggers/file.o strbuf.o
	${CC} ${LDFLAGS} -o $@ $^



wmr200.o: wmr200.c wmrdata.h common.h
	${CC} ${CFLAGS} -o $@ $<

serialize.o: serialize.c serialize.h wmrdata.h strbuf.h array.h common.h
	${CC} ${CFLAGS} -o $@ $<

loggers/server.o: loggers/server.c loggers/server.h array.h common.h
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
	rm -rf -- ${BINS} ${OBJS} wmrq.o wmrd.o
