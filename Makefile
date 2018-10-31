#		Author: Lauri Westerholm
#		Makefile for SysLogger program (library)
#		Creates a static library named liblogger.a
#		pthreads used -> link with -lpthread
#		Creates a library test executable named as test_logger


CC = gcc
CFLAGS = -Wall -pedantic
DFLAGS = -Wall -pedantic -g
Files = lib test_logger
O_FILES = threading.o pipes.o list.o
STATIC_F = ar rcs
LPTH = -lpthread
DEBUG = list_debug pipe_debug

default:	all

all:	$(Files)
	$(CC) $(CFLAGS) -static create_daemon.c -L. -l logger $(LPTH) -o logger_daemon


lib:	$(O_FILES) logger_header.h
	$(STATIC_F) liblogger.a threading.o pipes.o list.o

test_logger:	$(lib)
	$(CC) $(CFLAGS) logger_test.c -L. -l logger $(LPTH) -o test_logger

threading.o:	logger_header.h
	$(CC) $(CFLAGS) $(LPTH) -c threading.c

pipes.o:	pipes.h
	$(CC) $(CFLAGS) -c pipes.c

list.o:	list.h
	$(CC) $(CFLAGS) -c list.c

clean:
	$(RM) *.o logger_daemon test_logger pipe_test list_test liblogger.a

clean-objects:
	$(RM) *.o


#	DEBUG BUILDS

debug: $(DEBUG)

list_debug: list.h
	$(CC) $(DFLAGS) list.c list_test.c -o list_test

pipe_debug:	pipes.h
	$(CC) $(DFLAGS) pipes.c pipes_test.c -o pipe_test
