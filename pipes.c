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
      Thus, 6 chars should be enough for all pids
      And we need 14 + 6 = 20 chars for all fifo names */

  /*  Create fifoname string  */
  char fifoname[20];
  strncpy(fifoname, "/tmp/syslogger", 15); /* Add also null terminator */
  char pid_str[6];
  snprintf(pid_str, 6, "%d", pid);
  strncat(fifoname, pid_str, 5);  /* Starts from dest \0 byte and adds 5 src bytes + \0 */

  printf("PID: %d\nfifoname: %s\n", pid, fifoname);

  /*  Create the fifo */
  mkfifo(fifoname, PIPE_PERMISSIONS);

}


/*
 *    Tries to open the fifo which names is given as argument in correct mode
 *    Mode is either FIFO_WRITE or FIFO_READ (check pipe.h)
 *    Returns file descriptor to the fifo
 */

int open_fifo(char *fifoname, unsigned char mode)
{
  int fd;
  if (mode == FIFO_READ) {
    fd = open(fifoname, O_RDONLY);
  }
  else {
    fd = open(fifoname, O_WRONLY);
  }

  return fd;
}
