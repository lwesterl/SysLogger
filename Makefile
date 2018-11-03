#		Author: Lauri Westerholm
#		Makefile for SysLogger program (library)
#		Creates a static library named liblogger.a
#		pthreads used -> link with -lpthread
#		Creates a library test executable named as test_logger


CC = gcc
CFLAGS = -Wall -pedantic
DFLAGS = -Wall -pedantic -g
Files = lib stress_tester
O_FILES = threading.o pipes.o list.o signal_handler.o log.o syslogger.o
STATIC_F = ar rcs
LPTH = -lpthread
DEBUG = list_debug pipe_debug thread_debug log_debug test_logger stress_debug
EXE = test_logger thread_test list_test pipe_test SysLogger log_test Stress_tester stress_debug
DEXE = test_logger thread_test list_test pipe_test log_test stress_debug
VALG = valgrind --leak-check=yes

default:	all

all:	$(Files)
	$(CC) $(CFLAGS) -static create_daemon.c -L. -llogger $(LPTH) -o SysLogger

lib:	$(O_FILES) logger_header.h
	$(STATIC_F) liblogger.a ${O_FILES}

stress_tester: $(lib) stress_test.h
	$(CC) $(CFLAGS) -static stress_test.c -L. -llogger $(LPTH) -o Stress_tester

threading.o:	logger_header.h
	$(CC) $(CFLAGS) $(LPTH) -c threading.c

pipes.o:	pipes.h
	$(CC) $(CFLAGS) -c pipes.c

list.o:	list.h
	$(CC) $(CFLAGS) -c list.c

signal_handler.o: logger_header.h
	$(CC) $(CFLAGS) -c signal_handler.c

log.o: log.h
	$(CC) $(CFLAGS) -c log.c

syslogger.o: syslogger.h
	$(CC) $(CFLAGS) -c syslogger.c

#		CLEAN

clean:
	$(RM) ${O_FILES} ${EXE} liblogger.a

clean-objects:
	$(RM) ${O_FILES}

clean-debug:
	$(RM) ${DEXE}


#		DEBUG BUILDS

debug: $(DEBUG)

test_logger: lib
	$(CC) $(CFLAGS) logger_test.c -L. -llogger $(LPTH) -o test_logger

list_debug: list.h
	$(CC) $(DFLAGS) list.c list_test.c -o list_test

pipe_debug:	pipes.h
	$(CC) $(DFLAGS) pipes.c pipes_test.c threading.c signal_handler.c list.c log.c $(LPTH) -o pipe_test

thread_debug: logger_test.h
	$(CC) $(DFLAGS) logger_test.c pipes.c list.c signal_handler.c log.c threading.c  $(LPTH) -o thread_test

log_debug: log.h
	$(CC) $(DFLAGS) log_test.c log.c -o log_test

stress_debug: stress_test.h
	$(CC) $(DFLAGS) stress_test.c syslogger.c pipes.c  $(LPTH) -o stress_debug


test_memory:	thread_test
	$(VALG) ./thread_test

test_stress_test: stress_debug
	$(VALG) ./stress_debug
