/*
 *    Author: Lauri Westerholm
 *
 *    Header for a linked list (list.c)
 */

#ifndef LIST_HEADER
#define LIST_HEADER

/*    C libraries   */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

/*    STRUCTS   */

/*    Struct for linked list    */
typedef struct list {
  char fifoname[21];  /*  Fifonames takes max 21 bytes, sysloggerxxxxxnxxxxx0 */
  char status;  /* 1 == struct matching pipe still active, 0 == pipe removed */
  struct list *next;
  struct list *prev;
  pthread_t *thread; /* Pointer to the thread which handles the fifo */

} list_t;


/*    Function declarations   */
list_t *List(void);
list_t *add_entry(list_t *list_first, const char fname[]);
list_t *remove_entry(list_t *list, const char fname[]);
void remove_all(list_t *list);
int is_entry(list_t *list, const char fname[]);
list_t  *remove_non_active(list_t *list);
void clear_status(list_t *list);
void print_list(list_t *list);




#endif
