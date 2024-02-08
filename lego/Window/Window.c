#include "Window.h"
#include <eers.h>
#include <raylib.h>


///
/// \brief Configure timer handler
///
WILL_MOUNT(Window)
{
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
}

///
/// \brief 
///
RELEASE(Window)
{
    BeginDrawing();
}

DID_MOUNT(Window) {
	EndDrawing();
}
DID_UPDATE(Window)
{
	EndDrawing();
}	

//DID_UNMOUNT CloseWindow()
