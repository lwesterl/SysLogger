/*
 *    Author: Lauri Westerholm
 *
 *    Header file for pipes.c (part of the SysLogger program)
 */

#ifndef PIPES_HEADER
#define PIPES_HEADER


/*   C Library includes  */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "list.h"


/*  MACROS  */

/*  Permissions which the pipes are created with  */
#define PIPE_PERMISSIONS 0666
/*  Fifo open modes, read mode and write mode, stored as an unsigned char */
#define FIFO_READ 1
#define FIFO_WRITE 2
/* Max amount of bytes that can be written/read from fifo */
#define MAX_BYTES 200
/* Fifo name length ( /tmp/sysloggerxxxxx0, 0 == '\0' ) */
#define FIFO_NAME_LEN 20


/*  Function declarations */

void create_fifo(void);
int open_fifo(char *fifoname, unsigned char mode);
void concat_path(char *dest, const char *source);




#endif
