.PHONY: clean

CC=gcc
CFLAGS=-c -Wall -std=gnu11 -DDEBUG `pkg-config --cflags hidapi-libusb librrd` -I.
LDFLAGS=-Wall `pkg-config --libs hidapi-libusb librrd`
OBJS=common.o wmr200.o wmrd.o strbuf.o loggers/file.o loggers/rrd.o loggers/server.o
PROG=wmrd

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} $^

%.o: %.c %.h
	${CC} ${CFLAGS} -o $@ $<

loggers/%.o: loggers/%.c loggers/%.h
	${CC} ${CFLAGS} -o $@ $< 

clean:
	rm -rf -- ${PROG} *.o loggers/*.o
