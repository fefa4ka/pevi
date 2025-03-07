#include "buffer.h"
#include "config.h"
#include <lr_file.h>
#include <stdlib.h>
#include <string.h>

struct Buffer *buffer_init(size_t size) {
  struct Buffer *buffer = malloc(sizeof(struct Buffer));
  if (!buffer) {
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate buffer");
    return NULL;
  }
  
  // Initialize buffer fields
  buffer->type = PEVI_BUF_TEXT;
  buffer->path = NULL;
  buffer->fp = NULL;
  buffer->size = 0;
  buffer->cells = NULL;
  
  if (size) {
    struct lr_cell *cells = malloc(sizeof(struct lr_cell) * size);
    if (!cells) {
      ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate buffer cells");
      free(buffer);
      return NULL;
    }
    
    lr_result_t result = lr_init(&buffer->lr, size, cells);
    if (result != LR_SUCCESS) {
      ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_ERROR, "Failed to initialize linked ring");
      free(cells);
      free(buffer);
      return NULL;
    }
    
    buffer->cells = cells;
    buffer->size = size;
  }

  return buffer;
}

Buffer_t *buffer_open(void *filename) {
  if (!filename) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null filename parameter");
    return NULL;
  }
  
  FILE *file;
  Buffer_t *buffer;
  struct lr_cell *owner_cell;

  buffer = buffer_init(0);
  if (!buffer) {
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to initialize buffer");
    return NULL;
  }
  
  buffer->type = PEVI_BUF_FILE;
  buffer->path = strdup(filename);
  if (!buffer->path) {
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to duplicate filename");
    free(buffer);
    return NULL;
  }

  lr_result_t result = lr_file_open(&buffer->lr, buffer->path, 0);
  if (result != LR_SUCCESS) {
    ERROR_SET(ERROR_FILE_ACCESS, ERROR_ERROR, "Failed to open file in linked ring");
    free(buffer->path);
    free(buffer);
    return NULL;
  }

  return buffer;
}

bool buffer_save(Buffer_t *buffer, char *filename) {
  if (!buffer) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null buffer parameter");
    return false;
  }
  
  char *path = filename;
  if (path == NULL) {
    if (!buffer->path) {
      ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "No filename specified and buffer has no path");
      return false;
    }
    path = buffer->path;
  }
  
  lr_result_t result = lr_file_save(&buffer->lr, path);
  if (result != LR_SUCCESS) {
    ERROR_SET(ERROR_FILE_ACCESS, ERROR_ERROR, "Failed to save buffer to file");
    return false;
  }
  
  return true;
}
