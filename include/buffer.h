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
} Buffer_t;

Buffer_t *buffer_open(void *filename);
void buffer_save(Buffer_t *bufferm, char *filename);
