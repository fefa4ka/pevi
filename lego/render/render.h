#pragma once
#include <camera.h>

typedef void (*render_body_fn)();

void render_frame(Camera3D camera, render_body_fn Render3DBody,
                 render_body_fn Render2DBody);
