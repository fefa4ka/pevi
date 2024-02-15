#include "Cursor.h"
#include <eers.h>
#include <rlgl.h>

///
/// \brief
///
WILL_MOUNT(Cursor) {}

///
/// \brief
///
SHOULD_UPDATE(Cursor) { return next_props->is_visible; }

///
/// \brief
///
WILL_UPDATE(Cursor) {}

///
/// \brief
///
RELEASE(Cursor)
{

    rlPushMatrix();
    rlTranslatef(props->absolute_pos.x, props->absolute_pos.y,
                 props->absolute_pos.z);

    rlRotatef(RAD2DEG * props->angles.y, 0, 1,
              0); // Rotate around Y-axis (yaw)
    rlRotatef(RAD2DEG * props->angles.x, 1, 0,
              0); // Rotate around X-axis (pitch)
    //
    rlTranslatef(props->pos.x, props->pos.y, props->pos.z);

    if (props->angles.y || props->angles.x) {
        rlRotatef(90, 1, 0, 0);
    }
    DrawCubeV((Vector3){0}, props->size, PURPLE);
    DrawCubeWiresV((Vector3){0}, props->size, DARKPURPLE);

    rlPopMatrix();
}

DID_MOUNT_SKIP(Cursor);
DID_UPDATE_SKIP(Cursor);

