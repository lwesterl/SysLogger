/*
 *    Author: Lauri Westerholm
 *    Contains thread related functions
 */

 #include "logger_header.h"


/* Global variables */

volatile sig_atomic_t terminated = 0;
int logs_fd[2]; /* File descriptors to the log files */
pthread_mutex_t write_mutex; /* A mutex used to control the log writing */



/*
 *    The original thread executes this function (i.e. is called from main)
 *
 *    This thread creates other threads based on public pipes opened to
 *    temp-folder. The content of the new pipes, which are named as standard
 *    name + pid, should be written to the log
 *    This thread starts a new thread for every new pipe
 */

void main_thread(void)
{
  /* Open the logs, fd[0] == main log, fd[1] == error log */
  logs_fd[0] = open_log(LOG_FILE_NAME);
  logs_fd[1] = open_log(ERR_LOG_NAME);
  if (logs_fd[0] == -1 || logs_fd[1] == -1) {
    /* Error, remove the pid file and exit */
    remove(PID_FILE);
    exit(-1);
  }
  /* Write a log entry that SysLogger was started */
  write_log_message(" SysLogger daemon started\n", logs_fd);

  /* Init a mutex to synchronize log writing */
  pthread_mutex_init(&write_mutex, NULL); /* Default mutex attributes */

  /* Connect a real basic signal handler */
  struct sigaction sig_action = {.sa_handler = signal_handler};
  sig_action.sa_flags = 0;

  sigaction(SIGTERM, &sig_action, NULL);
  sigaction(SIGINT, &sig_action, NULL);

  /*  Construct an empty linked list of for fifonames, their status and
      pthread_t pointers  */

  list_t *list = List();
  list_t *list_begin = list; /* Pointer to the start of the list */

  /*  Execute this loop until terminated by SIGTERM */

  while(1)
  {
    if (terminated) {
      /* Make a clean exit */
      make_clean_exit(list_begin, CLEAN_EXIT);
    }

    /* First clear all the status bytes from the list entries */
    clear_status(list);
    list = list_begin; /* Reset pointer to the start of the list */

    /*  Check if new public pipes (which have the standard name) are added
        This also sets correct status for all the entries so that non-active
        can be removed easily
    */

    list_files(list);
    list = list_begin;

    /* Remove non-active fifo entries (and their pthread structs),
     returns new pointer to the first entry */
    list_begin = remove_non_active(list_begin);
    list = list_begin; /* Reset pointer to the start of the list */

    usleep(5000); /* Sleep 5 ms, otherwise cpu usage 100 % */
  }

}

/*
 *    Create a posix thread, pthread
 *    Return 1 if was successful, 0 if failed
 */

int create_thread(char *pipe_name, pthread_t *threads)
{
  if (threads == NULL) {
    /*  Bad alloc, now program should exit gracefully  */
    /* Possibly free the buffer */
    if (pipe_name != NULL) {
      free(pipe_name);
    }
    return -1;
  }
  /* Create thread with detached attribute */
  pthread_attr_t attr;
  if (pthread_attr_init(&attr) != 0) {
    /* Error creating the thread */
    return -1;
  }
  if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
    /* Error creating the thread */
    return -1;
  }
  /* Create a new thread */
  if (terminated) {
    free(pipe_name);
    free(threads);
    return -1;
  }
  int ret = pthread_create( threads, &attr, blocker_thread, (void *) pipe_name);
  pthread_attr_destroy(&attr);
  return ret;
}

/*
 *    All created threads (except the original, main thread) execute this
 *
 *    File descriptor to a pipe is given to a thread as input
 *    The thread opens the read end of the pipe
 *    Thread blocks until it can read message from the pipe
 *    Then the thread waits for to unlock mutex to write content to the log
 *    Finally the thread messages that it can be detached
 *    The previous is done by acquiring a write lock to a special pipe
 *    (the pipe which the main thread constantly reads in non-blocking mode)
 */

void *blocker_thread(void *ptr)
{
  /* Set cancel state to asynchronous */
  /* However, threads should be always let to itself cancel otherwise race
    conditions may occur */
  if (pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
    /* Error, should NOT ever happen */
    free (ptr);
    pthread_exit(NULL);
  }

  /*  Acquire pipename from casting the input */
  char *pipename = (char *) ptr;

  /* Open fifo matching the pipename, opens in non-blocking mode */
  int fd = open_fifo_read(pipename);

  if (fd < 0) {
    /* Opening failed */
    /* Write a log entry and exit thread */
    pthread_mutex_lock(&write_mutex);

    char err_buff[100] = ""; /* This should be enough for the error string */
    strerror_r(errno, err_buff, 100); /* Thread-safe */
    /* Add a line feed to the error buffer */
    int len = strlen(err_buff);
    err_buff[len] = '\n';
    err_buff[len + 1] = '\0';
    write_error_message(err_buff, logs_fd[1]);
    write_error_message("Thread exits\n", logs_fd[1]);

    pthread_mutex_unlock(&write_mutex);
    /* Remove the fifo and free the memory allocated for pipename */
    remove_fifo(pipename);
    pthread_exit(NULL);

  }

  ssize_t bytes_read = 0;
  char content[MAX_BYTES] = "";

  /* Start to read the fifo in non-blocking mode until the fifo isn't empty
     If fifo is empty 0 bytes are returned. Timeout after 2 seconds
     and remove the fifo, to avoid freezing the thread                    */

  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  struct timeval elapsed_time = start_time;


  while (1) {
    bytes_read = read(fd, &content, MAX_BYTES);
    if (bytes_read > 0) {
      /* Successfully read fifo content */
      break;
    }
    else if (terminated) {
      /* Program terminated, exit */
      close(fd);
      /* Remove the fifo and free the memory allocated for pipename */
      remove_fifo(pipename);
      pthread_exit(NULL);
    }
    else if (errno != EAGAIN && bytes_read < 0) {
      /* Some fatal error exit, shouldn't happen */
      close(fd);
      /* Remove the fifo and free the memory allocated for pipename */
      remove_fifo(pipename);
      pthread_exit(NULL);
    }
    else if ((elapsed_time.tv_sec > start_time.tv_sec + 1) &&
            (elapsed_time.tv_usec > start_time.tv_usec)) {
      /* usecs need to be checked also to keep the timeout exactly at 2 sec */
      close(fd);
      /* Remove the fifo and free the memory allocated for pipename */
      remove_fifo(pipename);
      pthread_exit(NULL);
    }
    /* Update elapsed_time */
    gettimeofday(&elapsed_time, NULL);

  }

  /* Write an log entry, try to lock write_mutex (block until locking possible)*/

  pthread_mutex_lock(&write_mutex);
  write_log_message(content, logs_fd);
  pthread_mutex_unlock(&write_mutex);

  /* Exit without stopping other threads */
  close(fd);
  /* Remove the fifo and free the memory allocated for pipename */
  remove_fifo(pipename);

  pthread_exit(NULL);
}



/*
 *    Goes through all named pipes in /tmp
 *    Picks the pipes which names contain syslogger-string
 *    Checks if the linked list (given as argument) contains
 *    entries for those pipes (calls is_entry)
 *    Adds entries for new pipes by calling add_entry
 *    Start a new thread for every new pipe by calling create_thread
 *    Pointer to the thread is stored in the list
 */

void list_files(list_t *list)
{
  list_t *start = list; /* Pointer to the start of the list */

  DIR *dirp;  /*  Directory stream */
  struct dirent *dir_object; /* dirent defined in dirent.h */

  /*  Open /tmp */
  dirp = opendir("/tmp");

  if (dirp) {
    /* Go through dir content */
    while ((dir_object = readdir(dirp)) != NULL)
    {
      /*  Pick only named pipes, fifos  */
      if (dir_object->d_type == DT_FIFO) {
        if (strstr(dir_object->d_name, "syslogger")) {

          /* Pipe is created by the program */
          //printf("%s\n", dir_object->d_name);
          /* Check if the list contains the fifo */

          if (is_entry(list, dir_object->d_name) == 0 && !terminated ) {

            /*  Add a new entry and reset list pointer */
            list = start;
            list_t *new = add_entry(list, dir_object->d_name);

            /* Create a pipename string */
            char *pipename = malloc(FIFO_NAME_LEN * sizeof(char));
            concat_path(pipename, dir_object->d_name);

            /*  Create a new thread which gets pipename as argument */
            if ( create_thread(pipename, new->thread) != 0 ) {
              /* Failed, now program should exit gracefully */
              make_clean_exit(start, THREAD_CREATION_ERROR_EXIT);
            }

          }
          list = start; /* Set list back to the start of the list */
        }

      }
    }
    closedir(dirp);
  }

}


/*
 *    Cancels all threads apart from the main thread
 *    Goes through the list and calls pthread_cancel
 *    Gets list_t pointer list as argument
 *
 *    This functions is intended to force threads to exit if the haven't done it
 *    itself
 *    It's prone to cause race conditions which result to a segmentation violation
 */

void cancel_all (list_t *list)
{
  while (list != NULL)
  {
    if (list->thread != NULL) {
      /* There is still a allocated pthread_t struct */
      if ( pthread_cancel((*(list->thread))) != 0) {
        /* Error is usually ::success meaning that the thread has already exited cleanly */
        /* These are not logged to the error log */
        //perror("Error in canceling threads: ");
      }
    }

    /*  Move the pointer to the next entry */
    list = list->next;
  }
}


/*
 *    Cancels all threads which are non-active
 *    Get list_t list pointer as argument
 */

void cancel_non_active (list_t *list)
{
  while(list != NULL)
  {
    if ((list->status == 0) && (list->thread != NULL))
    {
      /* Send a cancel request, this should have no effect because threads has
      already canceled itself */
      if ( pthread_cancel((*(list->thread))) != 0) {
        /* Error */
        //perror("Error canceling thread: ");
      }
    }
    /* Move pointer */
    list = list->next;
  }
}

/*
 *    Makes clean exit from SysLogger
 *    Called from the main thread when the program is terminated
 *    list_t list which contains the thread and pipe entries
 *    exit_reason is a macro value (CLEAN_EXIT, THREAD_CREATION_ERROR_EXIT)
 */

void make_clean_exit(list_t *list, int exit_reason)
 {
  /* Give time to all threads to cancel and then remove the list entries */
  list_t *list_begin = list; /* Store the first entry pointer */


  /* Free all the list entries */
  /* sleep gives other threads time to exit cleanly before the structs are freed */
  sleep(2);
  remove_all(list_begin);

  /* Destroy the mutex */
  pthread_mutex_destroy(&write_mutex);

  if (exit_reason == THREAD_CREATION_ERROR_EXIT) {
    /* Write the exit reason to the error log */
    write_error_message(" Thread creation error, forced exit\n", logs_fd[1]);
  }

  /* Write the final exit message to the log */
  char exit_msg[] = " SysLogger daemon stopped\n";
  write_log_message(exit_msg, logs_fd);

  /* Close the log file descriptors */
  close_logs(logs_fd);
  /*  Remove the pid file, if program is terminated by SysLogger stop, the file
      should be already removed                                               */
  remove(PID_FILE);

  /* EXIT */
  exit(0);
}
