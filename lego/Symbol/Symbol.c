#include "Symbol.h"
#include <eers.h>
#include <math.h>
#include <raymath.h>
#include <rlgl.h>

static Vector3 symbol_measure(Font font, char *symbol, float fontSize);

///
/// \brief
///
WILL_MOUNT(Symbol) {}
///
/// \brief
///
SHOULD_UPDATE(Symbol)
{
    Font *font   = &next_props->font;
    state->scale = next_props->font_size / (float)next_props->font.baseSize;

    if (!next_props->content)
        return false;

    //    if(props->content != next_props->content || !state->glyph) {
    int codepointByteCount = 0;
    int codepoint = GetCodepoint(next_props->content, &codepointByteCount);
    state->glyph  = GetGlyphIndex(next_props->font, codepoint);
    state->size   = symbol_measure(next_props->font, next_props->content,
                                   next_props->font_size);

    if (state->glyph)
        return true;

    return false;
}

///
/// \brief
///
WILL_UPDATE(Symbol)
{
    Font *font = &next_props->font;

    //	}

    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned
    // points to '?'

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    state->pos.y = next_props->pos.y;
    state->pos.x
        = next_props->pos.x
          + (float)(font->glyphs[state->glyph].offsetX - font->glyphPadding)
                / (float)font->baseSize * state->scale;
    state->pos.z
        = next_props->pos.z
          + (float)(font->glyphs[state->glyph].offsetY - font->glyphPadding)
                / (float)font->baseSize * state->scale;
}
///
/// \brief
///
RELEASE(Symbol)
{
    Font        *font  = &props->font;
    int          index = state->glyph;
    static float a;
    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for
    // outline/glow shader effects
    if (!state->glyph) {
        return;
    }
    Rectangle srcRec = {font->recs[index].x - (float)font->glyphPadding,
                        font->recs[index].y - (float)font->glyphPadding,
                        font->recs[index].width + 2.0f * font->glyphPadding,
                        font->recs[index].height + 2.0f * font->glyphPadding};

    float width = (float)(font->recs[index].width + 2.0f * font->glyphPadding)
                  / (float)font->baseSize * state->scale;
    float height = (float)(font->recs[index].height + 2.0f * font->glyphPadding)
                   / (float)font->baseSize * state->scale;

    if (font->texture.id > 0) {

        Ray          ray       = {0}; // Picking line ray
        RayCollision collision = {0}; // Ray collision hit info
                                      //
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture
        // (0.0f -> 1.0f)
        const float tx = srcRec.x / font->texture.width;
        const float ty = srcRec.y / font->texture.height;
        const float tw = (srcRec.x + srcRec.width) / font->texture.width;
        const float th = (srcRec.y + srcRec.height) / font->texture.height;

        rlCheckRenderBatchLimit(4 + 4 * props->backface);
        rlSetTexture(font->texture.id);

        rlPushMatrix();
        rlTranslatef(state->pos.x, state->pos.y, state->pos.z);

        rlBegin(RL_QUADS);
        rlColor4ub(props->tint.r, props->tint.g, props->tint.b, props->tint.a);

        // Front Face
        rlNormal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up
        rlTexCoord2f(tx, ty);
        rlVertex3f(x, y, z); // Top Left Of The Texture and Quad
        rlTexCoord2f(tx, th);
        rlVertex3f(x, y, z + height); // Bottom Left Of The Texture and Quad
        rlTexCoord2f(tw, th);
        rlVertex3f(x + width, y,
                   z + height); // Bottom Right Of The Texture and Quad
        rlTexCoord2f(tw, ty);
        rlVertex3f(x + width, y, z); // Top Right Of The Texture and Quad

        if (props->backface) {
            // Back Face
            rlNormal3f(0.0f, -1.0f, 0.0f); // Normal Pointing Down
            rlTexCoord2f(tx, ty);
            rlVertex3f(x, y, z); // Top Right Of The Texture and Quad
            rlTexCoord2f(tw, ty);
            rlVertex3f(x + width, y, z); // Top Left Of The Texture and Quad
            rlTexCoord2f(tw, th);
            rlVertex3f(x + width, y,
                       z + height); // Bottom Left Of The Texture and Quad
            rlTexCoord2f(tx, th);
            rlVertex3f(x, y,
                       z + height); // Bottom Right Of The Texture and Quad
        }
        rlEnd();


        if (props->is_hovered) {
            Camera cam = *props->camera;
            ray        = GetMouseRay(GetMousePosition(), cam);

            Mesh mesh
                = GenMeshCube(state->size.x, state->size.y, state->size.z);

            Matrix transform;
            transform
                = MatrixTranslate(props->absolute_pos.x, props->absolute_pos.y,
                                  props->absolute_pos.z);
            transform
                = MatrixMultiply(MatrixRotateY(props->angles.y), transform);
            transform
                = MatrixMultiply(MatrixRotateX(props->angles.x), transform);
            transform = MatrixMultiply(
                MatrixTranslate(state->pos.x + state->size.x / 2,
                                state->pos.y + state->size.y / 2,
                                state->pos.z + state->size.z / 2),
                transform);

            collision = GetRayCollisionMesh(ray, mesh, transform);
            if (!collision.hit) {
                ray = GetMouseRay(
                    (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2},
                    cam);
                collision = GetRayCollisionMesh(ray, mesh, transform);
            }

            if (collision.hit && props->on.hover)
                props->on.hover(self);

            if (collision.hit) {
                Vector3 cursor_size = state->size;
                cursor_size.y       = 0.15;

                DrawCubeWiresV((Vector3){state->size.x / 2, 0,

                                         state->size.z / 2},
                               cursor_size, RED);
            }
        }
        rlPopMatrix();

        //        DrawRotatedBoundingBox(originalBox, GRAY);

        rlSetTexture(0);
    }
}

DID_MOUNT_SKIP(Symbol);
DID_UPDATE_SKIP(Symbol);


// Measure a text in 3D. For some reason `MeasureTextEx()` just doesn't seem to
// work so i had to use this instead.
static Vector3 symbol_measure(Font font, char *symbol, float fontSize)
{
    float tempTextWidth = 0.0f; // Used to count longer text line width

    float scale      = fontSize / (float)font.baseSize;
    float textHeight = scale;
    float textWidth  = 0.0f;

    int letter = 0; // Current character
    int index  = 0; // Index position in sprite font


    int codepointByteCount = 0;
    letter                 = GetCodepoint(symbol, &codepointByteCount);
    index                  = GetGlyphIndex(font, letter);


    if (font.glyphs[index].advanceX != 0)
        textWidth
            += (font.glyphs[index].advanceX) / (float)font.baseSize * scale;
    else
        textWidth += (font.recs[index].width + font.glyphs[index].offsetX)
                     / (float)font.baseSize * scale;

    Vector3 vec = {0};
    vec.x       = textWidth; // Adds chars spacing to measure
    vec.y       = 0.25f;
    vec.z       = textHeight;

    return vec;
}
