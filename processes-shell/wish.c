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
#include "str_list.h"

// #define DEBUG
#define ERROR_MESSAGE "An error has occurred\n"

char *get_command_path(StringList *search_paths, Command *command) {
#ifdef DEBUG
  printf("looking for command in:\n");
  for (StringListNode *node = search_paths->start; node != NULL; node = node->next) {
    printf("- %s\n", node->str);
  }
#endif

  StringListNode *path;

  for (path = search_paths->start; path != NULL; path = path->next) {
    char *command_path = malloc((strlen(path->str) + strlen(command->name) + 2) * sizeof(char));
    if (command_path == NULL) {
      fprintf(stderr, "couldn't allocate memory for command path: %s", strerror(errno));
      exit(1);
    }
    strcpy(command_path, path->str);
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
  StringListNode *node;
  int i;

  for (i = 1, node = command->args->start; node != NULL; i++, node = node->next) {
    array[i] = node->str;
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

void execute_system_command(StringList *search_paths, Command *command) {
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
    wait(NULL);
    free(command_path);
    free(args_array);
  }
}

int execute_command(StringList *search_paths, Command *command) {
  if (strcmp(command->name, "exit") == 0) {
    if (command->args->len != 0) {
      fprintf(stderr, ERROR_MESSAGE);
      return 0;
    }
    
    return 1;
  } else if (strcmp(command->name, "path") == 0) {
    str_list_overwrite(command->args, search_paths);
  } else if (strcmp(command->name, "cd") == 0) {
    if (command->args->len != 1) {
      fprintf(stderr, ERROR_MESSAGE);
      return 0;
    }

    int rc = chdir(command->args->start->str);

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
    fprintf(stderr, "usage: wish [filename]\n");
    exit(1);
  } else if (argc == 2) {
    input = fopen(argv[1], "r");
    if (input == NULL) {
      fprintf(stderr, "couldn't open input file\n");
      exit(1);
    }
    interactive = 0;
  } else {
    input = stdin;
    interactive = 1;
  }
  
  Command *command;
  StringList *search_paths = str_list_init();
  str_list_append_item(search_paths, "/bin");

  while (1) {
    command = get_next_command(input, interactive);
    if (command == NULL) {
      break;
    }
    
    int rc = execute_command(search_paths, command);
    clear_command(command);

    if (rc != 0) {
      break;
    }
  }

  str_list_free(search_paths);
  return 0;
}
