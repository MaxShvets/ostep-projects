#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include <unistd.h>

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
  printf("wish> ");
  consume_whitespace();
  char *name = NULL;
  get_word(&name);

  size_t buff_size = 4;
  char **args = malloc(sizeof(char **) * buff_size);
  args[0] = name;
  if (args == NULL) {
    fprintf(stderr, "failed to allocate memory for arguments: %s", strerror(errno));
    exit(1);
  }
  int i = 1;

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

void execute_system_command(Command command) {
  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "failed to create a subprocess: %s", strerror(errno));
  } else if (rc == 0) {
    char command_path[50] = "/bin/";
    strncat(command_path, command.name, 10);
    execv(command_path, command.args);
    fprintf(stderr, "something went wrong");
  } else {
    wait(NULL);
  }
}

void execute_command(Command command) {
  if (strcmp(command.name, "exit") == 0) {
    exit(0);
  } else {
    execute_system_command(command);
  }
}

int main(int argc, char *argv[]) {
  Command command;

  //char *args[3] = { "echo", "hey", "boys",  NULL };
  //execv("/bin/echo", args);

  while (1) {
    command = get_command();
    execute_command(command);
  }
}
