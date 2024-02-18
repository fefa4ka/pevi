#pragma once

#include <eer.h>
#include <raylib.h>

#define Camera_new(instance)    eer(Camera, instance)
#define Camera_add(instance, props) eer_withprops(Camera, instance, _(props))

typedef struct {
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fovy;
    int projection;
    int mode;
    bool is_movable;
    bool is_mouse_enabled;
    bool is_enabled;
} Camera_props_t;

typedef struct {
	Camera3D camera;
} Camera_state_t;

eer_header(Camera);
