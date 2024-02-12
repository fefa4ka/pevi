#pragma once

#include <Camera.h>
#include <Clock.h>
#include <Menu.h>
#include <Serial.h>
#include <Symbol.h>
#include <Text.h>
#include <Window.h>
#include <eer_app.h>
#include <lr.h>
#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>
#include <rlgl.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "pemath.h"
#include "cmd.h"
#include "input.h"

enum pevi_mode { PEVI_MODE_FREE, PEVI_MODE_EDIT, PEVI_MODE_COMMAND };

struct Pevi {
    enum pevi_mode mode;
    bool is_fps_visible;
    Camera_props_t cam;

    char command[COMMAND_BUFFER_SIZE];
    char key_buffer[COMMAND_BUFFER_SIZE];

    struct linked_ring cmd_buffer;
    struct lr_cell  cmd_cells[COMMAND_BUFFER_SIZE];
};

struct Cursor {
	Vector3 pos;
	Vector2 angles;
	Color tint;
};

extern struct Pevi state;
