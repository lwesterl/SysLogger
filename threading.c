/*
 *    Author: Lauri Westerholm
 *    Contains thread related functions
 */

 #include "logger_header.h"




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

  /*  Construct an empty linked list of for fifonames, their status and
      pthread_t pointers  */

  list_t *list = List();
  list_t *list_begin = list; /* Pointer to the start of the list */

  /*  Execute this loop until terminated by SIGTERM */
  while(1)
  {
    /* First clear all the status bytes from the list entries */
    clear_status(list);
    list = list_begin; /* Reset pointer to the start of the list */

    /*  Check if new public pipes (which have the standard name) are added
        This also sets correct status for all the entries so that non-active
        can be removed easily
    */

    list_files(list);

    /* Remove non-active fifos (and their threads),
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
  /*  Alloc memory for a new pthread_t */
  printf("creating thread\n");
  threads = malloc(sizeof(pthread_t));
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
  /*  Acquire pipename from casting the input */
  char *pipename = (char *) ptr;
  printf("hello, I am a new thread \n");
  printf("Pipename: %s\n", pipename);

  /* Open fifo matching the pipename, blocks until the fifo is opened for write */
  int fd = open_fifo(pipename, FIFO_READ);

  if (fd == -1) {
    /* Opening failed, this shouldn't happen */
    /* Write a log entry and exit thread */
    perror("FIFO opening error:");
    pthread_exit(NULL);

  }

  size_t bytes_read = 0;
  char content[MAX_BYTES];

  /* Start to read the fifo and block until the fifo isn't empty */
  while (1) {
    bytes_read = read(fd, &content, MAX_BYTES);
    if (bytes_read > 0) {
      /* Successfullly read fifo content */
      break;
    }
    else if (bytes_read == -1) {
      /* Error, write errno to the error log and exit thread */
      /* IMPLEMENT */
      pthread_exit(NULL);
    }
  }
  /* Write an log entry */
  printf("Content: %s\n", content);



  /* Exit without stopping other threads */
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
            char pipename[FIFO_NAME_LEN];
            concat_path(pipename, dir_object->d_name);

            /*  Create a new thread which gets pipename as argument */
            if ( create_thread(pipename, new->thread) != 0 ) {
              /* Failed, now program should exit gracefully */
              /* NOT IMPLEMENTED */
              printf("FAILED !\n");
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
