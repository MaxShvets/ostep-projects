#ifndef STR_LIST_H
#define STR_LIST_H

typedef struct str_list_node {
  char *str;
  struct str_list_node *next;
} StringListNode;

typedef struct str_list {
  int len;
  StringListNode *start;
  StringListNode *end;
} StringList;

StringList* str_list_init();
int str_list_append_item(StringList *list, char *str);
void str_list_free(StringList *list);
void str_list_overwrite(StringList *src, StringList *dest);

#endif
