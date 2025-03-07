#include "buffer.h"
#include "config.h"
#include <lr_file.h>
#include <stdlib.h>
#include <string.h>

struct Buffer *buffer_init(size_t size) {
  struct Buffer *buffer = malloc(sizeof(struct Buffer));
  if (size) {
    struct lr_cell *cells = malloc(sizeof(struct lr_cell) * size);
    lr_result_t result = lr_init(&buffer->lr, size, cells);
    buffer->size = size;
  }

  return buffer;
}

Buffer_t *buffer_open(void *filename) {
  FILE *file;
  Buffer_t *buffer;
  struct lr_cell *owner_cell;

  buffer = buffer_init(0);
  buffer->type = PEVI_BUF_FILE;
  buffer->path = strdup(filename);

  lr_file_open(&buffer->lr, buffer->path, 0);

  return buffer;
}

void buffer_save(Buffer_t *buffer, char *filename) {
  char *path = filename;
  if (path == NULL) {
    path = buffer->path;
  }
  lr_file_save(&buffer->lr, path);
  return;
}
