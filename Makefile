#		Author: Lauri Westerholm
#		Makefile for SysLogger program (library)
#		Creates a static library named liblogger.a

CC = gcc
CFLAGS = -Wall -pedantic
Files = lib
O_FILES =
STATIC_F = ar rcs

default:	all

all:	$(Files)
	$(CC) $(CFLAGS) -static main.c -L. -l logger -o syslogger


lib:	$(O_FILES) logger_header.h
	$(STATIC_F) liblogger.a


clean:
	$(RM) *.o syslogger liblogger.a

clean-objects:
	$(RM) *.o
