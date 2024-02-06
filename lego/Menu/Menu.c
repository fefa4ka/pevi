#include "Menu.h"
#include <string.h>
#include <eers.h>

WILL_MOUNT_SKIP(Menu);

SHOULD_UPDATE(Menu)
{
    /* Command not found */
    if (*props->command && strcmp(props->command, state->command) != 0) {
        return true;
    }

    return false;
}

WILL_UPDATE(Menu)
{
    Menu_command_t *command
        = state->current_menu && state->current_menu->menu
              ? state->current_menu->menu
              : props->menu;

    strcpy(state->command, props->command);
    while (command->command) {
        /* Compare command as prefix */
        if (memcmp(command->command, state->command, strlen(command->command))
            == 0) {
            state->previous_menu = state->current_menu;
            state->current_menu  = command;
            return;
        }
        command++;
    }

    if (props->on.not_found)
        props->on.not_found(self);

    state->current_menu = 0;
}

RELEASE(Menu)
{
    Menu_command_t *command = state->current_menu;
    if (command) {
        command->callback(command->args);
        if (props->on.command)
            props->on.command(self);
    }
}

DID_MOUNT_SKIP(Menu);

DID_UPDATE(Menu) { *state->command = 0; }
