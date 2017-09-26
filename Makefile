.PHONY: all clean

SRC_DIR = src
INC_DIR = $(SRC_DIR)/include

BUILD_DIR = build
OBJS_DIR = $(BUILD_DIR)/objs
DEPS_DIR = $(BUILD_DIR)/deps

BINS = $(BUILD_DIR)/wmrd
SRCS = common.c \
	log.c \
	wmr200.c \
	strbuf.c \
	daemon.c \
	wmrdata.c \
	logger-rrd.c

MAINS = $(OBJS_DIR)/wmrd.o $(OBJS_DIR)/wmrq.o
OBJS = $(filter-out $(MAINS), $(addprefix $(OBJS_DIR)/, $(patsubst %.c, %.o, $(SRCS))))
DEPS = $(addprefix $(DEPS_DIR)/, $(patsubst %.c, %.d, $(SRCS)))

CFLAGS += -c -std=gnu11 \
	`pkg-config --cflags hidapi-libusb librrd` \
	-Wall -Wextra -Werror --pedantic -Wno-unused-function \
	-I $(INC_DIR) \
	-g

LDFLAGS += -Wall \
	-lpthread -lm \
	`pkg-config --libs hidapi-libusb librrd`

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS_DIR)/%.d Makefile
	$(CC) $(CFLAGS) -o $@ $<

$(DEPS_DIR)/%.d: $(SRC_DIR)/%.c Makefile
	$(CC) $(CFLAGS) -MM -MT $(OBJS_DIR)/$*.o -o $@ $<; sed 's_:_ $@:_' $@ > $@.tmp; mv $@.tmp $@

all: $(BINS)

clean:
	rm -f -- $(OBJS_DIR)/*.o $(DEPS_DIR)/*.d $(BINS)

$(BUILD_DIR)/wmrd: $(OBJS) $(OBJS_DIR)/wmrd.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/wmrq: $(OBJS) $(OBJS_DIR)/wmrq.o
	$(CC) $(LDFLAGS) -o $@ $^

include $(DEPS)
