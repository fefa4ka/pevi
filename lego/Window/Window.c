#include "Window.h"
#include <eers.h>
#include <raylib.h>


///
/// \brief Configure timer handler
///
WILL_MOUNT(Window)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(props->width, props->height, props->title);
}

///
/// \brief
///
SHOULD_UPDATE_SKIP(Window);

///
/// \brief
///
WILL_UPDATE(Window)
{
    if (props->on.before)
        props->on.before(self);
}

///
/// \brief
///
RELEASE(Window) { BeginDrawing(); }

DID_MOUNT(Window)
{
    if (props->on.after)
        props->on.after(self);
    EndDrawing();
}
DID_UPDATE(Window)
{

    if (props->on.after)
        props->on.after(self);
    EndDrawing();
}

// DID_UNMOUNT CloseWindow()
