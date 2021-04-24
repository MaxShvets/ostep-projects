#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct linked_list_node {
  void *value;
  struct linked_list_node *next;
} LinkedListNode;

typedef struct linked_list {
  int len;
  LinkedListNode *start;
  LinkedListNode *end;
} LinkedList;

LinkedList* linked_list_init();
int linked_list_append_item(LinkedList *list, void *value);
void linked_list_free(LinkedList *list);
void linked_list_free_nodes(LinkedListNode *list);

#endif
