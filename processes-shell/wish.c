#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"
#include "linked_list.h"

// #define DEBUG
#define ERROR_MESSAGE "An error has occurred\n"

char *get_command_path(LinkedList *search_paths, Command *command) {
#ifdef DEBUG
  printf("looking for command in:\n");
  for (LinkedListNode *node = search_paths->start; node != NULL; node = node->next) {
    printf("- %s\n", node->value);
  }
#endif

  LinkedListNode *path;

  for (path = search_paths->start; path != NULL; path = path->next) {
    char *command_path = malloc((strlen(path->value) + strlen(command->name) + 2) * sizeof(char));
    if (command_path == NULL) {
      fprintf(stderr, "couldn't allocate memory for command path: %s", strerror(errno));
      exit(1);
    }
    strcpy(command_path, path->value);
    strcat(command_path, "/");
    strcat(command_path, command->name);

    if (access(command_path, X_OK) == 0) {
      return command_path;
    }

    free(command_path);
  }

  return NULL;
}

char **get_command_args_array(Command *command) {
  char **array = malloc((command->args->len + 2) * sizeof(char *));
  array[0] = command->name;
  LinkedListNode *node;
  int i;

  for (i = 1, node = command->args->start; node != NULL; i++, node = node->next) {
    array[i] = node->value;
  }

  array[i] = NULL;

  return array;
}

int setup_output(Command *command) {
  if (command->out_file_name == NULL) {
    return 0;
  }

  FILE *out_file = fopen(command->out_file_name, "w");
  if (out_file == NULL) {
    fprintf(stderr, "error while opening file: %s\n", strerror(errno));
    return 1;
  }
  int out_fd = fileno(out_file);

  close(STDOUT_FILENO);
  dup2(out_fd, STDOUT_FILENO);

  close(STDERR_FILENO);
  dup2(out_fd, STDERR_FILENO);

  return 0;
}

void execute_system_command(LinkedList *search_paths, Command *command) {
  char *command_path = get_command_path(search_paths, command);
  if (command_path == NULL) {
    fprintf(stderr, ERROR_MESSAGE);
    return;
  }

  char **args_array = get_command_args_array(command);

#ifdef DEBUG
  printf("Command path: %s, %p\n", command_path, command_path);
  for (int i = 0; args_array[i] != NULL; i++) {
    printf("arg: %s, %p\n", args_array[i], args_array + i);
  }
  char *out_file_name = command->out_file_name;
  printf("Output path: %s\n", out_file_name == NULL ? "null" : out_file_name);
  printf("Is background: %d\n", command->is_background);
#endif

  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "failed to create a subprocess: %s", strerror(errno));
  } else if (rc == 0) {
    int rc = setup_output(command);
    if (rc != 0) {
      fprintf(stderr, ERROR_MESSAGE);
      return;
    }
    
    execv(command_path, args_array);
    fprintf(stderr, ERROR_MESSAGE);
  } else {
    free(command_path);
    free(args_array);
    if (!command->is_background) {
      waitpid(rc, NULL, 0);
    }
  }
}

void update_search_paths(LinkedList *search_paths, LinkedList *args) {
  linked_list_free_nodes(search_paths->start);
  search_paths->len = 0;
  search_paths->start = NULL;
  search_paths->end = NULL;
  LinkedListNode* arg;

  for (arg = args->start; arg != NULL; arg = arg->next) {
    linked_list_append_item(search_paths, strdup(arg->value));
  }
}

int execute_command(LinkedList *search_paths, Command *command) {
  if (strcmp(command->name, "exit") == 0) {
    if (command->args->len != 0) {
      fprintf(stderr, ERROR_MESSAGE);
      return 0;
    }
    
    return 1;
  } else if (strcmp(command->name, "path") == 0) {
    update_search_paths(search_paths, command->args);
  } else if (strcmp(command->name, "cd") == 0) {
    if (command->args->len != 1) {
      fprintf(stderr, ERROR_MESSAGE);
      return 0;
    }

    int rc = chdir(command->args->start->value);

    if (rc != 0) {
      fprintf(stderr, ERROR_MESSAGE);
    }
  } else {
    execute_system_command(search_paths, command);
  }

  return 0;
}

int main(int argc, char *argv[]) {
  FILE *input;
  int interactive;

  if (2 < argc) {
    fprintf(stderr, ERROR_MESSAGE);
    exit(1);
  } else if (argc == 2) {
    input = fopen(argv[1], "r");
    if (input == NULL) {
      fprintf(stderr, ERROR_MESSAGE);
      exit(1);
    }
    interactive = 0;
  } else {
    input = stdin;
    interactive = 1;
  }
  
  LinkedList *search_paths = linked_list_init();
  linked_list_append_item(search_paths, strdup("/bin"));
  int exited = 0;

  while (!exited) {
    CommandList *commands = command_list_init();
    int rc = get_next_commands(commands, input, interactive);
    if (rc == -1) {
      break;
    } else if (rc != 0) {
      fprintf(stderr, ERROR_MESSAGE);
      command_list_free(commands);
      continue;
    }

    if (commands->len == 0) {
      command_list_free(commands);
      continue;
    }

    CommandListNode *c;
    for (c = commands->start; c != NULL; c = c->next) {
      Command *command = c->value;
      rc = execute_command(search_paths, command);

      if (rc != 0) {
	exited = 1;
	break;
      }
    }

    command_list_free(commands);
  }

  while (wait(NULL) != -1) {
    // wait for next process
  }
  linked_list_free(search_paths);
  return 0;
}
