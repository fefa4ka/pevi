#pragma once

#include <eer.h>
#include <raylib.h>
#include <stdio.h>
#include <unistd.h>
#include <lr.h>
#include <sys/poll.h>

#define Terminal_new(instance)    eer(Terminal, instance)
#define Terminal(instance, props) eer_withprops(Terminal, instance, _(props))

typedef struct {
    FILE *fp;
    char   *command;
    struct linked_ring *buffer;
    void *parent;
    lr_owner_t owner;
    bool is_visible;
    Camera *camera;
    Vector3 pos;
    Vector2 angles;
    Vector3 size;
    Font font;
    float   font_size;
    float   spacing;
    float   line_spacing;
    Shader shader;
    Color tint;
    Color bg_color;

    struct {
        void (*hover)(eer_t *instance);
        void (*click)(eer_t *instance);
    } on;
} Terminal_props_t;

typedef struct {
fd_set fds;
int    fd;
struct pollfd _fds[2];

    char  *content;
    char  buffer[1024];
    size_t content_size;
    Vector3 pos;
    Vector3 size;
} Terminal_state_t;

eer_header(Terminal);
