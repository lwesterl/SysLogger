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
 *    Changes working directory to root
 *    Adds a new session and group for the process
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
    perror("Function [fork()] failed ");
    return -1;

  }
  else if (pid > 0) {
    /* pid positive, parent process */
    pause();
  }

  else {
    /* Child process */
    pause();
  }

}
