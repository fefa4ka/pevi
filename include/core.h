#pragma once
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include "command.h"

typedef struct {
  Vector3 pos;
  Vector3 angles;
} Plane;

enum pevi_mode {
  PEVI_MODE_FREE,
  PEVI_MODE_EDIT,
  PEVI_MODE_COMMAND,
  PEVI_MODE_DRAG_START,
  PEVI_MODE_DRAG_ON,
};

typedef struct {
  enum pevi_mode mode;
  bool is_fps_visible;
  bool is_closed;
  CommandBuffer_t command_buffer;
} Pevi_t;
           
void core_init();
