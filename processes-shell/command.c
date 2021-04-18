#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "command.h"

void init_command(Command *command) {
  command->name = NULL;
  command->args = str_list_init();
  command->out_file_name = NULL;
  command->is_background = 0;
}

void clear_command(Command *command) {
  free(command->name);
  str_list_free(command->args);
  free(command->out_file_name);
}

enum WordMeaning {COMMAND_NAME, ARG, OUTPUT_FILE};

int parse_command(Command *command, char *command_str) {
  int command_len = strlen(command_str);
  char word[command_len];
  int ci = 0;
  int wi = 0;
  char c;
  enum WordMeaning next_word_meaning = COMMAND_NAME;

  while ((c = command_str[ci++]) != '\0') {
    if (!isspace(c) && c != '>') {
      word[wi++] = c;
      continue;
    }

    if (c == '>') {
      next_word_meaning = OUTPUT_FILE;
    }

    if (wi == 0) {
      continue;
    }

    word[wi] = '\0';
    switch (next_word_meaning) {
    case COMMAND_NAME:
      command->name = strdup(word);
      next_word_meaning = ARG;
      break;
    case ARG:
      str_list_append_item(command->args, word);
      break;
    case OUTPUT_FILE:
      command->out_file_name = strdup(word);
    }

    wi = 0;
  }

  return 0;
}

int get_next_command(Command *command, FILE *input, int interactive) {
  static char *line = NULL;
  static char *to_free = NULL;
  static size_t linecap = 0;
  ssize_t linelen;

  while (command->name == NULL) {
    if (line == NULL) {
      free(to_free);
      if (interactive) {
	printf("wish> ");
      }
      linelen = getline(&line, &linecap, input);
      to_free = line;
    }

    if (linelen == -1) {
      free(line);
      return -1;
    }
    
    char *command_str = strsep(&line, "&");
    parse_command(command, command_str);
    if (line != NULL) {
      command->is_background = 1;
    }
  }
  
  return 0;
}
