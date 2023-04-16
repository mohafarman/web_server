CC = gcc
CFLAGS += -Wall -ggdb -Wextra -pedantic -std=c11
LDLIBS := -lm

SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
HDRS = $(wildcard $(INCDIR)/*.h)
TARGET = $(BINDIR)/server

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HDRS)
	mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*.o $(TARGET)

.PHONY: all clean

# BUILD_DIR := ./build
# SRC_DIRS := ./src
# HEADER_DIRS := ./include
#
# CC = gcc
# CFLAGS=-Wall -ggdb -Wextra -pedantic -std=c11 -lm
# LIBS = -lm
# PORT = 4001
#
# SRCS = $(SRC_DIRS)/server.c $(SRC_DIRS)/utils.c
# OBJS = $(SRCS:.c=.o)
# HEADERS = $(HEADER_DIRS)/server.h $(HEADER_DIRS)/utils.h
#
# PROG = server
#
# all: $(PROG)
#
# $(PROG): $(OBJS)
# 	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(PROG) $(OBJS) $(LIBS)
# 	@./$(BUILD_DIR)/$(PROG) $(PORT)
#
# $(OBJS): %.o: %.c $(HEADERS)
# 	$(CC) $(CFLAGS) -c $< -o $@
#
# clean:
# 	rm -f $(PROG) $(OBJS)
