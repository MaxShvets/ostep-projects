#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

LinkedList* linked_list_init() {
  LinkedList *list = malloc(sizeof(LinkedList));
  list->len = 0;
  list->start = NULL;
  list->end = NULL;
  return list;
}

int linked_list_append_item(LinkedList *list, void *value) {
  LinkedListNode *node = malloc(sizeof(LinkedListNode));
  node->value = value;
  node->next = NULL;
  
  if (list->start == NULL) {
    list->start = node;
  } else {
    list->end->next = node;
  }

  list->end = node;

  return ++(list->len);
}

void linked_list_free_nodes(LinkedListNode *start) {
  LinkedListNode *node = start;

  while (node != NULL) {
    LinkedListNode *next_node = node->next;
    free(node->value);
    free(node);
    node = next_node;
  }
}

void linked_list_free(LinkedList *list) {
  linked_list_free_nodes(list->start);
  free(list);
}

