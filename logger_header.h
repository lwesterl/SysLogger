/*
 *    Author: Lauri Westerholm
 *    Header file for the SysLogger program
 */

#ifndef LOGGER_HEADER
#define LOGGER_HEADER


/*  C libraries */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include "list.h"
#include "pipes.h"


/*  Structs */


/*  Function declarations */

void main_thread(void);
int create_thread(const char *pipe_name, pthread_t *threads);
void *blocker_thread(void *ptr);
void list_files(list_t *list);



#endif
