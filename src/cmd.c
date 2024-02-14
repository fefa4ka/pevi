#include "pevi.h"

void enable_command_mode(void *arg)
{
	state.mode = PEVI_MODE_COMMAND;
	state.cam.is_movable = false;
}



void set_fovy(void *fovy_str)
{
    int fovy;
    sscanf(fovy_str, "%*s %d", &fovy);
    state.cam.fovy = fovy;
}

void print_fps(void *arg) { state.is_fps_visible = !state.is_fps_visible; }

void on_command(eer_t *menu)
{
    *state.command = 0;

    state.mode           = PEVI_MODE_FREE;
    state.cam.is_movable = true;
}

void on_command_not_found(eer_t *menu)
{
    *state.command = 0;

    state.mode           = PEVI_MODE_FREE;
    state.cam.is_movable = true;
}

void           buffer_dump(void *command) { lr_dump(&state.cmd_buffer); }

void quit(void *command)
{
    CloseWindow();
    exit(0);
}


