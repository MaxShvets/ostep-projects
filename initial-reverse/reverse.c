#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/errno.h>

struct LineListNode {
  char *line;
  struct LineListNode *prev_node;
};

FILE *open_file(char *filename, char *mode) {
  FILE *file = fopen(filename, mode);
  if (file == NULL) {
    fprintf(stderr, "reverse: cannot open file '%s'\n", filename);
    exit(1);
  }

  return file;
}

FILE *get_in_file(int argc, char *argv[]) {
  if (argc < 2) {
    return stdin;
  }

  return open_file(argv[1], "r");
}

FILE *get_out_file(int argc, char *argv[]) {
  if (argc < 3) {
    return stdout;
  }

  return open_file(argv[2], "w");
}

int are_files_same(char* filename, char* other_filename) {
  struct stat file_stat, other_file_stat;
  
  if (stat(filename, &file_stat) < 0) {
    fprintf(stderr, "reverse: cannot open file '%s'\n", filename);
    exit(1);
  }

  if (stat(other_filename, &other_file_stat) < 0) {
    if (errno == ENOENT) {
      return 0;
    }
    fprintf(stderr, "reverse: cannot open file  '%s'\n", other_filename);
    exit(1);
  }
  
  return (file_stat.st_dev == other_file_stat.st_dev) && (file_stat.st_ino == other_file_stat.st_ino);
}

int main(int argc, char *argv[]) {
  if (3 < argc) {
    fprintf(stderr, "usage: reverse <input> <output>\n");
    exit(1);
  }

  if (argc == 3 && are_files_same(argv[1], argv[2])) {
    fprintf(stderr, "reverse: input and output file must differ\n");
    exit(1);
  }

  FILE *in_file = get_in_file(argc, argv);
  FILE *out_file = get_out_file(argc, argv);

  struct LineListNode *node = NULL;

  while (1) {
    struct LineListNode *new_node = malloc(sizeof(struct LineListNode));
    new_node->prev_node = node;
    new_node->line = NULL;
    size_t linecap = 0;
    ssize_t line_size = getline(&(new_node->line), &linecap, in_file);

    if (line_size <= 0) {
      break;
    }

    node = new_node;
  }

  while (node != NULL) {
    fprintf(out_file, "%s", node->line);
    node = node->prev_node;
  }
}
