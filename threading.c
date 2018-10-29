/*
 *    Author: Lauri Westerholm
 *    Contains thread related functions
 */

 #include "logger_header.h"

// pthread_detach(pthread_t thread)
// int pthread_create (pthread_t *thread, const pthread_attr_t *attr,
//                      void  *(*start_routine) (void *), void *arg)




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
  /*  Construct a empty list of pthread_t */
  pthread_t *threads_list;
  unsigned threads_count = 0;

  /*  Check if new public pipes (which have the standard name) are added  */

}

/*
 *    Create a posix thread, pthread
 *    Return 1 if was successful, 0 if failed
 *    index is the new amount of threads
 */

int create_thread(unsigned index, const char *pipe_name, pthread_t *threads)
{
  /*  Realloc memory for a new pthread_t */
  threads = realloc(threads, sizeof(pthread_t) * index);
  if (threads == NULL) {
    /*  Bad alloc, the best option is probably to exit the program  */
    /*  IMPLEMENT     write reason for exit to the debug log */
    exit(-1);
  }
  return pthread_create( &threads[index -1], NULL, blocker_thread, (void *) pipe_name);
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
}
