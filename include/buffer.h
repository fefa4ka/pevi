#pragma once
#include <lr.h>
#include <stdbool.h>
#include "error.h"

// Define LR_SUCCESS if not already defined
#ifndef LR_SUCCESS
#define LR_SUCCESS 0
#endif

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
bool buffer_save(Buffer_t *buffer, char *filename);
void buffer_free(Buffer_t *buffer);
