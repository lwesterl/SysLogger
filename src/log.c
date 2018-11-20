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
  int fd = open(log_name, O_WRONLY | O_APPEND | O_CREAT, 0666);
  return fd;

}


/*
 *    Closes both the standard and error logs
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
 *
 *    Messages are timestamped with millisecond precision
 *    Note: adds a line feed to the message
 */

void write_log_message(const char *message, int fd[2])
{
  struct timespec curr_time;
  clock_gettime(CLOCK_REALTIME, &curr_time);
  struct tm local_time;

  /* Get local time */
  time_t t = curr_time.tv_sec;
  localtime_r(&t, &local_time);
  char time_str[120]; /* This should be always enough */
  strftime(time_str, 120, "%A %Y-%m-%d %H:%M:%S", &local_time);
  int time_len = strlen(time_str);
  /* Convert nano seconds to millis and to string format */
  char millis[4] = "";
  snprintf(millis, 4, "%ld", curr_time.tv_nsec / 1000000);
  /* Concat the time buffers */
  time_str[time_len] = ':';
  time_len ++;
  time_str[time_len] = '\0';
  time_len ++;
  strncat(time_str, millis, 100 - time_len);
  time_len = strlen(time_str);
  /* Add a space to the end and new terminator */
  time_str[time_len] = ' ';
  time_str[time_len + 1] = '\0';

  /* Now concat the time to the message */
  int len = strlen(time_str) + strlen(message) + 2; /* '\n' + '\0' */
  char *content = malloc(len * sizeof(char));
  if (content != NULL) {

    strncpy(content, time_str, strlen(time_str) +1); /* Add also null */
    strncat(content, message, strlen(message)); /* Writes strlen(message) + null */
    content[len -2] = '\n';
    content[len -1] = '\0';

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


/*
 *    Gets an error message as the first argument
 *    Adds a timestamp to the message and writes it to the standard error
 *    log which it gets a file descriptor as the second argument
 *
 *    localtime is not thread-safe but calling this function at the first place
 *    the calling thread has locked mutex
 */


void write_error_message(char *err_msg, int fd)
{

  /* Get current time */
  time_t time_now = time(NULL);
  char *time_str = asctime(localtime(&time_now));
  time_str[strlen(time_str) - 1] = ':'; /* Replace '\n' with ':' */

  int len = strlen(time_str) + strlen(err_msg) + 1;
  char *message = malloc(len);

  if (message != NULL) {
    /* Concat the strings */
    strncpy(message, time_str, strlen(time_str) + 1);
    strncat(message, err_msg, strlen(err_msg));

    /* Write the message */
    write(fd, message, len - 1);
    free(message);
  }
}
