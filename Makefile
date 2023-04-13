#BUILD_DIR := ./build
#SRC_DIRS := ./src
#
#PROGRAM=server
#CC=gcc
#CFLAGS=-Wall -ggdb -Wextra -pedantic -std=gnu11
#LDLIBS=#-lmenu -lncurses -lform -lpthread
#HEADER= -I$(SRC_DIRS)/utils.h
#COMPILE=$(CC) $(CFLAGS) $(LDLIBS) $(HEADER)
#
#PROGRAM: $(PROGRAM).c
#	#@mkdir ../build
#	$(COMPILE) $(SRC_DIRS)/$(PROGRAM).c -o ./build/$(PROGRAM)
#	@./build/$(PROGRAM)
#
#$(PROGRAM).c:
#
## Run with -v, verbose, flag
#v: $(PROGRAM).c
#	#@mkdir ../build
#	$(COMPILE) $(CFLAGS) $(SRC_DIRS)/$(PROGRAM).c -o ./build/$(PROGRAM)
#	@./build/$(PROGRAM) -v
#


BUILD_DIR := ./build
SRC_DIRS := ./src
HEADER_DIRS := ./include

CC = gcc
CFLAGS=-Wall -ggdb -Wextra -pedantic -std=c11
LIBS = -lm
PORT = 4001

SRCS = $(SRC_DIRS)/server.c $(SRC_DIRS)/utils.c
OBJS = $(SRCS:.c=.o)
HEADERS = $(HEADER_DIRS)/server.h $(HEADER_DIRS)/utils.h

PROG = server

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(PROG) $(OBJS) $(LIBS)
	@./$(BUILD_DIR)/$(PROG) $(PORT)

$(OBJS): %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROG) $(OBJS)
