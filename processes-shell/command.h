#ifndef COMMAND_H
#define COMMAND_H

#include "linked_list.h"

typedef struct command {
  char *name;
  LinkedList *args;
  char *out_file_name;
  int is_background;
} Command;

typedef LinkedListNode CommandListNode;
typedef LinkedList CommandList;

CommandList *command_list_init();
void command_list_free(CommandList *command_list);
int get_next_commands(CommandList *command_list, FILE *input, int interactive);

#endif
