#ifndef COMMAND_H
#define COMMAND_H

#include "str_list.h"

typedef struct command {
  char *name;
  StringList *args;
  char *out_file_name;
  int is_background;
} Command;

void clear_command(Command *command);
Command *get_next_command(FILE *input, int interactive);

#endif
