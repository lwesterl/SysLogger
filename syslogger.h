/*
 *    Author: Lauri Westerholm
 *
 *    syslogger library, liblogger, header
 */



#ifndef SYSLOGGER_HEADER
#define SYSLOGGER_HEADER


/*    C Library includes    */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "pipes.h" /* MAX_BYTES macro defined there */

/*    Functions   */

int syslogger(const char *message);
char *get_prog_name(char *pid_str);
void parse_pid(char *pid_str);
char *parse_proc_name(const char *line);
int concat_message(char *dest, const char *pid_str, const char *name, const char *msg);


#endif
