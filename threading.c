/*
 *    Author: Lauri Westerholm
 *    Contains thread related functions
 */

 #include "logger_header.h"

/* Global variable */
volatile sig_atomic_t terminated = 0;


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
      /* Cancel all threads */

      cancel_all(list);
      printf("ALL canceled \n");

      /* Free all the list entries */
      /* sleep gives other threads time to exit cleanly before the structs are freed */
      sleep(1);
      remove_all(list_begin);

      /* EXIT */
      exit(0);
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

  }

}

/*
 *    Create a posix thread, pthread
 *    Return 1 if was successful, 0 if failed
 */

int create_thread(const char *pipe_name, pthread_t *threads)
{

  printf("creating thread\n");

  if (threads == NULL) {
    /*  Bad alloc, now program should exit gracefully  */
    /* NOT IMPLEMENTED */
    /*  IMPLEMENT: write exit reason to the debug log */

    exit(-1);
  }
  /* Create thread with detached attribute */
  pthread_attr_t attr;
  if (pthread_attr_init(&attr) != 0) {
    /* Error creating the thread */
    /* IMPLEMENT TERMINATING OF PROGRAM */
  }
  if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
    /* Error creating the thread */
    /* IMPLEMENT TERMINATING OF PROGRAM */
  }
  /* Create a new thread */
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
  if (pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
    /* Error */
    printf("Error in cancel state, thread exits \n");
    pthread_exit(NULL);
  }

  /*  Acquire pipename from casting the input */
  char *pipename = (char *) ptr;
  printf("hello, I am a new thread \n");
  printf("Pipename: %s\n", pipename);

  /* Open fifo matching the pipename, blocks until the fifo is opened for write */
  int fd = open_fifo_read(pipename);
  free(pipename); /* Free the heap buffer */

  if (fd < 0) {
    /* Opening failed */
    /* Write a log entry and exit thread */
    perror("FIFO opening error:");
    pthread_exit(NULL);

  }

  size_t bytes_read = 0;
  char content[MAX_BYTES] = "";

  /* Start to read the fifo in non-blocking mode until the fifo isn't empty
     If fifo is empty 0 bytes are returned. If fifo is deleted the thread will
     get stuck, only terminating whole program will cause the thread to exit */

  while (1) {
    bytes_read = read(fd, &content, MAX_BYTES);
    if (bytes_read > 0) {
      /* Successfullly read fifo content */
      break;
    }
    else if (terminated) {
      /* Program terminated, exit */
      close(fd);
      pthread_exit(NULL);
    }
    else if (errno != EAGAIN && bytes_read < 0) {
      /* Some fatal error exit, shouldn't happen */
      close(fd);
      pthread_exit(NULL);
    }
  }
  /* Write an log entry */
  printf("Content: %s\n", content);



  /* Exit without stopping other threads */
  close(fd);
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

          if (is_entry(list, dir_object->d_name) == 0) {

            /*  Add a new entry and reset list pointer */
            list = start;
            list_t *new = add_entry(list, dir_object->d_name);

            /* Create a pipename string */
            char *pipename = malloc(FIFO_NAME_LEN * sizeof(char));
            concat_path(pipename, dir_object->d_name);

            /*  Create a new thread which gets pipename as argument */
            if ( create_thread(pipename, new->thread) != 0 ) {
              /* Failed, now program should exit gracefully */
              /* NOT IMPLEMENTED */
              printf("FAILED !\n");
            }
            if (new-> thread == NULL) {
              printf("Thread: NULL\n");
            }

            printf("A thread created \n");

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
 */

void cancel_all (list_t *list)
{
  while (list != NULL)
  {
    if (list->thread != NULL) {
      /* There is still a allocated pthread_t struct */
      if ( pthread_cancel((*(list->thread))) != 0) {
        /* Error is usually ::success meaning that the thread has already exited cleanly */
        perror("Error in canceling threads: ");
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
      /* Send cancel request */
      if ( pthread_cancel((*(list->thread))) != 0) {
        /* Error */
        perror("Error canceling thread: ");
      }
    }
    /* Move pointer */
    list = list->next;
  }
}
