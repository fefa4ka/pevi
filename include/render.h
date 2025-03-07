#pragma once
#include "camera.h"
#include "phantom.h"

typedef void (*render_body_fn)(Pevi_t *pevi);

void render_frame(Pevi_t *pevi, Camera3D camera, render_body_fn Render3DBody,
                 render_body_fn Render2DBody);

void render_ui(Pevi_t *pevi);
void render_phantom(Phantom_t *phantom, Camera_t *camera, InputEvent_t *event);
