#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "command.h"

void clear_command(Command *command) {
  free(command->name);
  str_list_free(command->args);
  free(command->out_file_name);
  free(command);
}

int is_special_char(char c) {
  return c == EOF || c == '&' || c == '\n' || c == '&';
}

size_t get_word(char **wordp, FILE *input) {
  size_t word_size = 2;
  *wordp = malloc(sizeof(char) * word_size);
  if (*wordp == NULL) {
    fprintf(stderr, "failed to allocate memory for a word: %s", strerror(errno));
    exit(1);
  }

  int i = 0;
  char c;
  
  while ((c = getc(input))) {
    if (isspace(c) || is_special_char(c)) {
      ungetc(c, input);
      break;
    }

    if (i == word_size - 1) {
      word_size *= 2;
      *wordp = realloc(*wordp, word_size * sizeof(char));

      if (*wordp == NULL) {
	fprintf(stderr, "failed to allocate memory for a word: %s", strerror(errno));
	exit(1);
      }
    } 

    (*wordp)[i++] = c;
  }

  (*wordp)[i] = '\0';

  return i;
}

enum WordMeaning {COMMAND_NAME, ARG, OUTPUT_FILE};

int consume_whitespace(FILE *input) {
  char c = getc(input);

  while (isspace(c) && !is_special_char(c)) {
    c = getc(input);
  }

  return c;
}

Command *get_command(FILE *input, int interactive) {
  int at_eof = 0;

  while (!at_eof) {
    if (interactive) {
      printf("wish> ");
    }

    enum WordMeaning next_word_meaning = COMMAND_NAME;
    Command *command = malloc(sizeof(Command));
    command->name = NULL;
    command->args = str_list_init();
    command->out_file_name = NULL;
    command->is_background = 0;
    
    while (1) {
      char c = consume_whitespace(input);
      if (c == '\n') {
	interactive = 1;
	break;
      } else if (c == '&') {
	command->is_background = 1;
	break;
      } else if (c == EOF) {
	at_eof = 1;
	break;
      } else if (c == '>') {
	next_word_meaning = OUTPUT_FILE;
	continue;
      } else {
	ungetc(c, input);
      }

      char *word = NULL;
      get_word(&word, input);
      
      switch (next_word_meaning) {
      case COMMAND_NAME:
	command->name = strdup(word);
	break;
      case ARG:
	str_list_append_item(command->args, word);
	break;
      case OUTPUT_FILE:
	command->out_file_name = strdup(word);
      }
      
      free(word);
      next_word_meaning = ARG;
    }
    
    if (command->name == NULL) {
      str_list_free(command->args);
    } else {
      return command;
    }
  }
  
  return NULL;
}
