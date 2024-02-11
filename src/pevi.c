#include <Camera.h>
#include <Clock.h>
#include <Symbol.h>
#include <Text.h>
#include <Window.h>
#include <Serial.h>
#include <Menu.h>
#include <eer_app.h>
#include <lr.h>
#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>
#include <rlgl.h>
#include <string.h>


void on_shortcut(eer_t *menu);

void on_shortcut_not_found(eer_t *menu);
#define BUFFER_SIZE 1000
struct lr_cell     cells[BUFFER_SIZE];
struct linked_ring lr;

struct lr_cell     command_cells[BUFFER_SIZE] = {0};
struct linked_ring command_buffer             = {command_cells, BUFFER_SIZE};

#define COMMAND_BUFFER_SIZE 1024
char           command[COMMAND_BUFFER_SIZE];
char key_buffer[COMMAND_BUFFER_SIZE];
Clock(clk, &hw(timer), TIMESTAMP);
Camera_add(cam, _({(Vector3){10.0f, 0.0f, 1.0f}, (Vector3){0.0f, 0.0f, 0.0f},
                   (Vector3){0.0f, 1.0f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE,
                   CAMERA_FREE}));

void render_start(eer_t *win) {
//	BeginMode3D(cam.state.camera); // WHY THIS DON'T WORK?

}

bool is_fps_visible = true;
void render_end(eer_t *win) ;

Window(win, _({"pevi", 800, 400, &cam.state.camera,  
		   .on = { .before = render_start, .after= render_end}}));
Text_new(txt);
Symbol_new(dot);
void print_fps(void *arg) {
	is_fps_visible = !is_fps_visible;
}

bool command_mode = false;
void enable_edit_mode(void *arg);
void enable_command_mode(void *arg) {
command_mode = true;
}

Menu_command_t commands[] = {{"fps", print_fps},
                             {0}};

Menu_command_t shortcuts[] = {{":", enable_command_mode},
				{"i", enable_edit_mode},
                             {0}};

void on_command(eer_t *menu)
{
    *command = 0;
    command_mode = false;
}

void on_command_not_found(eer_t *menu)
{
    *command = 0;
    command_mode = false;
}

Menu(tty, _({.menu    = commands,
             .command = command,

             .on = {
                 .command   = on_command,
                 .not_found = on_command_not_found,
             }
	     }));



Menu(skm, _({.menu    = shortcuts,
             .command = key_buffer,

             .on = {
                 .command   = on_shortcut,
                 .not_found = on_shortcut_not_found,
             }
	     }));

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
Color cursor_color[BUFFER_SIZE];
float    cursor_angle[BUFFER_SIZE];

#define lr_last_cell(lr) ((lr)->cells + (lr)->size - 1)
// Function to calculate angles to rotate object towards camera
Vector2 CalculateBillboardAngles(Vector3 objectPosition, Vector3 cameraPosition,
                                 Vector3 cameraUp)
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

// Generates a nice color with a random hue
static Color GenerateRandomColor(float s, float v)
{
    const float Phi = 0.618033988749895f; // Golden ratio conjugate
    float       h   = (float)GetRandomValue(0, 360);
    h               = fmodf((h + h * Phi), 360.0f);
    return ColorFromHSV(h, s, v);
}

Vector3 CalculateCameraPositionFromBillboard(Vector3 playerPosition, Vector2 billboardAngles, Vector3 cameraUp, float distance) {
    // Calculate the direction vector from player to billboard
    Vector3 direction;
    direction.x = sinf(billboardAngles.x) * cosf(billboardAngles.y);
    direction.y = sinf(billboardAngles.y);
    direction.z = cosf(billboardAngles.x) * cosf(billboardAngles.y);

    // Calculate the right vector based on camera up vector
    Vector3 right = Vector3CrossProduct(cameraUp, direction);

    // Calculate the camera position
    Vector3 cameraPosition;
    cameraPosition.x = playerPosition.x + direction.x * distance;
    cameraPosition.y = playerPosition.y + direction.y * distance;
    cameraPosition.z = playerPosition.z + direction.z * distance;

    return cameraPosition;
}

bool current_symbol;
size_t current_symbol_owner;
size_t current_symbol_index;
void on_dot_hover(eer_t *symbol) {
	eer_self(Symbol, symbol);
	current_symbol = true;
	current_symbol_owner = (size_t)self->props.owner;
	current_symbol_index= (size_t)self->props.content_index;
}

void *key_init(void *baudrate)
{
	return 0;
}

int key_pressed = 0;
 bool key_is_data_received()
{

if(key_pressed) {
    return true;
}


if (IsKeyPressed(KEY_ENTER)) {
	key_pressed= '\r';
	return true;
}
key_pressed = GetCharPressed();
if(key_pressed) {
    return true;
}
return false;
}

bool key_is_transmit_ready()
{

    return true;
}

void key_transmit(uint8_t data)
{
}

unsigned char key_receive()
{
	int key = key_pressed;
	key_pressed = 0;
    return key;
}

eer_serial_handler_t eer_keyboard = {
    .init              = key_init,
    .is_data_received  = key_is_data_received,
    .is_transmit_ready = key_is_transmit_ready,
    .transmit          = key_transmit,
    .receive           = key_receive,
};

void read_symbol(eer_t *uart_ptr);
void read_command(eer_t *uart);
Serial(input,
       _({.handler  = &eer_keyboard,
          .buffer   = &command_buffer,
          .on       = {
                    .receive_block = read_command, /* Read and execute command */
                    .receive       = read_symbol   /* Echo input */
          }}));

void render_end(eer_t *win) {
	 EndMode3D();
	 if(is_fps_visible) {
         DrawFPS(10, 10);
	 }

	 if(command_mode) {
    DrawText(key_buffer,  10, GetScreenHeight() - 20, 10, GRAY);
	 }
}
void on_shortcut(eer_t *menu)
{
    lr_data_t data;
    while (Serial_read(&input, &data) == OK && data) {
    }
}

void on_shortcut_not_found(eer_t *menu)
{
    lr_data_t data;
    while (Serial_read(&input, &data) == OK && data) {
    }
}

/**
 * \brief    Echo each symbol from input to output
 */
void read_symbol(eer_t *uart_ptr)
{

    lr_read_string(&command_buffer, key_buffer, lr_owner(eer_prop(Serial, &input, handler)->receive));

}

/**
 * \brief    Read command from buffer
 */
void read_command(eer_t *uart)
{
    char     *command_symbol = command;
    lr_data_t data;
    while (Serial_read(&input, &data) == OK && data) {
        *command_symbol++ = (uint8_t)data;
    }
    *--command_symbol = 0;
}

/* UART communication */


void enable_edit_mode(void *args){
  edit_mode                         = true;
        cam.props.is_movable              = false;
	if(current_symbol) {
		current_cursor_index = current_symbol_owner;
   Vector3 text_pos = cursor_position[current_cursor_index];
            Vector3 cam_pos   = cursor_cam_position[current_cursor_index];
            Vector3 cam_up    = cursor_cam_up[current_cursor_index];

            Vector2 angles
                = CalculateBillboardAngles(text_pos, cam_pos, cam_up);

 		cam.state.camera.position = cam_pos;
 		cam.state.camera.up = cam_up;
 		cam.state.camera.target= text_pos;

	} else {
		cursor_position[cursor_index]     = cam.state.camera.target;
		cursor_cam_position[cursor_index] = cam.state.camera.position;
		cursor_cam_up[cursor_index]       = cam.state.camera.up;
		current_cursor                    = &cursor_position[cursor_index];
		current_cursor_index              = cursor_index;
float saturation = 0.1f; // High saturation for vivid colors
float value = 0.8f;      // High value for bright colors
Color randomColor = GenerateRandomColor(saturation, value);
cursor_color[cursor_index] = randomColor;
		cursor_index += 1;
	}

}
/* Application */
void edit_mode_process() {
	int key;
    int r= Serial_read(&input, &key);
if(r) { key = 0; }
   
        while (key > 0) {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125)) {
                lr_put(&lr, (char)key, lr_owner(current_cursor_index));
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
		int data;
            lr_pop(&lr, &data, lr_owner(current_cursor_index));
        }else if (IsKeyPressed(KEY_ESCAPE)) {
            edit_mode            = false;
            cam.props.is_movable = true;
current_symbol= 0;
        }else if (IsKeyPressed(KEY_ENTER))
        {
            // handle newline
            int len = TextLength(text);
            if (len < sizeof(text) - 1)
            {
                lr_put(&lr, (char)'\n', lr_owner(current_cursor_index));
            }
        } else if (IsKeyPressed(KEY_TAB))
        {
            // handle newline
            int len = TextLength(text);
            if (len < sizeof(text) - 1)
            {
                lr_put(&lr, (char)'\t', lr_owner(current_cursor_index));
            }
        }
}

void
shell() {
	ignite(input);
	if(command_mode) { 
		use(tty);
	} else if(edit_mode) {
		edit_mode_process();
	} else{
		use(skm);
	}
}


int main(void)
{
    lr_result_t result = lr_init(&lr, BUFFER_SIZE, cells);
    result = lr_init(&command_buffer, BUFFER_SIZE, command_cells);

//    Font fnt = LoadFontEx("FiraCode-Regular.ttf", 96, 0, 0);
    Font fnt = GetFontDefault();

    ignite(clk, win, cam);

    shell();

 
current_symbol = false;

    ClearBackground(RAYWHITE);
    //    DrawGrid(100, 1.0f);
    if (!edit_mode) {
    rlPushMatrix();
    	Vector3 pos = cam.state.camera.target;
            Vector2 angles
                = CalculateBillboardAngles(pos, cam.state.camera.position, cam.state.camera.up);

//            rlTranslatef(pos.x, pos.y, pos.z);
//
//            rlRotatef(RAD2DEG * angles.x, 0, 1,
//                      0); // Rotate around Y-axis (yaw)
//            rlRotatef(RAD2DEG * angles.y, 1, 0,
//                      0); // Rotate around X-axis (pitch)

        DrawCube(pos, 1.0f, 0.25f, 1.0f, PURPLE);
        DrawCubeWires(pos, 1.0f, 0.25f, 1.0f, DARKPURPLE);
            rlPopMatrix();
    }


    if (lr.owners != NULL) {
        struct lr_cell *owner_cell;

        for (owner_cell = lr_last_cell(&lr); owner_cell >= lr.owners;
             owner_cell--) {

            rlPushMatrix();

            lr_read_string(&lr, text, lr_owner(owner_cell->data));
            Vector3 text_poss = cursor_position[(size_t)owner_cell->data];
            Vector3 cam_pos   = cursor_cam_position[(size_t)owner_cell->data];
            Vector3 cam_up    = cursor_cam_up[(size_t)owner_cell->data];
            Color bg_color = cursor_color[(size_t)owner_cell->data];
            Vector3 text_pos  = {0};

            Vector2 angles
                = CalculateBillboardAngles(text_poss, cam_pos, cam_up);

            rlTranslatef(text_poss.x, text_poss.y, text_poss.z);

            rlRotatef(RAD2DEG * angles.x, 0, 1,
                      0); // Rotate around Y-axis (yaw)
            rlRotatef(RAD2DEG * angles.y, 1, 0,
                      0); // Rotate around X-axis (pitch)


            react(Text, txt,
                  _({.font      = GetFontDefault(),
			  .spacing = 0.6f,
                     .tint      = BLACK,
                     .content   = text,
		     .owner = owner_cell->data,
                     .font_size =12.0f,
                     .pos       = text_pos,
		     .absolute_pos = text_poss,
		     .camera = &cam.state.camera,
		     .bg_color= bg_color,
		     .on ={ .hover= on_dot_hover}}));

            if (edit_mode
                && (size_t)current_cursor_index == (size_t)owner_cell->data) {
		    txt.state.pos.z += 1.f;
		    Vector3 cursor_pos = txt.state.pos;
		    cursor_pos.x += 0.5f;
                DrawCube(cursor_pos, 0.8f, 0.25f, 0.25f, PURPLE);
                DrawCubeWires(cursor_pos, 0.8f, 0.25f, 0.25f, DARKPURPLE);
            }

            rlPopMatrix();
        }
    }


    halt(0);

    CloseWindow();
}

