#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "command.h"

void init_command(Command *command) {
  command->name = NULL;
  command->args = linked_list_init();
  command->out_file_name = NULL;
  command->is_background = 0;
}

void clear_command(Command *command) {
  free(command->name);
  linked_list_free(command->args);
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

  while (1) {
    c = command_str[ci++];
    if (!isspace(c) && c != '>' && c != '\0') {
      word[wi++] = c;
      continue;
    }

    if (wi != 0) {
      word[wi] = '\0';
      switch (next_word_meaning) {
      case COMMAND_NAME:
	command->name = strdup(word);
	next_word_meaning = ARG;
	break;
      case ARG:
	linked_list_append_item(command->args, strdup(word));
	break;
      case OUTPUT_FILE:
	if (command->out_file_name != NULL) {
	  return 1;
	}
	command->out_file_name = strdup(word);
      }
      
      wi = 0;
    }

    if (c == '>') {
      next_word_meaning = OUTPUT_FILE;
    } else if (c == '\0') {
      break;
    }
  }

  if (next_word_meaning == OUTPUT_FILE && command->out_file_name == NULL) {
    return 1;
  }

  if (command->name == NULL && command->out_file_name != NULL) {
    return 1;
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
    int rc = parse_command(command, command_str);
    if (rc != 0) {
      return rc;
    }
    
    if (line != NULL) {
      command->is_background = 1;
    }
  }
  
  return 0;
}
