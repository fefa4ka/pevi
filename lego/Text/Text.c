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
                                 props->font_size, props->spacing, props->line_spacing);
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
	Font font =props->font;
    Symbol_new(dot);
    

        DrawCubeV((Vector3){state->pos.x + state->size.x / 2, state->pos.y - 0.15,
                                 state->pos.z + state->size.z / 2},
                       state->size, props->bg_color);
    eer_init(dot);
    for (int index = 0; index < TextLength(props->content); index++) {
	if (props->content[index] == '\n')
        {

		state->pos.z += dot.state.size.z;
	    state->pos.x = -1 * state->size.x / 2.0f + props->pos.x;
	} else if(props->content[index] == '\t') {
	    state->pos.x += props->spacing * 2;
	} else{
		react(Symbol, dot,
		      _({.font      = props->font,
			 .tint      = props->tint,
			 .content   = &props->content[index],
			 .font_size = props->font_size,
			 .pos       = state->pos,
			 .absolute_pos = props->absolute_pos,
			 .camera = props->camera,
			 .on = { .hover = props->on.hover},
			 .owner = props->owner,
			 .content_index = index}));


	state->pos.x += dot.state.size.x;
//if (font.glyphs[index].advanceX != 0)
//                state->pos.x  += (font.glyphs[index].advanceX + props->spacing)
//                             / (float)font.baseSize * dot.state.scale;
//            else
//                state->pos.x
//                    += (font.recs[index].width + font.glyphs[index].offsetX)
//                       / (float)font.baseSize * dot.state.scale;

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
                    += (font.recs[index].width + fontSpacing + font.glyphs[index].offsetX)
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


