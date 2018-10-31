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
 *    Concats input string (sysloggerxxxxx) and path (/tmp/)
 *    Two arguments: dest buffer, source buffer
 *    source buffer should contain sysloggerxxxxx string (strlen )
 *    dest buffer should have space for FIFO_NAME_LEN chars (and be empty)
 */
void concat_path(char *dest, const char *source)
{
  strncpy(dest, "/tmp/", 6); /* Copy also terminator */
  /* Starts from the terminator and adds 14 chars + '/0'  */
  strncat(dest, source, 14);
}
