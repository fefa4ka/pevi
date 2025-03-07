#include "buffer.h"
#include "config.h"
#include "error.h"
#include "logger.h"
#include "memory.h"
#include <lr_file.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct Buffer *buffer_init(size_t size) {
  LOG_DEBUG("Initializing buffer with size %zu", size);
  
  struct Buffer *buffer = MALLOC(sizeof(struct Buffer));
  if (!buffer) {
    LOG_ERROR("Failed to allocate buffer");
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate buffer");
    return NULL;
  }
  
  // Initialize buffer fields
  buffer->type = PEVI_BUF_TEXT;
  buffer->path = NULL;
  buffer->fp = NULL;
  buffer->size = 0;
  buffer->cells = NULL;
  
  LOG_TRACE("Buffer structure allocated and initialized");
  
  if (size) {
    LOG_DEBUG("Allocating cells for buffer: %zu cells", size);
    struct lr_cell *cells = MALLOC(sizeof(struct lr_cell) * size);
    if (!cells) {
      LOG_ERROR("Failed to allocate buffer cells");
      ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate buffer cells");
      FREE(buffer);
      return NULL;
    }
    
    LOG_DEBUG("Initializing linked ring with %zu cells", size);
    lr_result_t result = lr_init(&buffer->lr, size, cells);
    if (result != LR_SUCCESS) {
      LOG_ERROR("Failed to initialize linked ring: result code %d", result);
      ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_ERROR, "Failed to initialize linked ring");
      FREE(cells);
      FREE(buffer);
      return NULL;
    }
    
    buffer->cells = cells;
    buffer->size = size;
    LOG_DEBUG("Buffer initialized with %zu cells", size);
  } else {
    LOG_DEBUG("Buffer initialized without cells (size 0)");
  }

  return buffer;
}

Buffer_t *buffer_open(void *filename) {
  if (!filename) {
    LOG_ERROR("Null filename parameter");
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null filename parameter");
    return NULL;
  }
  
  LOG_INFO("Opening buffer for file: %s", (char*)filename);
  
  FILE *file;
  Buffer_t *buffer;
  struct lr_cell *owner_cell;

  buffer = buffer_init(0);
  if (!buffer) {
    LOG_ERROR("Failed to initialize buffer");
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to initialize buffer");
    return NULL;
  }
  
  buffer->type = PEVI_BUF_FILE;
  LOG_DEBUG("Setting buffer type to PEVI_BUF_FILE");
  
  buffer->path = STRDUP(filename);
  if (!buffer->path) {
    LOG_ERROR("Failed to duplicate filename");
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to duplicate filename");
    buffer_free(buffer);
    return NULL;
  }
  LOG_DEBUG("Duplicated filename: %s", buffer->path);

  LOG_DEBUG("Opening file in linked ring: %s", buffer->path);
  lr_result_t result = lr_file_open(&buffer->lr, buffer->path, 0);
  if (result != LR_SUCCESS) {
    LOG_ERROR("Failed to open file in linked ring: %s (result code %d)", buffer->path, result);
    ERROR_SET(ERROR_FILE_ACCESS, ERROR_ERROR, "Failed to open file in linked ring");
    buffer_free(buffer);
    return NULL;
  }
  LOG_INFO("Successfully opened buffer for file: %s", buffer->path);

  return buffer;
}

bool buffer_save(Buffer_t *buffer, char *filename) {
  if (!buffer) {
    LOG_ERROR("Null buffer parameter");
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null buffer parameter");
    return false;
  }
  
  char *path = filename;
  if (path == NULL) {
    if (!buffer->path) {
      LOG_ERROR("No filename specified and buffer has no path");
      ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "No filename specified and buffer has no path");
      return false;
    }
    path = buffer->path;
  }
  
  LOG_INFO("Saving buffer to file: %s", path);
  lr_result_t result = lr_file_save(&buffer->lr, path);
  if (result != LR_SUCCESS) {
    LOG_ERROR("Failed to save buffer to file: %s (result code %d)", path, result);
    ERROR_SET(ERROR_FILE_ACCESS, ERROR_ERROR, "Failed to save buffer to file");
    return false;
  }
  
  LOG_INFO("Successfully saved buffer to file: %s", path);
  return true;
}

void buffer_free(Buffer_t *buffer) {
  if (!buffer) {
    LOG_WARNING("Attempt to free NULL buffer");
    return;
  }
  
  LOG_DEBUG("Freeing buffer resources");
  
  // Free the path string if it exists
  if (buffer->path) {
    LOG_TRACE("Freeing buffer path: %s", buffer->path);
    FREE(buffer->path);
    buffer->path = NULL;
  }
  
  // Close the file if it's open
  if (buffer->fp) {
    LOG_TRACE("Closing buffer file");
    fclose(buffer->fp);
    buffer->fp = NULL;
  }
  
  // Free the cells array if it exists
  if (buffer->cells) {
    LOG_TRACE("Freeing buffer cells array");
    FREE(buffer->cells);
    buffer->cells = NULL;
  }
  
  // Free the buffer structure itself
  LOG_TRACE("Freeing buffer structure");
  FREE(buffer);
  LOG_DEBUG("Buffer freed successfully");
}
