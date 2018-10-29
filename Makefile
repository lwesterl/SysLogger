#		Author: Lauri Westerholm
#		Makefile for SysLogger program (library)
#		Creates a static library named liblogger.a
#		pthreads used -> link with -lpthread
#		Creates a library test executable named as test_logger


CC = gcc
CFLAGS = -Wall -pedantic
Files = lib test_logger
O_FILES = threading.o
STATIC_F = ar rcs
LPTH = -lpthread

default:	all

all:	$(Files)
	$(CC) $(CFLAGS) -static create_daemon.c -L. -l logger $(LPTH) -o logger_daemon


lib:	$(O_FILES) logger_header.h
	$(STATIC_F) liblogger.a threading.o

test_logger:	logger_test.h
	$(CC) $(CFLAGS) logger_test.c -o test_logger

threading.o:	logger_header.h
	$(CC) $(CFLAGS) $(LPTH) -c threading.c

clean:
	$(RM) *.o logger_daemon test_logger liblogger.a

clean-objects:
	$(RM) *.o
