#include "camera.h"
#include <raymath.h>
#include <stdio.h>

void camera_handle(Camera_t *settings) {
  Camera3D *camera = &settings->camera;
  Vector3 movement = {0};

  if (settings->is_movable) {
    float distance_factor = Vector3Distance(camera->position, camera->target);
    float speed_factor =
        0.01f + 0.003f * distance_factor; // Adjust the coefficients as needed

    float move_updown = IsKeyDown(KEY_BACKSPACE);
    if (!move_updown && (IsKeyDown(KEY_ENTER)))
      move_updown = -1.;

    movement = (Vector3){
        (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) *
                speed_factor - // Move forward-backward
            (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * speed_factor,
        (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_SPACE)) *
                speed_factor - // Move right-left
            (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * speed_factor,
        (move_updown * speed_factor) // Move up-down
    };
  }
  float dx = 0;
  float dy = 0;

  if (settings->is_mouse_enabled) {
    dx = GetMouseDelta().x * 0.2;
    dy = GetMouseDelta().y * 0.2;
  }

  if (settings->is_movable) {
    if (!dx)
      dx = IsKeyDown(KEY_L) ? 1 : IsKeyDown(KEY_H) ? -1.0 : 0;
    if (!dy)
      dy = IsKeyDown(KEY_J) ? 1 : IsKeyDown(KEY_K) ? -1.0 : 0;
  }

  if (settings->is_enabled) {
    UpdateCameraPro(camera, movement,
                    (Vector3){dx, // Rotation: yaw
                              dy, // Rotation: pitch
                              0},
                    GetMouseWheelMove() * 2.0f); // Move to target (zoom)
    
    // Update ray for collision detection
    settings->ray = GetMouseRay(GetMousePosition(), *camera);
    settings->ray_center = GetMouseRay(
        (Vector2){GetScreenWidth() / 2.0, GetScreenHeight() / 2.0}, *camera);
  }
}

// Set camera mode based on editor mode
void camera_set_mode(Camera_t *camera, enum pevi_mode editor_mode) {
  switch (editor_mode) {
    case PEVI_MODE_FREE:
      camera->is_enabled = true;
      camera->is_movable = true;
      camera->is_mouse_enabled = true;
      camera->camera.projection = CAMERA_PERSPECTIVE;
      break;
      
    case PEVI_MODE_EDIT:
      camera->is_enabled = false;
      camera->is_movable = false;
      camera->camera.projection = CAMERA_ORTHOGRAPHIC;
      break;
      
    case PEVI_MODE_COMMAND:
      camera->is_enabled = false;
      camera->is_movable = false;
      break;
      
    default:
      // Keep current settings for other modes
      break;
  }
}

Vector3 camera_billboard_angles(Vector3 objectPosition, Vector3 cameraPosition,
                                Vector3 cameraUp) {
  Vector3 direction =
      Vector3Normalize(Vector3Subtract(cameraPosition, objectPosition));

  // Calculate yaw (horizontal rotation)
  float yaw = atan2f(direction.x, direction.z);

  // Calculate pitch (vertical rotation)
  // Find the angle between the direction vector and the vector pointing
  // directly up
  float pitch =
      acosf(Vector3DotProduct(direction, (Vector3){0.0f, 1.0f, 0.0f}));

  // Adjust pitch based on the camera's up vector
  float cameraUpAngle = atan2f(cameraUp.x, cameraUp.y);
  pitch *= cosf(cameraUpAngle);

  // Calculate roll
  Vector3 right = Vector3CrossProduct(direction, cameraUp);
  right = Vector3Normalize(right);
  float roll = atan2f(Vector3DotProduct(right, (Vector3){0.0f, 0.0f, 1.0f}),
                      Vector3DotProduct(right, (Vector3){0.0f, 1.0f, 0.0f}));

  return (Vector3){pitch, yaw, 0};
}

Plane camera_plane(Camera3D *camera) {
  Vector3 angles =
      camera_billboard_angles(camera->target, camera->position, camera->up);

  return (Plane){.pos = camera->target, .angles = angles};
}
