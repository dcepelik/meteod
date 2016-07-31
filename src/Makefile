.PHONY: clean

CFLAGS+=-c \
	-std=gnu11 \
	-Wall -Wextra -Werror --pedantic -ggdb3 \
	-Wno-unused-function \
	`pkg-config --cflags hidapi-libusb librrd` \
	-I.

LDFLAGS+=`pkg-config --libs hidapi-libusb librrd` \
	-lpthread \
	-lm \
	-Wall

OBJS=common.o log.o wmr200.o strbuf.o serialize.o daemon.o server.o wmrdata.o \
	loggers/yaml.o loggers/rrd.o loggers/server.o

BINS=wmrd wmrq


all: $(BINS)

wmrd: wmrd.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

wmrq: wmrq.o common.o serialize.o loggers/yaml.o strbuf.o wmrdata.o
	$(CC) $(LDFLAGS) -o $@ $^


wmr200.o: wmr200.c wmrdata.h common.h
	$(CC) $(CFLAGS) -o $@ $<

serialize.o: serialize.c serialize.h wmrdata.h strbuf.h array.h common.h
	$(CC) $(CFLAGS) -o $@ $<

server.o: server.c array.h common.h serialize.h
	$(CC) $(CFLAGS) -o $@ $<

loggers/rrd.o: loggers/rrd.c loggers/rrd.h wmrdata.h strbuf.h common.h
	$(CC) $(CFLAGS) -o $@ $<



%.o: %.c %.h common.h
	$(CC) $(CFLAGS) -o $@ $<

loggers/%.o: loggers/%.c loggers/%.h
	$(CC) $(CFLAGS) -o $@ $< 



clean:
	rm -rf -- $(BINS) $(OBJS) wmrq.o wmrd.o

install: $(BINS)
	sudo install $(BINS) /usr/bin
