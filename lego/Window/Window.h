#pragma once

#include <eer.h>
#include <raylib.h>

#define Window_new(instance)    eer(Window, instance)
#define Window(instance, props) eer_withprops(Window, instance, _(props))

typedef struct {
    char   *title;
    unsigned int width;
    unsigned int height;
    Camera *camera;

    struct {
        void (*before)(eer_t *instance);
        void (*after)(eer_t *instance);
    } on;
} Window_props_t;

typedef struct {
    Vector3 pos;
} Window_state_t;

eer_header(Window);
