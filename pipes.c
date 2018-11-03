/*
 *    Author: Lauri Westerholm
 *
 *    File contains functions related to creating and accessing pipes
 *    which the SysLogger program uses
 *
 *    Notice: The created public pipes are named as syslogger+pid() (pid of the
 *    program which created the pipe). The pipes are created to /tpm
 */

#include "pipes.h"


/*
 *    Opens the fifo which names is given as argument
 *    Opens in non-blocking mode -> should always succeed
 *    (even if the fifo isn't opened for writing)
 */

int open_fifo_read(char *fifoname)
{
  int fd = -2;
  fd = open(fifoname, O_RDONLY | O_NONBLOCK);
  return fd;

}


/*
 *    Creates a fifo for syslogger() - call
 *    Arguments: process pid in str format
 *    Returns the fifo name
 *
 *    Created fifo is concat of /tmp/syslogger + pid + n + tid
 *    The char n and tid is added to make syslogger function
 *    possible to call from multiple threads at 'same time'
 */

 char *syslogger_fifo(char *pid_str)
 {
   /* Get the thread id */
   pid_t tid = syscall(SYS_gettid);
   /* Also tid is max 5 numbers / chars */
   char tid_str[6] = "";
   snprintf(tid_str, 6, "%d", tid);

   /* Concat the strings */
   const char syslog[] = "/tmp/syslogger";
   int len = strlen(syslog) + strlen(pid_str) + strlen(tid_str) +2;
   char *fifo = malloc(len * sizeof(char));

   strncpy(fifo, syslog, strlen(syslog) + 1);
   strncat(fifo, pid_str, strlen(pid_str));
   /* Add the char n and '\0' */
   int index = strlen(syslog) + strlen(pid_str);
   fifo[index] = 'n';
   fifo[index + 1] = '\0';
   strncat(fifo, tid_str, strlen(tid_str));

   /*  Create the fifo */
   mkfifo(fifo, PIPE_PERMISSIONS);

   /* Return fifo name string */
   return fifo;

 }

/*
 *    Removes a fifo pipe
 *    fifoname is a dynamically allocated string which is freed after removing
 *    the fifo
 *
 *    This function should be called from blocker_thread before exiting
 */

void remove_fifo(char *fifoname)
{
  remove(fifoname);
  free(fifoname);
}



/*
 *    Concats input string (sysloggerxxxxxnxxxxx) and path (/tmp/)
 *    Two arguments: dest buffer, source buffer
 *    source buffer should contain sysloggerxxxxxnxxxxx string (strlen <= 20)
 *    dest buffer should have space for FIFO_NAME_LEN chars (and be empty)
 */
void concat_path(char *dest, const char *source)
{
  strncpy(dest, "/tmp/", 6); /* Copy also terminator */
  /* Starts from the terminator and adds strlen(source) chars + '/0'  */
  strncat(dest, source, strlen(source));
}




/*    OLD IMPLEMENTATIONS   */

 /*
  *    Creates a new fifo (named pipe) to /tmp
  *    Fifo name is concatenation of /tmp/syslogger and callers pid
  *    Returns name of the fifo
  */

 void create_fifo(void)
 {
   pid_t pid = getpid();
   /*  By cat /proc/sys/kernel/pid_max, max pid = 32768
       Thus, 6 (5 + '/0') chars should be enough for all pids
       And we need 14 + 6 = 20 chars for all fifo names */

   /*  Create fifoname string  */
   char fifoname[FIFO_NAME_LEN];
   strncpy(fifoname, "/tmp/syslogger", 15); /* Add also null terminator */
   char pid_str[6];
   snprintf(pid_str, 6, "%d", pid);
   strncat(fifoname, pid_str, 5);  /* Starts from dest \0 byte and adds 5 src bytes + \0 */

   printf("PID: %d\nfifoname: %s\n", pid, fifoname);

   /*  Create the fifo */
   mkfifo(fifoname, PIPE_PERMISSIONS);

 }
