#pragma once
#include <stdbool.h>
#define COMMAND_BUFFER_SIZE 256

typedef struct {
  const char *name;
  void (*command_function)(void *arg);
  void *arg;
} Command_t;

typedef struct command_buffer {
  char buffer[COMMAND_BUFFER_SIZE];
  int length;
} CommandBuffer_t;

bool command_buffer_update(CommandBuffer_t *cb);
