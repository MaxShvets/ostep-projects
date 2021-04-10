#ifndef COMMAND_H
#define COMMAND_H

#include "str_list.h"

typedef struct command {
  char *name;
  StringList *args;
  char *out_file_name;
} Command;

void clear_command(Command *command);
Command *get_command(FILE *input);

#endif
