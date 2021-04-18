#ifndef COMMAND_H
#define COMMAND_H

#include "str_list.h"

typedef struct command {
  char *name;
  StringList *args;
  char *out_file_name;
  int is_background;
} Command;

void init_command(Command *command);
void clear_command(Command *command);
int get_next_command(Command *command, FILE *input, int interactive);

#endif
