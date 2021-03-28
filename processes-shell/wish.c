#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DEBUG

typedef struct str_list_node {
  char *str;
  struct str_list_node *next;
} StringListNode;

typedef struct str_list {
  int len;
  StringListNode *start;
  StringListNode *end;
} StringList;

StringList* str_list_init() {
  StringList *list = malloc(sizeof(StringList));
  list->len = 0;
  list->start = NULL;
  list->end = NULL;
  return list;
}

int str_list_append_item(StringList *list, char *str) {
  StringListNode *node = malloc(sizeof(StringListNode));
  node->str = strdup(str);
  node->next = NULL;
  
  if (list->start == NULL) {
    list->start = node;
  } else {
    list->end->next = node;
  }

  list->end = node;

  return ++(list->len);
}

void str_list_free(StringList *list) {
  StringListNode *node = list->start;
  
  while (node != NULL) {
    StringListNode *next_node = node->next;
    free(node->str);
    free(node);
    node = next_node;
  }

  free(list);
}

typedef struct command {
  char *name;
  StringList *args;
} Command;

void clear_command(Command command) {
  free(command.name);
  str_list_free(command.args);
}

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

  StringList *args_list = str_list_init();

  while (consume_whitespace() != 1) {
    char *word = NULL;
    get_word(&word);
    str_list_append_item(args_list, word);
  }

  Command command = { name, args_list };

  return command;
}

char *get_command_path(Command command) {
  char base[6] = "/bin/";
  char *path = malloc((strlen(base) + strlen(command.name) + 1) * sizeof(char));
  if (path == NULL) {
    fprintf(stderr, "couldn't allocate memory for command path: %s", strerror(errno));
    exit(1);
  }
  strcpy(path, base);
  strcat(path, command.name);

  return path;
}

char **get_command_args_array(Command command) {
  char **array = malloc((command.args->len + 2) * sizeof(char *));
  array[0] = command.name;
  StringListNode *node;
  int i;
  
  for (i = 1, node = command.args->start; node != NULL; i++, node = node->next) {
    array[i] = node->str;
  }

  array[i] = NULL;

  return array;
}

void execute_system_command(Command command) {
  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "failed to create a subprocess: %s", strerror(errno));
  } else if (rc == 0) {
    char *command_path = get_command_path(command);
    char **args_array = get_command_args_array(command);
    
#ifdef DEBUG
    printf("Command path: %s, %p\n", command_path, command_path);
    for (int i = 0; args_array[i] != NULL; i++) {
      printf("arg: %s, %p\n", args_array[i], args_array + i);
    }
#endif
    
    execv(command_path, args_array);
    fprintf(stderr, "something went wrong\n");
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

  while (1) {
    command = get_command();
    execute_command(command);
    clear_command(command);
  }
}
