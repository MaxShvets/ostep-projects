#include <stdio.h>
#include <stdlib.h>
#define LINE_SIZE 100

void output_file(char *filename) {
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    printf("wcat: cannot open file\n");
    exit(1);
  }

  char line[LINE_SIZE];

  while (fgets(line, LINE_SIZE, file)) {
    printf("%s", line);
  }

  fclose(file);
}

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    output_file(argv[i]);
  }
}
