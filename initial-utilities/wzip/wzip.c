#include <stdio.h>
#include <stdlib.h>
#define BUFF_SIZE 100

void output_run(int run_length, char run_char) {
  fwrite(&run_length, sizeof(int), 1, stdout);
  fwrite(&run_char, sizeof(char), 1, stdout);
}

void output_compressed_file(char *filename, int *run_length, char *current_char) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("wzip: can't open file");
    exit(1);
  }

  char buff[BUFF_SIZE];
  size_t amount_read = 0;

  while((amount_read = fread(buff, sizeof(char), BUFF_SIZE, file))) {
    for (int i = 0; i < amount_read; i++) {
      char next_char = buff[i];
      if (next_char == *current_char) {
	(*run_length)++;
	continue;
      } 

      if (0 < *run_length) {
	output_run(*run_length, *current_char);
      }

      *current_char = next_char;
      *run_length = 1;
    }
  }

  fclose(file);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("wzip: file1 [file2 ...]\n");
    exit(1);
  }

  char current_char = '\0';
  int run_length = 0;

  for (int i = 1; i < argc; i++) {
    output_compressed_file(argv[i], &run_length, &current_char);
  }

  output_run(run_length, current_char);
}
