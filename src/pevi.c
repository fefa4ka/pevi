#include <Camera.h>
#include <Clock.h>
#include <Symbol.h>
#include <Text.h>
#include <Window.h>
#include <eer_app.h>
#include <lr.h>
#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>
#include <rlgl.h>
#include <string.h>

Clock(clk, &hw(timer), TIMESTAMP);
Window(win, _({"pevi", 800, 400}));
Camera_add(cam, _({(Vector3){10.0f, 0.0f, 1.0f}, (Vector3){0.0f, 0.0f, 0.0f},
                   (Vector3){0.0f, 1.0f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE,
                   CAMERA_THIRD_PERSON}));
Text_new(txt);
Symbol_new(dot);


#define BUFFER_SIZE 1000
struct lr_cell     cells[BUFFER_SIZE];
struct linked_ring lr;

char     text[BUFFER_SIZE] = {0}; //{'p', 'e', 'v', 'i', '\0'};
bool     edit_mode         = false;
Vector3  cursor_position[BUFFER_SIZE];
Vector3  cursor_cam_position[BUFFER_SIZE];
Vector3  cursor_cam_up[BUFFER_SIZE];
Matrix   cursor_matrix[BUFFER_SIZE];
size_t   cursor_index         = 0;
size_t   current_cursor_index = 0;
Vector3 *current_cursor;
Vector3  cursor_initial = (Vector3){0.0f, 0.0f, 0.0f};
Vector2  cursor_screen[BUFFER_SIZE];
float    cursor_angle[BUFFER_SIZE];

#define lr_last_cell(lr) ((lr)->cells + (lr)->size - 1)
// Function to calculate angles to rotate object towards camera
Vector2 CalculateBillboardAnglesYP(Vector3 objectPosition,
                                   Vector3 cameraPosition, Vector3 cameraUp)
{
    Vector3 direction
        = Vector3Normalize(Vector3Subtract(cameraPosition, objectPosition));

    // Calculate yaw (horizontal rotation)
    float yaw = atan2f(direction.x, direction.z);

    // Calculate pitch (vertical rotation)
    // Adjust the direction with respect to camera's up vector
    Vector3 right = Vector3CrossProduct(cameraUp, direction);
    direction
        = Vector3CrossProduct(direction, right); // Re-orthogonalize direction

    float pitch = asinf(direction.y);

    return (Vector2){yaw, pitch};
}


int main(void)
{
    lr_result_t result = lr_init(&lr, BUFFER_SIZE, cells);

    ignite(clk, win, cam);

    // Get char pressed (unicode character) on the queue
    int       key = GetCharPressed();
    lr_data_t data;

    // Check if more characters have been pressed on the same frame
    if (key == 'i') {
        edit_mode                         = true;
        cursor_position[cursor_index]     = cam.state.camera.target;
        cursor_cam_position[cursor_index] = cam.state.camera.position;
        cursor_cam_up[cursor_index]       = cam.state.camera.up;
        current_cursor                    = &cursor_position[cursor_index];
        current_cursor_index              = cursor_index;
        cursor_index += 1;
    }

    if (edit_mode) {
        while (key > 0) {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125)) {
                lr_put(&lr, (char)key, lr_owner(current_cursor_index));
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            lr_pop(&lr, &data, lr_owner(current_cursor_index));
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            edit_mode = false;
        }
    }

    ClearBackground(RAYWHITE);
    DrawGrid(100, 1.0f);
    DrawCube(cam.state.camera.target, 1.0f, 0.25f, 1.f, PURPLE);
    DrawCubeWires(cam.state.camera.target, 1.0f, 0.25f, 1.0f, DARKPURPLE);


    if (lr.owners != NULL) {
        struct lr_cell *owner_cell;

        for (owner_cell = lr_last_cell(&lr); owner_cell >= lr.owners;
             owner_cell--) {

            rlPushMatrix();

            lr_read_string(&lr, text, lr_owner(owner_cell->data));
            Vector3 text_poss = cursor_position[(size_t)owner_cell->data];
            Vector3 cam_pos   = cursor_cam_position[(size_t)owner_cell->data];
            Vector3 cam_up    = cursor_cam_up[(size_t)owner_cell->data];
            Vector3 text_pos  = {0};

            Vector2 angles
                = CalculateBillboardAnglesYP(text_poss, cam_pos, cam_up);

            rlTranslatef(text_poss.x, text_poss.y, text_poss.z);

            rlRotatef(RAD2DEG * angles.x, 0, 1,
                      0); // Rotate around Y-axis (yaw)
            rlRotatef(RAD2DEG * angles.y, 1, 0,
                      0); // Rotate around X-axis (pitch)


            Font fnt = GetFontDefault();

            for (int index = 0; index < TextLength(text); index++) {
                react(Symbol, dot,
                      _({.font      = GetFontDefault(),
                         .tint      = RED,
                         .content   = &text[index],
                         .font_size = 18.0f,
                         .pos       = text_pos}));

                text_pos.x += dot.state.size.x + 1.0f;
            }

            rlPopMatrix();
        }
    }


    halt(0);

    CloseWindow();
}

