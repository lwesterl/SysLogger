/*
 *    Author: Lauri Westerholm
 *
 *    Test file for pipes.c functions
 *    Not part of the final SysLogger program
 */

#include "pipes.h"



int main(void)
{
  create_fifo();
  list_files();
  return 0;
}
