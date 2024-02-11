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
	if(props->on.before)  props->on.before(self);
}

DID_MOUNT(Window) {EndMode3D(); 
	if(props->on.after)  props->on.after(self);
	EndDrawing();
}
DID_UPDATE(Window)
{
	if(props->on.after)  props->on.after(self);
	EndDrawing();
}	

//DID_UNMOUNT CloseWindow()
