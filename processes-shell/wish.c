#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>

typedef struct command {
  char *name;
  char **args;
} Command;

int consume_whitespace() {
  char c;

  while (1) {
    c = getc(stdin);
    
    if (!isspace(c)) {
      ungetc(c, stdin);
      return 0;
    } else if (c == '\n') {
      return 1;
    }
  }
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

Command get_command() {
  consume_whitespace();
  char *name = NULL;
  get_word(&name);

  size_t buff_size = 2;
  char **args = malloc(sizeof(char **) * buff_size);
  if (args == NULL) {
    fprintf(stderr, "failed to allocate memory for arguments: %s", strerror(errno));
    exit(1);
  }
  int i = 0;

  while (consume_whitespace() != 1) {
    if (i == buff_size - 1) {
      buff_size *= 2;
      args = realloc(args, sizeof(char **) * buff_size);
      
      if (args == NULL) {
	fprintf(stderr, "failed to allocate memory for arguments: %s", strerror(errno));
	exit(1);
      }
    }
    
    get_word(args + (i++));
  }

  args[i] = NULL;
  Command command = { name, args };

  return command;
}

int main(int argc, char *argv[]) {
  Command command = get_command();
  
  printf("command: %s\n", command.name);
  int i = 0;
  while(command.args[i] != NULL) {
    printf("arg: %s\n", command.args[i++]);
  }
}
