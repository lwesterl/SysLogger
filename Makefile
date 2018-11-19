#		Author: Lauri Westerholm
#		Makefile for SysLogger program
#		Creates a static library named liblogger.a
#		pthreads used -> link with -lpthread

# 	Creates two main executables: SysLogger and Stress_tester


CC = gcc
CFLAGS = -Wall -pedantic
DFLAGS = -Wall -pedantic -g
SRC = src/
TEST = test/
PRIMARY = lib Stress_tester
O_FILES = threading.o pipes.o list.o signal_handler.o log.o syslogger.o
STATIC_F = ar rcs
LPTH = -lpthread
LINKER = -L. -llogger -lpthread
EXE = SysLogger Stress_tester
DEXE = thread_test list_test pipe_test log_test stress_debug
VALG = valgrind --leak-check=yes

default:	all

# Normal files

all: $(O_FILES) $(PRIMARY)
	$(CC) $(CFLAGS) -static $(SRC)create_daemon.c $(LINKER) -o SysLogger

lib:	$(O_FILES)
	 $(STATIC_F) liblogger.a ${O_FILES}

Stress_tester: $(lib)
	$(CC) $(CFLAGS) -static $(SRC)stress_test.c $(LINKER) -o Stress_tester

threading.o:
	$(CC) $(CFLAGS) -c $(SRC)threading.c

pipes.o:
	$(CC) $(CFLAGS) -c $(SRC)pipes.c

list.o:
	$(CC) $(CFLAGS) -c $(SRC)list.c

signal_handler.o:
	$(CC) $(CFLAGS) -c $(SRC)signal_handler.c

log.o:
	$(CC) $(CFLAGS) -c $(SRC)log.c

syslogger.o:
	$(CC) $(CFLAGS) -c $(SRC)syslogger.c

# Debug files

debug:	$(DEXE) $(stress_debug)

thread_test: $(O_FILES)
	$(CC) $(DFLAGS) ${O_FILES} $(TEST)logger_test.c $(LPTH) -o thread_test

list_test:	$(list.o)
	$(CC) $(DFLAGS) list.o $(TEST)list_test.c -o list_test

pipe_test:	$(pipe.o)
	$(CC) $(DFLAGS) pipes.o $(TEST)pipes_test.c -o pipe_test

log_test:	$(log.o)
	$(CC) $(DFLAGS) log.o $(TEST)log_test.c -o log_test

stress_debug:
	$(CC) $(DFLAGS) $(SRC)syslogger.c $(SRC)pipes.c $(SRC)stress_test.c $(LPTH) -o stress_debug

# Clean

clean:
	$(RM) ${EXE} ${DEXE} ${O_FILES} liblogger.a

clean-debug:
	$(RM) ${DEXE}

clean-objects:
	$(RM) ${O_FILES}

clean-lib:
	$(RM) liblogger.a


# Valgrind commands

test_memory:	thread_test
	$(VALG) ./thread_test

test_stress_test: stress_debug
	$(VALG) ./stress_debug mem_test
