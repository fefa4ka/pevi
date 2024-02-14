#include "Cursor.h"
#include <eers.h>


///
/// \brief
///
WILL_MOUNT(Cursor) {}

///
/// \brief
///
SHOULD_UPDATE(Cursor)
{
    return next_props->is_visible;
}

///
/// \brief
///
WILL_UPDATE(Cursor)
{

}

///
/// \brief
///
RELEASE(Cursor)
{
        DrawCubeV(props->pos, props->size, PURPLE);
        DrawCubeWiresV(props->pos, props->size, DARKPURPLE);
}

DID_MOUNT_SKIP(Cursor);
DID_UPDATE_SKIP(Cursor);


