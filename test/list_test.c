/*
 *    Author: Lauri Westerholm
 *
 *    Test main for linked list implementation (list.c)
 *    Only for test use
 */

#include "../src/list.h"
#include <assert.h>

int main(void)
{
  list_t *list = List();
  list_t *first_entry = list;
  char test[20] = "hello_world";
  for (char i = 0; i < 10; i++)
  {
    list_t *new;
    /*  For test create 10 unique strings */
    test[11] = i+100; /* add just some number to get visible ascii chars */
    test[12] = '\0';
    new = add_entry(list, test);
    printf("New entry: %s\n", new->fifoname);
    list = first_entry;
  }
  /* Print all entries */
  printf("ALL ENTRIES!\n");
  print_list(list);
  list = first_entry;

  /* Now try to remove each other entry   */
  printf("Remove entries \n");
  for (char i = 0; i < 20; i+=2)
  {
    test[11] = i + 100;
    test[12] = '\0';
    first_entry = remove_entry(first_entry, test);
  }

  /*  Print entries */
  printf("After removing some entries: \n");
  list = first_entry;
  print_list(list);
  list = first_entry;

  printf("Test finding etries \n");
  test[11] = (char) 103;
  assert(1 == is_entry(list, test));
  test[11] = (char) 102;
  assert( 0 == is_entry(list, test));
  printf("TEST OK\n");


  remove_all(first_entry);

  /* Another test, now with removing non-active entries */
  first_entry = List();
  list = first_entry;
  char pipename[20] = "test_pipe";
  for (char i = 10; i < 20; i++)
  {
    pipename[9] = i+50;
    add_entry(list, pipename);
    list = first_entry;

  }
  printf("______ALL ENTRIES_____\n");
  print_list(list);
  list = first_entry;
  /* Now clear status for all the entries */
  clear_status(list);
  list = first_entry;
  /* Activate some entries */
  for (char i = 10; i < 20; i+=2)
  {
    pipename[9] = i + 50;
    /* Set active only some of the entries */
    is_entry(list, pipename);
    list = first_entry;

  }
  /* Try to remove non-active entries, only odd entries should remain */
  printf("_______AFTER DEACTIVATING EVEN ENTRIES____\n");
  first_entry = remove_non_active(list);
  list = first_entry;
  print_list(list);

  /* Remove all */
  remove_all(first_entry);
  return 0;
}
