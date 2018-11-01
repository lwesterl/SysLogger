#		Author: Lauri Westerholm
#		Makefile for SysLogger program (library)
#		Creates a static library named liblogger.a
#		pthreads used -> link with -lpthread
#		Creates a library test executable named as test_logger


CC = gcc
CFLAGS = -Wall -pedantic
DFLAGS = -Wall -pedantic -g
Files = lib test_logger
O_FILES = threading.o pipes.o list.o signal_handler.o
STATIC_F = ar rcs
LPTH = -lpthread
DEBUG = list_debug pipe_debug thread_debug
EXE = logger_daemon test_logger thread_test list_test pipe_test


default:	all

all:	$(Files)
	$(CC) $(CFLAGS) -static create_daemon.c -L. -l logger $(LPTH) -o SysLogger


lib:	$(O_FILES) logger_header.h
	$(STATIC_F) liblogger.a ${O_FILES}

test_logger:	$(lib)
	$(CC) $(CFLAGS) logger_test.c -L. -l logger $(LPTH) -o test_logger

threading.o:	logger_header.h
	$(CC) $(CFLAGS) $(LPTH) -c threading.c

pipes.o:	pipes.h
	$(CC) $(CFLAGS) -c pipes.c

list.o:	list.h
	$(CC) $(CFLAGS) -c list.c

signal_handler.o: logger_header.h
	$(CC) $(CFLAGS) -c signal_handler.c

clean:
	$(RM) ${O_FILES} ${EXE} liblogger.a

clean-objects:
	$(RM) ${O_FILES}


#	DEBUG BUILDS

debug: $(DEBUG)

list_debug: list.h
	$(CC) $(DFLAGS) list.c list_test.c -o list_test

pipe_debug:	pipes.h
	$(CC) $(DFLAGS) pipes.c pipes_test.c threading.c signal_handler.c list.c $(LPTH) -o pipe_test

thread_debug: logger_test.h
	$(CC) $(DFLAGS) logger_test.c pipes.c list.c signal_handler.c threading.c  $(LPTH) -o thread_test

test_memory:	thread_test
	valgrind --leak-check=yes ./thread_test
