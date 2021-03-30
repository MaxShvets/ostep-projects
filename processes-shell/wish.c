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

void str_list_free_nodes(StringListNode *start) {
  StringListNode *node = start;

  while (node != NULL) {
    StringListNode *next_node = node->next;
    free(node->str);
    free(node);
    node = next_node;
  }
}

void str_list_free(StringList *list) {
  str_list_free_nodes(list->start);
  free(list);
}

void str_list_overwrite(StringList *src, StringList *dest) {
  str_list_free_nodes(dest->start);
  dest->len = 0;
  dest->start = NULL;
  dest->end = NULL;
  StringListNode* src_node;

  for (src_node = src->start; src_node != NULL; src_node = src_node->next) {
    str_list_append_item(dest, src_node->str);
  }
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
    free(word);
  }

  Command command = { name, args_list };

  return command;
}

char *get_command_path(StringList *search_paths, Command command) {
  StringListNode *path;

  for (path = search_paths->start; path != NULL; path = path->next) {
    char *command_path = malloc((strlen(path->str) + strlen(command.name) + 2) * sizeof(char));
    if (command_path == NULL) {
      fprintf(stderr, "couldn't allocate memory for command path: %s", strerror(errno));
      exit(1);
    }
    strcpy(command_path, path->str);
    strcat(command_path, "/");
    strcat(command_path, command.name);

    if (access(command_path, X_OK) == 0) {
      return command_path;
    }
  }

  return NULL;
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

void execute_system_command(StringList *search_paths, Command command) {
  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "failed to create a subprocess: %s", strerror(errno));
  } else if (rc == 0) {
    char *command_path = get_command_path(search_paths, command);
    if (command_path == NULL) {
      printf("something went wrong\n");
      return;
    }

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

int execute_command(StringList *search_paths, Command command) {
#ifdef DEBUG
  printf("paths:\n");
  for (StringListNode *node = search_paths->start; node != NULL; node = node->next) {
    printf("- %s\n", node->str);
  }
#endif

  if (strcmp(command.name, "exit") == 0) {
    return 1;
  } else if (strcmp(command.name, "path") == 0) {
    str_list_overwrite(command.args, search_paths);
  } else {
    execute_system_command(search_paths, command);
  }

  return 0;
}

int main(int argc, char *argv[]) {
  Command command;
  StringList *search_paths = str_list_init();
  str_list_append_item(search_paths, "/bin");

  while (1) {
    command = get_command();
    int rc = execute_command(search_paths, command);
    clear_command(command);

    if (rc != 0) {
      break;
    }
  }

  str_list_free(search_paths);
}
