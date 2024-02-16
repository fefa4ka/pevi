#include "Text.h"
#include "Symbol.h"
#include <eers.h>
#include <math.h>
#include <raymath.h>
#include <rlgl.h>

// Measure a text in 3D. For some reason `MeasureTextEx()` just doesn't seem to
// work so i had to use this instead.
static Vector3 MeasureText3D(Font font, const char *text, float fontSize,
                             float fontSpacing, float lineSpacing);

// Generates a nice color with a random hue
static Color GenerateRandomColor(float s, float v);

///
/// \brief
///
WILL_MOUNT(Text) {}

///
/// \brief
///
SHOULD_UPDATE(Text)
{
    Font *font   = &next_props->font;
    state->scale = next_props->font_size / (float)next_props->font.baseSize;
    state->size  = MeasureText3D(next_props->font, next_props->content, next_props->font_size,
                                 next_props->spacing, next_props->line_spacing);
    state->pos   = (Vector3){0};
    Vector2 world_pos = GetWorldToScreen(next_props->pos, *next_props->camera);

    Matrix transform;
    transform = MatrixTranslate(next_props->pos.x, next_props->pos.y, next_props->pos.z);
    transform = MatrixMultiply(MatrixRotateY(next_props->angles.y), transform);
    transform = MatrixMultiply(MatrixRotateX(next_props->angles.x), transform);
    transform = MatrixMultiply(
        MatrixTranslate(state->size.x, state->size.y, state->size.z),
        transform);
    Vector3 finalPosition = {0, 0, 0};
    finalPosition         = Vector3Transform(finalPosition, transform);

    Vector2 world_pos_right = GetWorldToScreen(finalPosition, *next_props->camera);

    return world_pos_right.x > 0 && world_pos_right.y > 0 && world_pos.x < GetScreenWidth()
           && world_pos.y < GetScreenHeight();
}

///
/// \brief
///
WILL_UPDATE(Text)
{

    //    DrawCubeWiresV((Vector3){ state->pos.x + state->size.x, state->pos.y,
    //    state->pos.z + state->size.z/2}, state->size, RED);
}

///
/// \brief
///
RELEASE(Text)
{

    Font font = props->font;
    Symbol_new(dot);

    if (!props->content)
        return;

    Ray          ray       = {0}; // Picking line ray
    RayCollision collision = {0}; // Ray collision hit info
    ray                    = GetMouseRay(GetMousePosition(), *props->camera);

    Mesh mesh = GenMeshCube(state->size.x, state->size.y, state->size.z);

    Matrix transform;
    transform = MatrixTranslate(props->pos.x, props->pos.y, props->pos.z);
    transform = MatrixMultiply(MatrixRotateY(props->angles.y), transform);
    transform = MatrixMultiply(MatrixRotateX(props->angles.x), transform);
    transform = MatrixMultiply(
        MatrixTranslate(state->size.x / 2, -0.15, state->size.z / 2),
        transform);
    collision = GetRayCollisionMesh(ray, mesh, transform);
    if (!collision.hit) {
        ray = GetMouseRay(
            (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2},
            *props->camera);
        collision = GetRayCollisionMesh(ray, mesh, transform);
    }

    rlPushMatrix();
    rlTranslatef(props->pos.x, props->pos.y, props->pos.z);

    rlRotatef(RAD2DEG * props->angles.y, 0, 1,
              0); // Rotate around Y-axis (yaw)
    rlRotatef(RAD2DEG * props->angles.x, 1, 0,
              0); // Rotate around X-axis (pitch)

    DrawCubeV((Vector3){state->size.x / 2, -0.15, state->size.z / 2},
              state->size, props->bg_color);

    if (collision.hit) {

        if (collision.hit && props->on.hover)
            props->on.hover(self);

        DrawCubeWiresV((Vector3){state->size.x / 2, -0.15, state->size.z / 2},
                       state->size, BLACK);
    }

    eer_init();
    for (int index = 0; index < TextLength(props->content); index++) {
        if (props->content[index] == '\n') {

            state->pos.z += dot.state.size.z;
            state->pos.x = 0;
        } else if (props->content[index] == '\t') {
            state->pos.x += props->spacing * 2;
        } else {
            shoot(Symbol, dot,
                  _({.font          = props->font,
                     .tint          = props->tint,
                     .parent        = props->parent,
                     .content       = &props->content[index],
                     .font_size     = props->font_size,
                     .pos           = state->pos,
                     .angles        = props->angles,
                     .absolute_pos  = props->pos,
                     .camera        = props->camera,
                     .on            = {.hover = props->on.hover},
                     .owner         = props->owner,
                     .is_hovered    = collision.hit,
                     .shader        = props->shader,
                     .content_index = index}));


            state->pos.x += dot.state.size.x
                            + props->spacing / font.baseSize * state->scale;
            // if (font.glyphs[index].advanceX != 0)
            //                 state->pos.x  += (font.glyphs[index].advanceX +
            //                 props->spacing)
            //                              / (float)font.baseSize *
            //                              dot.state.scale;
            //             else
            //                 state->pos.x
            //                     += (font.recs[index].width +
            //                     font.glyphs[index].offsetX)
            //                        / (float)font.baseSize * dot.state.scale;
        }
    }

    rlPopMatrix();
}

DID_MOUNT_SKIP(Text);
DID_UPDATE(Text){

};

// Measure a text in 3D. For some reason `MeasureTextEx()` just doesn't seem to
// work so i had to use this instead.
static Vector3 MeasureText3D(Font font, const char *text, float fontSize,
                             float fontSpacing, float lineSpacing)
{
    int len        = TextLength(text);
    int tempLen    = 0; // Used to count longer text line num chars
    int lenCounter = 0;

    float tempTextWidth = 0.0f; // Used to count longer text line width

    float scale      = fontSize / (float)font.baseSize;
    float textHeight = scale;
    float textWidth  = 0.0f;

    int letter = 0; // Current character
    int index  = 0; // Index position in sprite font

    for (int i = 0; i < len; i++) {
        lenCounter++;

        int next = 0;
        letter   = GetCodepoint(&text[i], &next);
        index    = GetGlyphIndex(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad byte is
        // found (and return 0x3f) but we need to draw all of the bad bytes
        // using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f)
            next = 1;
        i += next - 1;

        if (letter != '\n') {
            if (font.glyphs[index].advanceX != 0)
                textWidth += (font.glyphs[index].advanceX)
                             / (float)font.baseSize * scale;
            else
                textWidth
                    += (font.recs[index].width + font.glyphs[index].offsetX)
                       / (float)font.baseSize * scale;
        } else {
            if (tempTextWidth < textWidth)
                tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth  = 0.0f;
            textHeight += scale + lineSpacing / (float)font.baseSize * scale;
        }

        if (tempLen < lenCounter)
            tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth)
        tempTextWidth = textWidth;

    Vector3 vec = {0};
    vec.x       = tempTextWidth
            + (float)((tempLen - 1) * fontSpacing / (float)font.baseSize
                      * scale); // Adds chars spacing to measure
    vec.y = 0.25f;
    vec.z = textHeight;

    return vec;
}

