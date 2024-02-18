#pragma once

#include <eer.h>
#include <raylib.h>
#include <lr.h>

#define Text_new(instance)    eer(Text, instance)
#define Text(instance, props) eer_withprops(Text, instance, _(props))

typedef struct {
    Font font;
    Shader shader;
    Color tint;
    Color bg_color;
    char   *content;
    void *parent;
    lr_owner_t owner;
    void *parent_arg;
    Vector3 pos;
    Vector2 angles;
    float   font_size;
    float   spacing;
    float   line_spacing;
    Camera *camera;
    bool is_selected;
    struct {
        void (*hover)(eer_t *instance);
        void (*click)(eer_t *instance);
        void (*cursor)(eer_t *instance);
    } on;
} Text_props_t;

typedef struct {
    Vector3 pos;
    Vector3 size;
    float scale;
    bool is_selected;
} Text_state_t;

eer_header(Text);
