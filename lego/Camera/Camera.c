#include "Camera.h"
#include <eers.h>


///
/// \brief 
///
WILL_MOUNT(Camera) {
	state->camera = (Camera){0};
	state->camera.position = props->position;
	state->camera.target = props->target;
	state->camera.up = props->up;

	state->camera.fovy = props->fovy;
	state->camera.projection = props->projection;
}

///
/// \brief
///
SHOULD_UPDATE(Camera){
    UpdateCameraPro(&state->camera,
                    (Vector3){
                        (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * 0.01f
                            - // Move forward-backward
                            (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 0.01f,
                        (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 0.01f
                            - // Move right-left
                            (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 0.01f,
                        0.0f // Move up-down
                    },
                    (Vector3){
                        GetMouseDelta().x * 0.2f, // Rotation: yaw
                        GetMouseDelta().y * 0.2f, // Rotation: pitch
                        0.0f                       // Rotation: roll
                    },
                    GetMouseWheelMove() * 2.0f); // Move to target (zoom)
						 
	return true;
}

///
/// \brief
///
WILL_UPDATE(Camera) {
    BeginMode3D(state->camera);
}

///
/// \brief
///
RELEASE(Camera) {
}

DID_MOUNT_SKIP(Camera);
DID_UPDATE(Camera) {
    EndMode3D();
}

