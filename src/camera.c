#include "camera.h"
#include "error.h"
#include <raymath.h>
#include <stdio.h>
#include "logger.h"
#include "phantom.h"
#include "phantom_list.h"


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
    // Handle mouse wheel zoom separately to work in all modes
    float wheel_move = GetMouseWheelMove() * 2.0f;
    
    // In edit mode, only apply zoom (no movement or rotation)
    extern Pevi_t pevi;
    if (pevi.mode == PEVI_MODE_EDIT) {
      // Only apply zoom in edit mode
      UpdateCameraPro(camera, (Vector3){0}, (Vector3){0}, wheel_move);
    } else {
      // Normal camera update in other modes
      UpdateCameraPro(camera, movement,
                      (Vector3){dx, // Rotation: yaw
                                dy, // Rotation: pitch
                                0},
                      wheel_move); // Move to target (zoom)
    }
    
    // Update ray for collision detection
    settings->ray = GetMouseRay(GetMousePosition(), *camera);
    settings->ray_center = GetMouseRay(
        (Vector2){GetScreenWidth() / 2.0, GetScreenHeight() / 2.0}, *camera);
  }
}

// Set camera mode based on editor mode
void camera_set_mode(Camera_t *camera, enum pevi_mode editor_mode) {
  extern Pevi_t pevi;
  
  switch (editor_mode) {
    case PEVI_MODE_FREE:
      // Restore saved camera state if coming from edit mode
      if (pevi.mode == PEVI_MODE_EDIT) {
        camera->camera.position = camera->saved_state.position;
        camera->camera.target = camera->saved_state.target;
        camera->camera.projection = camera->saved_state.projection;
        camera->is_mouse_enabled = true;  // Ensure mouse is enabled
        LOG_INFO("Restored camera state from edit mode");
      }
      
      camera->is_enabled = true;
      camera->is_movable = true;
      camera->is_mouse_enabled = true;
      break;
      
    case PEVI_MODE_EDIT:
      // Save current camera state before switching to edit mode
      camera->saved_state.position = camera->camera.position;
      camera->saved_state.target = camera->camera.target;
      camera->saved_state.projection = camera->camera.projection;
      LOG_INFO("Saved camera state for edit mode");
      
      // Point camera at active phantom
      Phantom_t *active = phantom_list_get_active(pevi.phantoms);
      if (active) {
        // Position camera to face the phantom directly
        Vector3 phantom_pos = active->plane.pos;
        
        // Calculate the center of the phantom text area
        Vector4 phantom_size = phantom_measure(active);
        Vector3 phantom_center = {
          phantom_pos.x + phantom_size.x / 2.0f,
          phantom_pos.y,
          phantom_pos.z + phantom_size.z / 2.0f
        };
        
        Vector3 phantom_normal = {
          sinf(active->plane.angles.y) * cosf(active->plane.angles.x),
          sinf(active->plane.angles.x),
          cosf(active->plane.angles.y) * cosf(active->plane.angles.x)
        };
        
        // Position camera at a fixed distance from phantom center
        float distance = 10.0f;
        camera->camera.position = Vector3Add(phantom_center, Vector3Scale(phantom_normal, distance));
        camera->camera.target = phantom_center;
        camera->camera.projection = CAMERA_ORTHOGRAPHIC;
        
        LOG_INFO("Camera positioned to face phantom center in edit mode");
      }
      
      camera->is_enabled = true;  // Keep camera enabled for mouse wheel
      camera->is_movable = false;
      camera->is_mouse_enabled = false;  // Disable mouse look but keep wheel
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
