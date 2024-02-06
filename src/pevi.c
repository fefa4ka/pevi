#include <Camera.h>
#include <Clock.h>
#include <Text.h>
#include <Symbol.h>
#include <Window.h>
#include <eer_app.h>
#include <math.h> // Required for: sinf()
#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>
#include <rlgl.h>

// To make it work with the older RLGL module just comment the line below
#define RAYLIB_NEW_RLGL

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
#define LETTER_BOUNDRY_SIZE  0.25f
#define TEXT_MAX_LAYERS      32
#define LETTER_BOUNDRY_COLOR VIOLET
bool SHOW_LETTER_BOUNDRY = false;
bool SHOW_TEXT_BOUNDRY   = false;

Clock(clk, &hw(timer), TIMESTAMP);
Window(win, _({"pevi", 800, 400}));
Camera_add(cam, _({(Vector3){10.0f, 10.0f, 10.0f}, (Vector3){0.0f, 0.0f, 0.0f},
                   (Vector3){0.0f, 1.0f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE,
                   CAMERA_THIRD_PERSON}));
Text_new(txt);
Symbol_new(dot);

char *text = "pevi";

int main(void)
{
    ignite(clk, win, cam);


    ClearBackground(RAYWHITE);
    DrawGrid(10, 2.0f);

    // Matrix rotationMatrix = GetCameraViewMatrix(&cam.state.camera);
    rlPushMatrix();

    // rlMultMatrixf(MatrixToFloat(rotationMatrix));
    rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    rlRotatef(90.0f, 0.0f, 0.0f, -1.0f);

    for (int index = 0; index < 10; index++) {
        react(Text, txt,
              _({.content   = "/usr/bin",
                 .pos       = {0, 0, txt.state.size.z * index},
                 .font_size = 5.0f}));
    }
int width=0;
//    for(int index = 0; index < TextLength(text); index++) {
//	    printf("[%c] = size x=%f,y=%f,z=%f\n",text[index],dot.state.size.x, dot.state.size.y, dot.state.size.z);
//
//    for (int index = 0; index < TextLength(text); index++) {
	    react(Symbol, dot, _({ .font = GetFontDefault(),
				    .tint = RED,
				    .content = &text[0],
				    .font_size = 18.0f,
				    .pos = {0,0, 0}}));
	    
  react(Symbol, dot, _({ .font = GetFontDefault(),
				    .tint = RED,
				    .content = &text[1],
				    .font_size = 18.0f,
				    .pos = {0,0, dot.state.size.z * 1.5}}));
	  //  width+= dot.state.size.x + 1.0f;
//    }
    


    rlPopMatrix();

    halt(0);

    CloseWindow();
}

