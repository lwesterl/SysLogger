/*
 *    Author: Lauri Westerholm
 *
 *    A stress test main for the SysLogger program
 *    The interface to SysLogger:
 *    int syslogger(const char *message)
 */

#include "stress_test.h"

/*
 *    Starts multiple processes and each process multiple threads
 *    Each thread tries to write a message to SysLogger using syslogger function
 *
 *    FORKS_TARGET defines amount of the forks
 */

 int main(void)
 {
   printf("Stress test started\n");

   /* Check the pid file exists, pid file location is defined in logger_header.h
      but including the whole header seems a bit excessive                    */

   int fd = open("/tmp/SysLogger.pid", O_RDONLY);
   if (fd < 0) {
     printf("SysLogger isn't running at the moment\n");
     return 0;
   }
   /* Opening the pid file is used just as an indicator that SysLogger is running */
   close(fd);
   pid_t pid;

   for (int i = 0; i < FORKS_TARGET; i ++)
   {
     /* Fork */
     pid = fork();
     if (pid == 0) {
       /* Child */
       child_process(i);
     }
     if (pid == -1) {
       /* Fork failed, return from parent */
       return -1;
     }
   }
   /* Return from parent */
   return 0;
 }


/*
 *    Creates THREADS_TARGER amount of new threads
 *    The threads execute thread_function
 *    The threads are created with detached state so that their resources are
 *    automatically collected at exit
 *    Gets process number count as argument
 *
 *    Allocates a string based on count and thread number
 *    The string pointer is given to thread_function as argument
 */

 void child_process(int count)
 {
   int ret;
   //printf("Hello\n");
   for (int i = 0; i < THREADS_TARGER; i ++)
   {
     /* Create THREADS_TARGER amount of detached threads */

     pthread_t thread;
     /* Create thread with detached attribute */
     pthread_attr_t attr;
     if (pthread_attr_init(&attr) != 0) {
       /* Error creating the thread */
       exit(-1);
     }
     if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
       /* Error creating the thread */
       exit(-1);
     }
     /* Create a thread number buffer from the current i value, i max 99 */
     char thread_number[3] = "";
     snprintf(thread_number, 3, "%d", i);
     /* Create a process number buffer from count, count max 99 */
     char process_number[3] = "";
     snprintf(process_number, 3, "%d", count);

     //printf("Hello 222 \n");
     /* Concat the buffers to base strings */
     char base_str[] = " Stress tester process number, ";
     char base_str2[] = ", thread number, ";
     int len = strlen(base_str) + strlen(process_number) + strlen(base_str2)
                + strlen(thread_number) + 2; /* Line feed + terminator */
     char *message = malloc(len * sizeof(char));

     if (message != NULL) {

       strncpy(message, base_str, strlen(base_str) + 1);
       strncat(message, process_number, strlen(process_number));
       strncat(message, base_str2, strlen(base_str2));
       strncat(message, thread_number, strlen(thread_number));
       int index = strlen(message);
       message[index] = '\n'; /* Add a line feed */
       message[index + 1] = '\0';

       /* Create the thread */
       //printf("Thread creation: %s\n", message);
       /*if (syslogger(message) == 0) {
         smt failed in syslog
         printf("SysLogger failed \n");
       }*/
       ret = pthread_create(&thread, &attr, thread_function, (void *) message);
       pthread_attr_destroy(&attr);
       if (ret != 0) {
          //smt went wrong, exit
         perror("Thread creation error:");
         exit(-1);
       }

     }
     //exit(0);
   }
   /* All threads succesfully created, exit */
   pause();
   exit(0);
 }

/*
 *    Calls syslog
 *    This function is executed by every thread child_process creates
 *    Get syslog message as an argument
 */

 void *thread_function(void *buffer)
 {

   /* Do type cast to a string */
   char *message = (char *) buffer;
   //printf("%s\n", message);

   /* Call syslogger */
   if (syslogger(message) == 0) {
     /* smt failed in syslog  */
     printf("SysLogger failed \n");
   }

   /* Free the dynamicelly allocated pointer and exit*/
   free(message);
   pthread_exit(NULL);

 }
