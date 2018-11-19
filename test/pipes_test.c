/*
 *    Author: Lauri Westerholm
 *
 *    Test file for pipes.c functions
 *    Not part of the final SysLogger program
 */

#include "../src/pipes.h"



int main(void)
{
  printf("Tries to create one fifo to /tmp\n");
  create_fifo();
  return 0;
}
