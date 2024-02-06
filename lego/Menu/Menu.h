#pragma once

#include <eer.h>

#define Menu(instance, props) eer_withprops(Menu, instance, _(props))

typedef struct Menu_command {
    const char *command;
    void (*callback)(void *instance);
    void *args;

    struct Menu_command *menu;
} Menu_command_t;

typedef struct {
    Menu_command_t *menu;
    const char *command;
    struct {
        void (*command)(eer_t *instance);
        void (*not_found)(eer_t *instance);
    } on;
} Menu_props_t;

typedef struct {
    Menu_command_t *previous_menu;
    Menu_command_t *current_menu;
    char command[32];
} Menu_state_t;


eer_header(Menu);
