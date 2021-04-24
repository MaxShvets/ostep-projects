#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "linked_list.h"
#include "command.h"

Command *command_init() {
  Command *command = malloc(sizeof(Command));
  command->name = NULL;
  command->args = linked_list_init();
  command->out_file_name = NULL;
  command->is_background = 0;

  return command;
}

void command_free(Command *command) {
  free(command->name);
  linked_list_free(command->args);
  free(command->out_file_name);
}

CommandList* command_list_init() {
  return linked_list_init();
}

void command_list_free(CommandList *command_list) {
  CommandListNode *c;
  for (c = command_list->start; c != NULL; c = c->next) {
    command_free(c->value);
  }

  linked_list_free(command_list);
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

int get_next_commands(CommandList *command_list, FILE *input, int interactive) {
  char *line = NULL;
  char *to_free = NULL;
  size_t linecap = 0;
  ssize_t linelen;

  if (interactive) {
    printf("wish> ");
  }

  linelen = getline(&line, &linecap, input);
  to_free = line;

  if (linelen == -1) {
    free(line);
    return -1;
  }

  while (line != NULL) {     
    char *command_str = strsep(&line, "&");
    Command *command = command_init();
    int rc = parse_command(command, command_str);
    if (rc != 0) {
      return rc;
    }
    
    if (line != NULL) {
      command->is_background = 1;
    }

    if (command->name != NULL) {
      linked_list_append_item(command_list, command);
    }
  }

  free(to_free);
  
  return 0;
}
