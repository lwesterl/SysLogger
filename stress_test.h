/*
 *    Author: Lauri Westerholm
 *
 *    Header for stress_test.c
 */


#ifndef STRESS_TEST_HEADER
#define STRESS_TEST_HEADER

/*    C Library includes    */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include "syslogger.h"   /* syslogger defined there */

/*    Macros    */

/*  How many forks should be done, max 1 - 99 */
#define FORKS_TARGET 10
/* How many threads each fork should start, 1 - 99 */
#define THREADS_TARGET 50

/*    Function declarations   */

void child_process(int count);
void *thread_function(void *buffer);

#endif
