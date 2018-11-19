/*
 *    Author: Lauri Westerholm
 *
 *    Contains functions related to liblogger public interface
 */

 #include "syslogger.h"



/*
 *    Public interface to the syslogger program
 *    This is the function which is called and linked to an arbitrary program
 *    to make entries to the log
 *
 *    Gets the log message as an argument
 *    Construct a new fifo matching the calling thread
 *    Adds the caller pid and name to the message and tries to send it to the
 *    running SysLogger daemon
 *    If the message is succesfully written, returns 1
 *    If smt fails, returns 0
 */

int syslogger(const char *message)
{

  char msg[MAX_BYTES] = "";
  char pid_str[6] = "";
  parse_pid(pid_str);
  char *proc_name = get_prog_name(pid_str);
  if (proc_name != NULL) {

    /*  Concat proc_name and pid to the message */
    if (concat_message(msg, pid_str, proc_name, message)) {
      /* The message was succesfully created */

      /* Create the fifo */
      char *fifoname = syslogger_fifo(pid_str);

      free(proc_name);

      /* Open the fifo, if it isn't opened for reading, errno should be ENXIO */
      int fd = open(fifoname, O_WRONLY | O_NONBLOCK);

      struct timeval time_elapsed;
      gettimeofday(&time_elapsed, NULL);
      struct timeval start_time = time_elapsed;

      /* Give 2 sec time for the reading thread to connect and then timeout */
      /* Break loop also if an unexpected error occurs */
      while (1)
      {
        if (fd > 0) break;
        else if (fd < 0 && errno != ENXIO) {
          break;
        }
        else if ((time_elapsed.tv_sec > start_time.tv_sec + 1) &&
        (time_elapsed.tv_usec > start_time.tv_usec)) {
          /* Also usec checked to keep the timeout exactly at 2 sec */
          break;
        }
        fd = open(fifoname, O_WRONLY | O_NONBLOCK);
        /* Also, update time elapsed */
        gettimeofday(&time_elapsed, NULL);
      }
      free(fifoname);

      if (fd > 0) {
        /* The fifo was succesfully opened, write the message and return */
        write(fd, msg, strlen(msg));
        return 1;
      }
    }

  }

  return 0;
}

/*
 *    Get current program name from /proc/pid/status
 *    Name is on the first line
 *    Return pointer to a buffer which contains the process name
 */

char *get_prog_name(char *pid_str)
{
  char *proc_name = NULL;

  /* Construct a string, path to the program status file */
  const char proc[] = "/proc/";
  const char status[] = "/status";
  int len = strlen(proc) + strlen(pid_str) + strlen(status) +1;
  char *path = malloc(len * sizeof(char));

  strncpy(path, proc, strlen(proc) + 1);
  strncat(path, pid_str, strlen(pid_str)); /* Overwrites terminator and adds len + 1 chars */
  strncat(path, status, strlen(status));

  /* Open the file stream */
  FILE *fp;
  if ((fp = fopen(path, "r")) != NULL) {
    char temp[30]; /* This should be enough for the program name */
    fgets(temp, 30, fp);

    /* Parse proc name */
    proc_name = parse_proc_name(temp);
  }

  free(path);
  return proc_name;
}

/*
 *    Converts the pid to a string
 *    Gets the string pointer as argument (string size should be 6 chars)
 */

void parse_pid(char *pid_str)
{
  pid_t pid = getpid();
  snprintf(pid_str, 6, "%d", pid);

}


/*
 *    Parses program name from input string
 *    The input string should be first line from /proc/pid/status
 *    Returns pointer to the name string (name contains one space in front of it)
 */

char *parse_proc_name(const char *line)
{
  unsigned i = 0;
  unsigned j = 0;
  int bool_colon = 0;
  /* Go through the line and add non-empty chars after : */

  char *name = malloc(strlen(line)+ 1); /*  Excessive amount of memory */
  for (; i < strlen(line); i++)
  {
    if (line[i] == ':') {
      bool_colon = 1;
    }
    else if (bool_colon && (line[i] != ' ') && (line[i] != '\n')) {
      /* Char which is part of the name,
       to remove the front tab, check line[i] != '\t' */
      name[j] = line[i];
      j++;
    }
  }
  /* Add a null terminator and return the name */
  /* Replace the front tab with a space */
  name[0] = ' ';
  name[j] = '\0';
  return name;
}

/*
 *    Concats program name, pid and user message to one string
 *    Content is placed to dest buffer which size is MAX_BYTES (pipes.h)
 *    Returns 1 if concat was successful, 0 if it failed
 *
 *    name and pid should take max 17 + 5 chars
 *    (kernel defines max proc name, TASK_COMM_LEN, to 16 bytes and then one
 *    space is added in parse_proc_name) so they should always fit in dest
 */

int concat_message(char *dest, const char *pid_str, const char *name, const char *msg)
{
  /* Copy name to dest  */
  strncpy(dest, name, strlen(name));
  /* Add one space and null*/
  dest[strlen(name)] = ' ';
  dest[strlen(name) + 1] = '\0';
  /* Concat the pid */
  strncat(dest, pid_str, strlen(pid_str));

  int index = strlen(name) + 1 + strlen(pid_str); /* Current index */
  /* Add  ':' ' ' '\0'  compination */
  dest[index] = ':';
  index ++;
  dest[index] = ' ';
  index ++;
  dest[index] = '\0'; /* This should be overwritten by strncat */

  /* Add msg if possible */
  if (strlen(msg) < MAX_BYTES - index) {
    /* Still space left, add msg */
    strncat(dest, msg, strlen(msg)); /* Writes null terminator */
    return 1;
  }
  return 0;

}
