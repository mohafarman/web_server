BUILD_DIR := ./build
SRC_DIRS := ./src

PROGRAM=server
CC=gcc
CFLAGS=-Wall -ggdb -Wextra -pedantic -std=gnu11
LDLIBS=#-lmenu -lncurses -lform -lpthread
COMPILE=$(CC) $(CFLAGS) $(LDLIBS)

PROGRAM: $(PROGRAM).c
	#@mkdir ../build
	$(COMPILE) $(CFLAGS) $(SRC_DIRS)/$(PROGRAM).c -o ./build/$(PROGRAM)
	@./build/$(PROGRAM)

$(PROGRAM).c:

# Run with -v, verbose, flag
v: $(PROGRAM).c
	#@mkdir ../build
	$(COMPILE) $(CFLAGS) $(SRC_DIRS)/$(PROGRAM).c -o ./build/$(PROGRAM)
	@./build/$(PROGRAM) -v
