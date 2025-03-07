#include "buffer.h"
#include "config.h"
#include "error.h"
#include "memory.h"
#include <lr_file.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct Buffer *buffer_init(size_t size) {
  struct Buffer *buffer = MALLOC(sizeof(struct Buffer));
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
    struct lr_cell *cells = MALLOC(sizeof(struct lr_cell) * size);
    if (!cells) {
      ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate buffer cells");
      FREE(buffer);
      return NULL;
    }
    
    lr_result_t result = lr_init(&buffer->lr, size, cells);
    if (result != LR_SUCCESS) {
      ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_ERROR, "Failed to initialize linked ring");
      FREE(cells);
      FREE(buffer);
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
  buffer->path = STRDUP(filename);
  if (!buffer->path) {
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to duplicate filename");
    buffer_free(buffer);
    return NULL;
  }

  lr_result_t result = lr_file_open(&buffer->lr, buffer->path, 0);
  if (result != LR_SUCCESS) {
    ERROR_SET(ERROR_FILE_ACCESS, ERROR_ERROR, "Failed to open file in linked ring");
    buffer_free(buffer);
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

void buffer_free(Buffer_t *buffer) {
  if (!buffer) return;
  
  // Free the path string if it exists
  if (buffer->path) {
    FREE(buffer->path);
    buffer->path = NULL;
  }
  
  // Close the file if it's open
  if (buffer->fp) {
    fclose(buffer->fp);
    buffer->fp = NULL;
  }
  
  // Free the cells array if it exists
  if (buffer->cells) {
    FREE(buffer->cells);
    buffer->cells = NULL;
  }
  
  // Free the buffer structure itself
  FREE(buffer);
}
