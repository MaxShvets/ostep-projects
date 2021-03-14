#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void print_matching_lines(FILE *file, char *searchterm) {
  char *line = NULL;
  size_t linecap = 0;
  
  while (0 < getline(&line, &linecap, file)) {
    if (strstr(line, searchterm)) {
      printf("%s", line);
    }
  }
}


int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("wgrep: searchterm [file ...]\n");
    exit(1);
  }

  char *searchterm = argv[1];

  if (argc == 2) {
    print_matching_lines(stdin, searchterm);
    return 0;
  }

  for (int i = 2; i < argc; i++) {
    FILE *file = fopen(argv[i], "r");
    if (file == NULL) {
      printf("wgrep: cannot open file\n");
      exit(1);
    }

    print_matching_lines(file, searchterm);
    fclose(file);
  }

  return 0;
}
