/*
 *    Author: Lauri Westerholm
 *
 *    Test main for log.c
 *    Not a part of the SysLogger program (just for testing)
 */

#include "log.h"


/*
 *    Just a little test, after running check what's in LOG_FILE_NAME
 *    and ERR_LOG_NAME logs
 */

int main(void)
{
  int fd_logs[2];

  fd_logs[0] = open_log(LOG_FILE_NAME);
  fd_logs[1] = open_log(ERR_LOG_NAME);
  printf ("FDs: %d , %d\n", fd_logs[0], fd_logs[1]);

  /* Test writing */
  char message[] = "Hello world, a log test message \n";
  write_log_message(message, fd_logs);
  char error_msg[] = "Error, critical ERROR !!!!!!!!\n";
  write_error_message(error_msg, fd_logs[1]);
  for (int i = 0; i < 5; i++) {
    char msg[] = "Hello\n";
    write_log_message(msg, fd_logs);
  }
  close_logs(fd_logs);

  return 0;
}
