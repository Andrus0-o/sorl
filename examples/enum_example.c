#include <stdio.h>
#include <stdlib.h>

enum FileState {
  Closed = 0,
  Opened = 1,
};

typedef struct File {
  enum FileState file_state;
  FILE *handle;
} File;

void open(File *f, const char *path) {
  if (f->file_state == Opened) {
    fprintf(stderr, "State error\n");
    exit(EXIT_FAILURE);
  }
  f->handle = fopen(path, "r");

  if (f->handle == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  f->file_state = Opened;
}

void close(File *f) {
  if (f->file_state == Closed) {
    fprintf(stderr, "State error\n");
    exit(EXIT_FAILURE);
  }
  if (fclose(f->handle) != 0) {
    perror("fclose");
    exit(EXIT_FAILURE);
  }
  f->handle = NULL;
  f->file_state = Closed;
}

int read_char(File *f) {
  if (f->file_state == Closed) {
    fprintf(stderr, "State error\n");
    exit(EXIT_FAILURE);
  }
  return fgetc(f->handle);
}

int main() {
  File file = {
      .file_state = 0,
  };
  open(&file, "example.txt");
  int ch;
  while ((ch = read_char(&file)) != EOF)
    putchar(ch);
  close(&file);
  return 0;
}
