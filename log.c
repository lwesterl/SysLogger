/*
 *    Author: Lauri Westerholm
 *
 *    Contains files relating to log writing
 */


#include "log.h"


/*
 *    Opens the filename given as argument
 *    (should be either LOG_FILE_NAME or ERR_LOG_NAME)
 *    If the log doesn't exist, creates it
 *    The log is opened in 'append' mode
 *    Returns a file descriptor to the log
 */

int open_log(char *log_name)
{
  /*  Create the log file if it doesn't exist */
  int fd = open(log_name, O_APPEND | O_CREAT, S_IWUSR | S_IRUSR);
  return fd;

}


/*
 *    Closes both the standard and error log
 *    Gets fds as argument, two integer array
 *    Doesn't check the return values because pretty much nothing
 *    can be done if close() fails
 */

void close_logs(int fd[2])
{
  close(fd[0]);
  close(fd[1]);
}

/*
 *    Writes message (pointer as argument) to the standard log adding a timestamp
 *    Fds to the logs as array of two integers (fd[0] std log, fd[1] err log)
 *    If writing fails, writes entry to the error log
 */

void write_log_message(const char *message, int fd[2])
{
  /* Get current time */
  time_t time_now = time(NULL);
  char *time_str = asctime(localtime(&time_now));

  /* Now concat the two buffers */
  int len = strlen(time_str) + strlen(message) + 1;
  char *content = malloc(len * sizeof(char));
  if (content != NULL) {

    strncpy(content, time_str, strlen(time_str) +1); /* Add also null */
    strncat(content, message, strlen(message)); /* Writes strlen(message) + null */

    /*  Try to write the message */
    if ( write(fd[0], content, len - 1) < (len - 1)) {

      /*  Smt failed during writing */
      /*  Create an error message */
      char error_str[] = "Function [ write() ] error\n";
      int err_len = strlen(error_str) + strlen(time_str) +1;
      char *err_msg = malloc(err_len * sizeof(char));
      if (err_msg != NULL) {
        strncpy(err_msg, time_str, strlen(time_str) + 1);
        strncat(err_msg, error_str, strlen(error_str));

        write(fd[1], err_msg, err_len -1); /* Don't write the null */
        free(err_msg);
      }

    }
    free(content);
  }


}