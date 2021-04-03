#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "command.h"

void clear_command(Command command) {
  free(command.name);
  str_list_free(command.args);
  free(command.out_file_name);
}

size_t get_word(char **wordp) {
  size_t word_size = 2;
  *wordp = malloc(sizeof(char) * word_size);
  if (*wordp == NULL) {
    fprintf(stderr, "failed to allocate memory for a word: %s", strerror(errno));
    exit(1);
  }

  int i = 0;
  char c;

  while ((c = getc(stdin))) {
    if (i == word_size - 1) {
      word_size *= 2;
      *wordp = realloc(*wordp, word_size * sizeof(char));

      if (*wordp == NULL) {
	fprintf(stderr, "failed to allocate memory for a word: %s", strerror(errno));
	exit(1);
      }
    } 

    if (isspace(c)) {
      ungetc(c, stdin);
      break;
    }

    (*wordp)[i++] = c;
  }

  (*wordp)[i] = '\0';

  return i;
}

enum WordMeaning {COMMAND_NAME, ARG, OUTPUT_FILE};

int consume_whitespace() {
  char c = getc(stdin);

  while (isspace(c) && c != '\n') {
    c = getc(stdin);
  }

  return c;
}

Command get_command() {
  printf("wish> ");
  enum WordMeaning next_word_meaning = COMMAND_NAME;
  Command command = { NULL, str_list_init(), NULL };

  while (1) {
    char c = consume_whitespace();
    if (c == '\n') {
      break;
    } else if (c == '>') {
      next_word_meaning = OUTPUT_FILE;
      continue;
    } else {
      ungetc(c, stdin);
    }
    
    char *word = NULL;
    get_word(&word);

    switch (next_word_meaning) {
    case COMMAND_NAME:
      command.name = strdup(word);
      break;
    case ARG:
      str_list_append_item(command.args, word);
      break;
    case OUTPUT_FILE:
      command.out_file_name = strdup(word);
    }

    free(word);
    next_word_meaning = ARG;
  }

  return command;
}
