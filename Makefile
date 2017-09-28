# 
#  Makefile for meteod
#  Copyright (c) David Čepelík <d@dcepelik.cz> 2017
#
#  Based on [1] and many other sources.
#
#  [1] http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# 

.SILENT:
.PHONY: dbg opt all clean

SRC_DIR = src
INC_DIR = $(SRC_DIR)/include

BUILD_DIR = build
DEPS_DIR = $(BUILD_DIR)/deps
DBG_DIR = $(BUILD_DIR)/dbg
OPT_DIR = $(BUILD_DIR)/opt

BINS = wmrd
SRCS = common.c \
	log.c \
	wmr200.c \
	strbuf.c \
	server.c \
	daemon.c \
	wmrd.c \
	logger-rrd.c

MAINS = $(patsubst %, %.c, $(BINS))

DEPS = $(addprefix $(DEPS_DIR)/, $(patsubst %.c, %.d, $(SRCS)))

DBG_BINS = $(addprefix $(DBG_DIR)/, $(BINS))
OPT_BINS = $(addprefix $(OPT_DIR)/, $(BINS))

DBG_OBJS = $(addprefix $(DBG_DIR)/, $(patsubst %.c, %.o, $(filter-out $(MAINS), $(SRCS))))
OPT_OBJS = $(addprefix $(OPT_DIR)/, $(patsubst %.c, %.o, $(filter-out $(MAINS), $(SRCS))))

CFLAGS += -c -std=gnu11 \
	`pkg-config --cflags hidapi-libusb librrd` \
	-Wall -Wextra -Werror --pedantic -Wno-unused-function \
		-Wno-gnu-statement-expression \
	-I $(INC_DIR)

DBG_CFLAGS += $(CFLAGS) -g -fsanitize=address
OPT_CFLAGS += $(CFLAGS) -O

LDFLAGS += -Wall \
	-lpthread -lm \
	`pkg-config --libs hidapi-libusb librrd`

DBG_LDFLAGS += $(LDFLAGS) -fsanitize=address
OPT_LDFLAGS += $(LDFLAGS)

#
#  This is the only non-trivial part of the Makefile. It auto-generates
#  dependencies for modules, which are stored in .d files within $(DEPS_DIR).
#
#  To do that, it uses whatever $(CC) is configured (at least GCC or clang
#  should work) to generate a dependency list, which is then further processed.
#
#  Initially, the contents of the file are in the form
#
#      object.o : src1.c src2.c ...
#
#  Which is changed to
#
#      object.o object.d : src1.c src2.c ...
#
#  So that the dependencies are re-generated whenever a object.o's dependency
#  changes.
#
#  Then each of the source files is added without dependenices as in:
#
#      src1.c src2.c ... :
#
#  This is to avoid some corner-cases when some of the source files are deleted.
#
$(DEPS_DIR)/%.d: $(SRC_DIR)/%.c Makefile
	echo DEPS $@
	$(CC) $(DBG_CFLAGS) -MM -MT $(DBG_DIR)/$*.o -o $@ $<
	sed 's_:_ $@:_' $@ > $@.tmp
	cp $@.tmp $@
	sed -e 's/ *\\$$//g' -e 's/ \+/ /g' -e 's/^[^:]*: //' -e 's/$$/:/' < $@.tmp >> $@
	rm $@.tmp

$(DBG_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS_DIR)/%.d Makefile
	echo "CC   $@"
	$(CC) $(DBG_CFLAGS) -o $@ $<

$(OPT_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS_DIR)/%.d Makefile
	echo "CC   $@"
	$(CC) $(OPT_CFLAGS) -o $@ $<

dbg: $(DBG_BINS)
opt: $(OPT_BINS)
all: dbg opt

clean:
	rm -f -- $(DEPS_DIR)/*.d $(DBG_DIR)/*.o $(DBG_BINS) $(OPT_DIR)/*.o $(OPT_BINS)

$(DBG_BINS): $(DBG_DIR)/%: $(DBG_OBJS) $(DBG_DIR)/%.o
	echo LINK $@
	$(CC) $(DBG_LDFLAGS) -o $@ $^

$(OPT_BINS): $(OPT_DIR)/%: $(OPT_OBJS) $(OPT_DIR)/%.o
	echo LINK $@
	$(CC) $(OPT_LDFLAGS) -o $@ $^

include $(DEPS)
