# SysLogger

Own implementation of the syslog daemon and an associated library

## Repository structure

1. doc
  * Contains project diary
2. Makefile
  * Basic Makefile used to build SysLogger program and liblogger static library
3. src
  * All primary source codes and headers used with SysLogger
4. test
  * All test source codes and headers which were used to test SysLogger

## Build Instructions

* Use make to build the primary executables
  * Executables generated by make debug should only be used for testing
  * Also, make test_memory and test_stress_test are only for testing        purposes
* Use make clean-objects to remove unnecessary object files
* Use make clean to remove all SysLogger related executables, objectives and liblogger static library


 Command | Description
 ------- | ------------
 make / make all | Build SysLogger, Stress_tester and liblogger
 make debug      | Build debug executables
 make clean      | Clean all executables, objectives + liblogger
 make clean-objectives | Clean objectives
 make clean-debug | Clean debug executables
 make clean-lib   | Clean liblogger
 make test_memory | Run Valgrind leakcheck on thread_test
 make test_stress_test | Run Valgrind leakcheck on stress_debug

## Usage Instructions

1. Starting the daemon
   1.  make executables
   2.  From base directory run command: ./SysLogger start
   3.  SysLogger should now be running: _SysLogger daemon succesfully started_
   4.  If SysLogger was already running: _SysLogger daemon is already running_

2. Stopping the daemon
   1. From base directory run command: ./SysLogger stop
   2. Syslogger should now be stopped: _SysLogger daemon stopped_
   3. If SysLogger was already stopped: _Error stopping the daemon, is it really running?_

3. Stress testing SysLogger
   1. From base directory run command: ./Stress_tester
   2. If SysLogger isn't running: _SysLogger isn't running at the moment_
   3. If stress test fails: _SysLogger failed_
   4. Otherwise, test entries should be written to /var/tmp/SysLogger.log

4. Using SysLogger elsewhere
   1. Include src/syslogger.h (and src/pipes.h, syslogger.h needs also this)
   2. Send SysLogger messages via _int syslogger(const char *message)_
      * Max length for one message is 199 chars
      * Return value: 1 (success), 0 (failure)
      * Line feed is added at the end of the message
      * Message format: _TIME PROGRAM PID: MESSAGE_
   3. Link binaries with static library liblogger (-llogger)

## Operating principles & Limitations

  * SysLogger outputs messages to /var/tmp/SysLogger.log
    * It would be better to write under root but it would require root privileges
  * Error messages are written to /var/tmp/SysLogger_error.log
  * Messages are sent via named pipes (fifos)
    * Setting priority to messages isn't possible
  * Under too high workload timeouts will trigger after 2s
    * In this case _syslogger_ returns 0
    * No error messages are written (to limit workload)
    * During stress testing SysLogger has been tested with 500 simultaneous senders without problems
  * _syslogger_ is thread-safe


## Brief File Content Description

  ### create_daemon.c
    * Contains main for SysLogger
    * Creates SysLogger daemon process
    * Initializes pid file to /tmp
      * Better place would be at /run

  ### threading.c & logger_header.h
    * Contains functions for the main thread and 'blocker_threads'
    * Main thread creates other threads and handles incoming signals
    * 'blocker_threads' read fifos created by _syslogger_
    * After receiving message they lock mutex and write messages to the log

  ### syslogger.c & syslogger.h
    * Contains SysLogger Public interface
    * _syslogger_ which handles connecting to the daemon is defined here
    * Contains also multiple string processing helper functions

  ### list.c & list.h
    * Contains doubly linked list implementation
    * This linked list is used by the main thread to store pointers to all other threads

  ### pipe.c & pipe.h
    * Contains functions related to opening, closing and creating pipes (fifos)

  ### log.c & log.h
    * Contains functions relating to SysLogger log writing

  ### signal_handler.c
    * Really simple signal handler which the main thread uses

  ### stress_test.c & stress_test.h
    * Contains the stress test for SysLogger
    * Not part of the SysLogger program itself
