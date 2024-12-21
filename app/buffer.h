#pragma once
#include <lr.h>

typedef enum { PEVI_BUF_TEXT, PEVI_BUF_FILE, PEVI_BUF_TERMINAL } Buffer_mode;

typedef struct Buffer {
  Buffer_mode type;
  char *path;

  FILE *fp;
  size_t size;
  struct linked_ring lr;
  struct lr_cell *cells;

  struct Buffer *prev;
  struct Buffer *next;
} Buffer_t;

Buffer_t *file_open(void *filename);
