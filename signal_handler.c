/*
 *    Author: Lauri Westerholm
 *
 *    Contains really simple signal handler
 *    Used in SysLogger program
 */

#include "logger_header.h"


void signal_handler(int sig)
{
  if ((sig == SIGINT) || (sig == SIGTERM)) {
    /* Change value of the global variable */
    terminated = 1;
  }
}
