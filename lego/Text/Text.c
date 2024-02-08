#include "Text.h"
#include "Symbol.h"
#include <eers.h>
#include <math.h>
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

    state->size  = MeasureText3D(GetFontDefault(), props->content,
                                 props->font_size, 1.0f, 0.0f);
    state->pos.x = -1 * state->size.x / 2.0f + props->pos.x;
    state->pos.y = props->pos.y;
    state->pos.z = props->pos.z;
    return true;
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
    Symbol_new(dot);

    eer_init(dot);
    for (int index = 0; index < TextLength(props->content); index++) {
	if (props->content[index] != '\n')
        {
        react(Symbol, dot,
              _({.font      = props->font,
                 .tint      = props->tint,
                 .content   = &props->content[index],
                 .font_size = props->font_size,
                 .pos       = state->pos}));


        state->pos.x += dot.state.size.x + props->spacing;
        } else {
		state->pos.z += dot.state.size.z;
    state->pos.x = -1 * state->size.x / 2.0f + props->pos.x;

	}
    }
}

DID_MOUNT_SKIP(Text);
DID_UPDATE_SKIP(Text);

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
                textWidth += (font.glyphs[index].advanceX + fontSpacing)
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

// Generates a nice color with a random hue
static Color GenerateRandomColor(float s, float v)
{
    const float Phi = 0.618033988749895f; // Golden ratio conjugate
    float       h   = (float)GetRandomValue(0, 360);
    h               = fmodf((h + h * Phi), 360.0f);
    return ColorFromHSV(h, s, v);
}
