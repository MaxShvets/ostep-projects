#include <stdlib.h>
#include <string.h>

#include "str_list.h"

StringList* str_list_init() {
  StringList *list = malloc(sizeof(StringList));
  list->len = 0;
  list->start = NULL;
  list->end = NULL;
  return list;
}

int str_list_append_item(StringList *list, char *str) {
  StringListNode *node = malloc(sizeof(StringListNode));
  node->str = strdup(str);
  node->next = NULL;
  
  if (list->start == NULL) {
    list->start = node;
  } else {
    list->end->next = node;
  }

  list->end = node;

  return ++(list->len);
}

void str_list_free_nodes(StringListNode *start) {
  StringListNode *node = start;

  while (node != NULL) {
    StringListNode *next_node = node->next;
    free(node->str);
    free(node);
    node = next_node;
  }
}

void str_list_free(StringList *list) {
  str_list_free_nodes(list->start);
  free(list);
}

void str_list_overwrite(StringList *src, StringList *dest) {
  str_list_free_nodes(dest->start);
  dest->len = 0;
  dest->start = NULL;
  dest->end = NULL;
  StringListNode* src_node;

  for (src_node = src->start; src_node != NULL; src_node = src_node->next) {
    str_list_append_item(dest, src_node->str);
  }
}
