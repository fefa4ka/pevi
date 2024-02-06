#include "Symbol.h"
#include <eers.h>
#include <math.h>
#include <rlgl.h>

static Vector3 symbol_measure(Font font, char *symbol, float fontSize, float fontSpacing, float lineSpacing);
///
/// \brief
///
WILL_MOUNT(Symbol) {}

///
/// \brief
///
SHOULD_UPDATE(Symbol)
{
    int codepointByteCount = 0;
    int codepoint          = GetCodepoint(props->content, &codepointByteCount);
    state->glyph           = GetGlyphIndex(props->font, codepoint);
    state->size = symbol_measure(props->font, props->content, props->font_size, 1.0f, 0.0f);
    return true;
}

///
/// \brief
///
WILL_UPDATE(Symbol)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned
    // points to '?'
    int   index = state->glyph;
    float scale = props->font_size / (float)props->font.baseSize;
    Font *font  = &props->font;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    state->pos.y = props->pos.y;
    state->pos.x = props->pos.x + (float)(font->glyphs[index].offsetX - font->glyphPadding)
                   / (float)font->baseSize * scale;
    state->pos.z = props->pos.z + (float)(font->glyphs[index].offsetY - font->glyphPadding)
                    / (float)font->baseSize * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for
    // outline/glow shader effects
    Rectangle srcRec = {font->recs[index].x - (float)font->glyphPadding,
                        font->recs[index].y - (float)font->glyphPadding,
                        font->recs[index].width + 2.0f * font->glyphPadding,
                        font->recs[index].height + 2.0f * font->glyphPadding};

    float width = (float)(font->recs[index].width + 2.0f * font->glyphPadding)
                  / (float)font->baseSize * scale;
    float height = (float)(font->recs[index].height + 2.0f * font->glyphPadding)
                   / (float)font->baseSize * scale;

    if (font->texture.id > 0) {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture
        // (0.0f -> 1.0f)
        const float tx = srcRec.x / font->texture.width;
        const float ty = srcRec.y / font->texture.height;
        const float tw = (srcRec.x + srcRec.width) / font->texture.width;
        const float th = (srcRec.y + srcRec.height) / font->texture.height;

        //		if (SHOW_LETTER_BOUNDRY) DrawCubeWiresV((Vector3){
        // position.x + width/2, position.y, position.z + height/2}, (Vector3){
        // width, LETTER_BOUNDRY_SIZE, height }, LETTER_BOUNDRY_COLOR);

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
        rlPopMatrix();

        rlSetTexture(0);
    }
}

///
/// \brief
///
RELEASE(Symbol) {}

DID_MOUNT_SKIP(Symbol);
DID_UPDATE_SKIP(Symbol);


// Measure a text in 3D. For some reason `MeasureTextEx()` just doesn't seem to work so i had to use this instead.
static Vector3 symbol_measure(Font font, char *symbol, float fontSize, float fontSpacing, float lineSpacing)
{
	float tempTextWidth = 0.0f;     // Used to count longer text line width

	float scale = fontSize/(float)font.baseSize;
	float textHeight = 1;//scale;
	float textWidth = 0.0f;

	int letter = 0;                 // Current character
	int index = 0;                  // Index position in sprite font



    int codepointByteCount = 0;
		letter = GetCodepoint(symbol, &codepointByteCount);
		index = GetGlyphIndex(font, letter);


			if (font.glyphs[index].advanceX != 0) textWidth += (font.glyphs[index].advanceX+fontSpacing)/(float)font.baseSize*scale;
			else textWidth += (font.recs[index].width + font.glyphs[index].offsetX)/(float)font.baseSize*scale;

		Vector3 vec = { 0 };
	vec.x = textWidth; // Adds chars spacing to measure
	vec.y = 0.25f;
	vec.z = textHeight;

	return vec;
}
