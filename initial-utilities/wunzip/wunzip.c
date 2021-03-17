#include <stdio.h>
#include <stdlib.h>

void output_run(int run_length, char run_char) {
  for (int i = 0; i < run_length; i++) {
    printf("%c", run_char);
  }
}

void output_decompressed_file(char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("wunzip: can't open file");
    exit(1);
  }

  int run_length = 0;
  char run_char = '\0';
  int amount_read = 0;

  while (1) {
    amount_read = fread(&run_length, sizeof(int), 1, file);
    if (amount_read != 1) {
      break;
    }
    amount_read = fread(&run_char, sizeof(char), 1, file);
    output_run(run_length, run_char);
  }

  fclose(file);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("wunzip: file1 [file2 ...]\n");
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    output_decompressed_file(argv[i]);
  }
}
