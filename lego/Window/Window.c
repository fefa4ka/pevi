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

    // Loading file to memory
    int            fileSize = 0;
    unsigned char *fileData = LoadFileData("font.ttf", &fileSize);

    // SDF font generation from TTF font
    Font fontSDF       = {0};
    fontSDF.baseSize   = 32;
    fontSDF.glyphCount = 95;
    // Parameters > font size: 16, no glyphs array provided (0), glyphs count: 0
    // (defaults to 95)
    fontSDF.glyphs
        = LoadFontData(fileData, fileSize, fontSDF.baseSize, 0, 0, FONT_SDF);
    // Parameters > glyphs count: 95, font size: 16, glyphs padding in image: 0
    // px, pack method: 1 (Skyline algorythm)
    Image atlas     = GenImageFontAtlas(fontSDF.glyphs, &fontSDF.recs, 95,
                                        fontSDF.baseSize, 0, 1);
    fontSDF.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);

    UnloadFileData(fileData); // Free memory from loaded file

    // Load SDF required shader (we use default vertex shader)
    Shader shader = LoadShader(0, TextFormat("sdf.fs", 330));
    SetTextureFilter(fontSDF.texture, TEXTURE_FILTER_BILINEAR); // Required for
    state->font   = fontSDF;
    state->shader = shader;
//    state->font = GetFontDefault();
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
