/*
 *    Author: Lauri Westerholm
 *
 *    Linked list implementation
 *    Intended to use in SysLogger program
 */


#include "list.h"


/*
 *    Construct a new linked list object
 *    Return pointer to it
 */

list_t *List(void)
{
  list_t *new_list = malloc(sizeof(list_t));
  /*  A good practise would be to check return value of malloc  */
  new_list->prev = NULL;
  new_list->next = NULL;
  new_list->status = 1;
  new_list->fifoname[0] = 0; /* Init to empty */
  new_list->thread = NULL;
  return new_list;
}

/*
 *    Adds given filename to the linked list structure
 *    list_first is a pointer to the first list item
 *    fname is the fifoname for the entry to be added
 *    Set status char 1 (pipe is active)
 *    Returns pointer to the new list item
 */

list_t *add_entry(list_t *list_first, const char fname[])
{
  /*  Check if this is the first list item created which hasn't got a fifoname */
  if (list_first->fifoname[0] == 0) {
    strncpy(list_first->fifoname, fname, 20);
    list_first->status = 1; /* Set status */
    return list_first;

  }
  else {
    /*    Need to create a new entry to the end of the list    */
    while (list_first->next != NULL)
    {
      list_first = list_first->next;
    }
    /*  Now end of the list is reached  */

    list_first->next = malloc(sizeof(list_t));
    /*  A good practise would be to check return value of malloc  */
    list_t *new = list_first->next;
    strncpy(new->fifoname, fname, 20);
    new->prev = list_first;
    new->next = NULL;
    new->status = 1; /* Set status */
    new->thread = NULL;
    return new;

  }
}

/*
 *    Removes entry from the linked list structure
 *    The entry which fifoname matches to fname is removed
 *    Gets also pointer to the first list entry as argument
 *    Returns pointer to (possibly new) first item
 */

list_t *remove_entry(list_t *list, const char fname[])
{
  /*  If there is only one entry in the list and it name matches, create a
      complitely new list */
  if ((list->next == NULL) && (list->prev == NULL) &&
    (strcmp(list->fifoname, fname) == 0))
    {
      free(list);
      return List();
    }

  /*  Holds the first memory address */
  list_t *first = list;

  while(list != NULL)
  {

    if (strcmp(list->fifoname, fname) == 0) {
      /*  Found a matching entry  */
      list_t *temp_prev = list->prev;
      list_t *temp_next = list->next;
      /*  Skip current list entry */
      if (temp_next != NULL && temp_prev != NULL) {
        /* Removing entry from the middle of the list */
        temp_prev->next = temp_next;
        temp_next->prev = temp_prev;
      }
      else if (temp_next == NULL) {
        /*  Removing the final entry  */
        temp_prev->next = NULL;
      }

      else if (temp_prev == NULL)
      {
        /*  Removing the first entry, so first needs to be reassigned */
        first = temp_next;
        first->prev = NULL;
      }


      /*  Free the current list entry and exit this function */
      free(list);
      break;
    }
    list = list->next;
  }
  /*  Return the first entry  */
  return first;

}


/*
 *    Removes all list entries
 *    As argument gets pointer to start of the list
 *
 *    This should be called when SysLogger is stopped (E.g. after receiving SIGTERM)
 *    Otherwise memory used to store the list is leaked
 */

void remove_all(list_t *list)
{
  /* Delete all list entries */
  while(list->next != NULL)
  {
    list_t *temp_next = list->next;
    temp_next->prev = NULL;
    /*  Free list  and reassign its memory address */
    free(list);
    list = temp_next;
  }
  /*  Now remove the final entry */
  free(list);
}


/*
 *    Checks if fifoname givens as parameter fname exits in the linked
 *    list. The first entry of the list is given as argument
 *    Set status 1 if name exists
 *    Return 1 if name exists and 0 if it doesn't
 */

int is_entry(list_t *list, const char fname[])
{
  /*  Iterate over all the list entries */
  while (list != NULL)
  {
    if (strcmp(list->fifoname, fname) == 0) {
      /*  Found a matching name */
      list->status = 1;
      return 1;
    }
    else {
      list = list->next;
    }
  }
  /* No match found */
  return 0;

}


/*
 *    Removes all entries from the list which status is 0 (the plain base entry cannot be removed)
 *    Used to remove non-active pipes (pipe doesn't exists anymore) from the
 *    list. Called from the main thread
 *    Returns pointer to the start of the list
 */

list_t  *remove_non_active(list_t *list)
{
  /* Check if there is only one entry */
  if ((list->status == 0) && (list->next == NULL) && (list->prev == NULL)) {
    if (list->fifoname[0] == 0) {
      /* Only the base entry left, cannot be removed  */
      return list;
    }
    else {
      /*  Create a new plain base entry */
      free(list);
      return List();

    }

  }

  list_t *start = list;
  while(list != NULL)
  {
    if (list->status == 0) {
      /* Remove the entry */
      list_t *temp_next = list->next;
      list_t *temp_prev = list->prev;
      if (temp_prev != NULL && temp_next != NULL) {
        /*  Safe to relink entries */
        temp_prev->next = temp_next;
        temp_next->prev = temp_prev;
        free(list);
        list = temp_next;
      }
      else if (temp_prev == NULL) {
        /* Removing the first entry */
        temp_next->prev = NULL;
        free(list);
        list = temp_next;
        /*  Also list has now new start pointer */
        start = list;
      }
      else if(temp_next == NULL) {
        /* Removing the last entry */
        temp_prev->next = NULL;
        free(list);
        list = temp_prev;
      }

    }
    /* Go to the next entry */
    list = list->next;
  }
  return start;
}


/*
 *    Sets list status for all entries to 0
 *    Intended to be called before checking which fifos still exits
 *    After checking fifos, non-active entries cleared with remove_non_active
 *    Argument is pointer to the start of the linked list
 */

void clear_status(list_t *list)
{
  while (list != NULL)
  {
    /* Set status to 0 */
    list->status = 0;
    list = list->next;
  }
}

/*
 *    Prints all the list entries to stdout
 *    Argument is pointer to the first entry
 *    Used to test list implementation in list_test.c
 */

void print_list(list_t *list)
{
  while (list != NULL)
  {
    printf("Fifoname: %s\n", list->fifoname);
    list = list->next;
  }
}
