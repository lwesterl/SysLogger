/*
 *    Author: Lauri Westerholm
 *    File contains functions needed to create a daemon process
 */



#include "logger_header.h"



/*
 *    Main for SysLogger program
 *    Executed from terminal
 *
 *    Agrs: start/stop
 *    Start:
 *      Starts SysLogger program as a daemon (a traditional deamon process)
 *    Stop:
 *      Stops running SysLogger daemon
 */

int main(int args, char *argv[])
{
  /* Check args */
  if (args != 2) {
    printf("Function [ SysLogger ] invalid args: SysLogger start/stop \n");
    return 0;
  }
  else if (strcmp(argv[1], "start") == 0) {
    /* Start SysLogger */
    start_SysLogger();
  }
  else if (strcmp(argv[1], "stop") == 0) {
    /* Stop SysLogger */
    stop_SysLogger();
  }
  else {
    printf("Function [ SysLogger ] invalid args: SysLogger start/stop \n");
    return 0;
  }

  return 1;
}



/*
 *    Creating SysLogger daemon process:
 *    Closes all excessive file descriptors
 *    Creates a child process using fork()
 *    Adds a new session (and group for the process) to the child
 *    The reforks and kills the middle parent switching the ownership of the final
 *    child process to the init process
 *    Changes working directory to root and redirects fd 0, 1, 2 to /dev/null
 *    <-> A daemon created
 *
 *    Starts to execute the main SysLogger code:
 *      - Creates a child process to write log entries
 *      - Starts to manage syncronization and thread creation
 */

void start_SysLogger(void) {
  /* Close all files descriptors apart from stdin, stdout, stderr */

  struct rlimit limits;
  if (getrlimit(RLIMIT_NOFILE, &limits) != 0) {
    printf ("Setting up daemon failed\n");
    exit(-1);
  }

  /* Close all file descriptors from 3 to the soft limit
  (leads to many bad file descriptor errors which are just ignored) */
  for (int fd = 3;  fd < limits.rlim_cur; fd++)
  {
    if (close(fd) == -1 && errno != EBADF) {
      perror("Function [ close() ] error ");
    }
  }

  /* Create an unmamed pipe used to check that the daemon creation succeeds */
  int pipe_fd[2];
  pipe(pipe_fd);

  /* Create processes */
  pid_t pid = fork();

  if (pid == -1) {
    /*  Fork failed, return */
    perror("Function [ fork() ] failed ");
    printf("Setting up SysLogger daemon failed \n");
    exit(-1);

  }
  else if (pid > 0) {
    /* pid positive, parent process */
    /*  exit from parent */
    /*  This should inform user that fork was ok, so it should open a pipe
        with a timeout, NOT IMPLEMTED */
    /* Use poll() to check when pipe has content to read, add 2 s timeout */
    close(pipe_fd[1]); /* Close write end of the pipe */
    struct pollfd poll_fd;
    poll_fd.fd = pipe_fd[0];
    poll_fd.events = POLLIN;
    poll_fd.revents = POLLIN;

    if (poll(&poll_fd, 1, 2000) > 0) {
      /* Success, read fd content (100 chars should be more than enough for the message) */
      char buffer[100] = "";
      read(pipe_fd[0], &buffer, 100);
      close(pipe_fd[0]);
      printf("%s\n", buffer);
    }
    else {
      close(pipe_fd[0]);
      printf("Setting up SysLogger daemon failed \n");
    }
    exit(0);

  }

  else {
    /*  Child process  */

    if (setsid() < 0) {
      /*  Failed to start a new session, exit */
      perror("Function [ setsid() ] failed ");
      exit(-1);
    }

    /* Fork again to permanently disconnect from any teminal */
    pid = fork();

    if (pid == -1) {
      /* Fork failed, exit */
      perror("Function [ fork() ] failed");
      exit(-1);
    }
    else if (pid > 0) {
      /* Parent process, exit */
      exit(0);
    }

    else {
      /* The final child process, owned by init */

      /*  Unmask files  */
      umask(0);

      /*  Change working dir to root */
      chdir("/");
      /* Close open file descriptors: stdin, stdout and stderr  */
      close(0);
      close(1);
      close(2);
      /* Open three descriptors to /dev/null */
      open("/dev/null", O_RDONLY);
      open("/dev/null", O_WRONLY);
      open("/dev/null", O_WRONLY);

      /* Check if the program is already running (try to write pid to the pid file) */
      int ok = write_pid_file();

      /* Write to the pipe that if daemon was successfully created  */
      close(pipe_fd[0]); /* Close read end of the pipe */
      struct pollfd poll_fd;
      poll_fd.fd = pipe_fd[1];
      poll_fd.events = POLLOUT;
      poll_fd.revents = POLLOUT;

      if (poll(&poll_fd, 1, 2000) > 0) {
        /* Success, write a message */

        if (ok) {
          /* New daemon created */
          char message[] = "SysLogger daemon succesfully started";
          write(pipe_fd[1], &message, strlen(message));
          close(pipe_fd[1]);
        }
        else if (ok == 0) {
          /* Daemon already started */
          char message[] = "SysLogger daemon is already running";
          write(pipe_fd[1], &message, strlen(message));
          close(pipe_fd[1]);
          /* Exit this process to avoid multiple daemon situation */
          exit(0);
        }
        else {
          char message[] = "Something went severely wrong during creating SysLogger";
          write(pipe_fd[1], &message, strlen(message));
          close(pipe_fd[1]);
          exit(-1);
        }

      }
      else {
        /* Failed, exit */
        close(pipe_fd[1]);
        exit(-1);
      }


      /*
       *    Now process is a daemon and it has been informed to user
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
         pause();
          //main_thread();
       }
       else {
         /* child */
         pause();
       }
    }


  }

}

/*
 *    Stop SysLogger daemon
 *    Get running daemon process id from PID_FILE
 *    Sends SIGTERM to the id  and id + 1 and removes the PID_FILE
 *
 *    Notice: parent daemon process id is in the file but also child is
 *    created which id is parent + 1
 *    Also, ignore the PID_FILE write lock (otherwise this won't work)
 */

void stop_SysLogger(void)
{
  int fd = open(PID_FILE, O_RDWR);
  if (fd == -1) {
    printf("Error stopping the daemon, is it really running?\n");
    exit(0);
  }

  /* Now read pid, pid max len 5 */
  char pid_str[6] = "";
  read(fd, &pid_str, 5);
  close(fd);
  long pid = strtol(pid_str, NULL, 10);

  /* Send SIGTERMs */
  kill(pid, SIGTERM);
  kill(pid + 1, SIGTERM);

  /* Remove PID_FILE */
  remove(PID_FILE);

  printf("SysLogger daemon stopped\n");
  exit(0);

}


/*
 *    Opens SysLogger program pid file
 *    Tries to acquire a write lock
 *    If can't acquire the lock, returns 0 (SysLogger should exit)
 *    If locking is possible, writes pid to the file and returns 1
 *    If writing fails, smt went severely wrong (SysLogger should exit), returns -1
 *
 *    Lock is freed when the SysLogger program exits (must not be freed prior)
 */

int write_pid_file(void)
{
  /*  Open the pid file (create with write and read permissons if it doesn't exist) */
  int fd = open(PID_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  struct flock flock;
  flock.l_type = F_WRLCK;
  flock.l_whence = SEEK_SET;
  flock.l_start = 0;
  flock.l_len = 0; /* Whole file */
  flock.l_pid = getpid();

  /* Try to lock the pid file */
  if (fcntl(fd, F_SETLK, &flock) == -1) {
    /* File already in use and locked */
    return 0;
  }

  /* Write pid to the file (max pid = 32768) */
  char pid_str[6] = "";
  snprintf(pid_str, 6, "%d", getpid());
  if (write(fd, pid_str, 6) < 6) {
    /* Error whole program should exit */
    return -1;
  }
  /* Success */
  return 1;

}
