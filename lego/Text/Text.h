#pragma once

#include <eer.h>
#include <raylib.h>

#define Text_new(instance)    eer(Text, instance)
#define Text(instance, props) eer_withprops(Text, instance, _(props))

typedef struct {
    Font font;
    Color tint;
    char   *content;
    float   font_size;
    float   spacing;
    Vector3 pos;
    Vector3 absolute_pos;
    Camera *camera;
} Text_props_t;

typedef struct {
    Vector3 size;
    Vector3 pos;
} Text_state_t;

eer_header(Text);
