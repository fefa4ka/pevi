#pragma once
#include "window.h"

typedef struct {
  Camera3D camera;
  CameraMode mode;

  bool is_movable;
  bool is_mouse_enabled;
  bool is_enabled;

  Ray ray;
  Ray ray_center;
} Camera_t;

void camera_handle(Camera_t *settings);
Plane camera_plane(Camera *camera);
