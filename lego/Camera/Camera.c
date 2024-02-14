#include "Camera.h"
#include <eers.h>
#include <raymath.h>


///
/// \brief
///
WILL_MOUNT(Camera)
{
    state->camera          = (Camera){0};
    state->camera.position = props->position;
    state->camera.target   = props->target;
    state->camera.up       = props->up;

    state->camera.fovy       = props->fovy;
    state->camera.projection = props->projection;
}

///
/// \brief
///
SHOULD_UPDATE(Camera)
{
    Vector3 movement = {0};
    if (props->is_movable) {

        float distance_factor
            = Vector3Distance(state->camera.position, state->camera.target);
        float speed_factor
            = 0.01f
              + 0.003f * distance_factor; // Adjust the coefficients as needed

        movement = (Vector3){
            (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * speed_factor
                - // Move forward-backward
                (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * speed_factor,
            (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * speed_factor
                - // Move right-left
                (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * speed_factor,
            0.0f // Move up-down
        };
    }
    UpdateCameraPro(&state->camera, movement,
                    (Vector3){
                        GetMouseDelta().x * 0.2f, // Rotation: yaw
                        GetMouseDelta().y * 0.2f, // Rotation: pitch
                        0.0f                      // Rotation: roll
                    },
                    GetMouseWheelMove() * 2.0f); // Move to target (zoom)
    return true;
}

///
/// \brief
///
WILL_UPDATE(Camera) { BeginMode3D(state->camera); }

///
/// \brief
///
RELEASE(Camera) {}

DID_MOUNT_SKIP(Camera);
DID_UPDATE(Camera) {}

