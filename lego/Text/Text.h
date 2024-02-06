#pragma once

#include <eer.h>
#include <raylib.h>

#define Text_new(instance)    eer(Text, instance)
#define Text(instance, props) eer_withprops(Text, instance, _(props))

typedef struct {
    char   *content;
    float   font_size;
    Vector3 pos;
} Text_props_t;

typedef struct {
    Vector3 size;
    Vector3 pos;
} Text_state_t;

eer_header(Text);
