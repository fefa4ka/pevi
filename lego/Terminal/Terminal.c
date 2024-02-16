#include "Terminal.h"
#include <eers.h>
#include <Text.h>

///
/// \brief
///
WILL_MOUNT(Terminal) {
    state->fp = popen(props->command, "r");
    if (state->fp == NULL) {
        perror("popen");
	return;
    }

    state->fd = fileno(state->fp);
    state->buffer[0] = '\0';
}

///
/// \brief
///
SHOULD_UPDATE(Terminal) { 

        FD_ZERO(&state->fds);
        FD_SET(state->fd, &state->fds);
        FD_SET(STDIN_FILENO, &state->fds);

        if (select(state->fd + 1, &state->fds, NULL, NULL, NULL) == -1) {
		return false;
        }

	return true; 

}

///
/// \brief
///
WILL_UPDATE(Terminal) {
    char  buffer[BUFSIZ];
        if (FD_ISSET(state->fd, &state->fds)) {
            ssize_t bytes_read = read(state->fd, state->buffer, BUFSIZ);
            if (bytes_read <= 0) return;
	}

         //   for (ssize_t i = 0; i < bytes_read; i++) {
         //       lr_put(state.file_buffer, (char)buffer[i],
         //              lr_owner(state.cursor.plane));
         //   }
            
}

///
/// \brief
///
RELEASE(Terminal)
{
    Text_new(txt);

    eer_init(txt);
    react(Text, txt,
          _({
              .font      = GetFontDefault(),
              .spacing   = 0.6f,
              .tint      = BLACK,
              .content   = props->command,
              .owner     = 0,
              .font_size = 12.0f,
              .angles    = props->angles,
              .pos       = props->pos,
              .camera    = props->camera,
              .bg_color  = props->bg_color,
          }));
    if(state->buffer) {
    react(Text, txt,
          _({
              .font      = GetFontDefault(),
              .spacing   = 0.6f,
              .tint      = BLACK,
              .content   = state->buffer,
              .owner     = 0,
              .font_size = 12.0f,
              .angles    = props->angles,
              .pos       = props->pos,
              .camera    = props->camera,
              .bg_color  = props->bg_color,
          }));
    }
}

DID_MOUNT_SKIP(Terminal);
DID_UPDATE_SKIP(Terminal);

