/*
 *    Author: Lauri Westerholm
 *    File contains functions needed to create a daemon process
 */



#include "logger_header.h"



/*
 *    Main for create_daemon
 *    Executed from terminal
 *    Starts SysLogger program as a daemon
 *
 *    Creates a child process using fork()
 *    Kills the parent switching the ownership of the child to the init process
 *    Adds a new session (and group for the process)
 *    Changes working directory to root and closes all file descriptors
 *    <-> A daemon created
 *
 *    Starts to execute the main SysLogger code:
 *      - Creates a child process to write log entries
 *      - Starts to manage syncronization and thread creation
 */

int main(int args, char *argv[])
{

  /* Create processes */
  pid_t pid = fork();

  if (pid == -1) {
    /*  Fork failed, return */
    perror("Function [ fork() ] failed ");
    return -1;

  }
  else if (pid > 0) {
    /* pid positive, parent process */
    /*  exit from parent */
    exit(0);

  }

  else {
    /*  Child process  */

    /*  Unmask files and start a new session  */
    umask(0);

    if (setsid() < 0) {
      /*  Failed to start a new session, exit */
      perror("Function [ setsid() ] failed ");
      exit(-1);
    }

    /*  Change working dir to root */
    chdir("/");
    /* Close open file descriptors: stdin, stdout and stderr  */
    close(0);
    close(1);
    close(2);

    /*
     *    Now process is a daemon
     *    Create a child process which starts handling log writing
     *    Parent starts to handle thread syncronization and creation
     */

     pid = fork();

     if (pid == -1) {
       /* Fork failed, exit. User could be notified */
       exit(-1);
     }
     else if (pid > 0) {
       /* Parent, start syncronization  */
        main_thread();
     }
     else {
       /* child */
       pause();
     }


  }

  return 0;
}
