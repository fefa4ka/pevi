#include "Terminal.h"
#include <Text.h>
#include <eers.h>
#include <raymath.h>

void swap_pollfd(struct pollfd *a, struct pollfd *b)
{
    if (a == b)
        return;
    struct pollfd tmp = *a;
    *a                = *b;
    *b                = tmp;
}
///
/// \brief
///

#include <fcntl.h>
WILL_MOUNT(Terminal)
{
    state->fd        = fileno(props->fp);
    state->buffer[0] = '\0';

    state->_fds[0].fd     = STDIN_FILENO;
    state->_fds[0].events = POLLIN; // Wait for input on stdin
    state->_fds[1].fd     = state->fd;
    state->_fds[1].events = POLLIN; // Wait for input on command output
    fcntl(state->_fds[0].fd, F_SETFL,
          fcntl(state->_fds[0].fd, F_GETFL) | O_NONBLOCK);
    fcntl(state->_fds[1].fd, F_SETFL,
          fcntl(state->_fds[1].fd, F_GETFL) | O_NONBLOCK);
}

///
/// \brief
///
SHOULD_UPDATE(Terminal) { return true; }

///
/// \brief
///
WILL_UPDATE(Terminal)
{
    char buffer[BUFSIZ];

    int ret = poll(state->_fds, 2, 1); // Wait indefinitely for events

    if (ret == 0 || ret == -1) {
        return;
    }

    struct pollfd *curr = &state->_fds[1];


    if (state->_fds[1].revents & POLLIN) { // Input available on stdin
        ssize_t bytes_read = read(state->fd, buffer, BUFSIZ);
        if (bytes_read <= 0)
            return;

        for (ssize_t i = 0; i < bytes_read; i++) {
            lr_put(props->buffer, (char)buffer[i], lr_owner(props->owner));
        }
    }
}

///
/// \brief
///
RELEASE(Terminal)
{
    Text_new(txt);

    eer_init();

    shoot(Text, txt,
          _({.font      = props->font,
             .spacing   = 0.6f,
             .tint      = BLACK,
             .content   = props->command,
             .parent    = (void *)props->buffer,
             .owner     = props->owner,
             .font_size = 24.,
             .angles    = props->angles,
             .pos       = props->pos,
             .camera    = props->camera,
             .bg_color  = props->bg_color,
             .shader    = props->shader, 
	     .on.hover = props->on.hover,
	     }));


    Matrix transform;
    transform = MatrixTranslate(props->pos.x, props->pos.y, props->pos.z);
    transform = MatrixMultiply(MatrixRotateY(props->angles.y), transform);
    transform = MatrixMultiply(MatrixRotateX(props->angles.x), transform);
    transform = MatrixMultiply(
        MatrixTranslate(0, 0, txt.state.size.z + 0.15f),
        transform);
    Vector3 finalPosition = {0, 0, 0};
    finalPosition         = Vector3Transform(finalPosition, transform);

    char text[1024];
    lr_read_string(props->buffer, text, lr_owner(props->owner));
    shoot(Text, txt,
          _({.font      = props->font,
             .spacing   = 0.6f,
             .tint      = BLACK,
             .content   = text,
             .parent    = (void *)props->buffer,
             .owner     = props->owner,
             .font_size = 24.,
             .angles    = props->angles,
             .pos       = finalPosition,
             .camera    = props->camera,
             .bg_color  = props->bg_color,
             .shader    = props->shader,
	     .on.hover = props->on.hover,
	     }));
}

DID_MOUNT_SKIP(Terminal);
DID_UPDATE_SKIP(Terminal);

