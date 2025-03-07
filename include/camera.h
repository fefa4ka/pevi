#pragma once
#include "window.h"
#include "core.h"

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
void camera_set_mode(Camera_t *camera, enum pevi_mode editor_mode);
Plane camera_plane(Camera3D *camera);
