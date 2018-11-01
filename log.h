/*
 *    Author: Lauri Westerholm
 *
 *    Header for log.c
 */

#ifndef LOG_HEADER
#define LOG_HEADER


/*    C Libraries   */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/*    Macros    */

/*  The name of the standard log file */
#define LOG_FILE_NAME "/var/tmp/SysLogger.log"

/*  The name of the error log */
#define ERR_LOG_NAME "/var/tmp/Syslogger_error.log"


/*    Function declarations   */

int open_log(char *log_name);
void close_logs(int fd[2]);
void write_log_message(const char *message, int fd[2]);


#endif
