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

char text[BUFFER_SIZE] = {0}; //{'p', 'e', 'v', 'i', '\0'};
bool edit_mode = false;
Vector3 cursor_position[BUFFER_SIZE];
Vector3 cursor_cam_position[BUFFER_SIZE];
Vector3 cursor_cam_up[BUFFER_SIZE];
Matrix cursor_matrix[BUFFER_SIZE];
size_t cursor_index = 0;
size_t current_cursor_index = 0;
Vector3 *current_cursor;
Vector3 cursor_initial = (Vector3){0.0f, 0.0f, 0.0f};
Vector2 cursor_screen[BUFFER_SIZE];
float cursor_angle[BUFFER_SIZE];

#define lr_last_cell(lr) ((lr)->cells + (lr)->size - 1)
// Function to calculate angles to rotate object towards camera
Vector2 CalculateBillboardAnglesYP(Vector3 objectPosition, Vector3 cameraPosition, Vector3 cameraUp) {
    Vector3 direction = Vector3Normalize(Vector3Subtract(cameraPosition, objectPosition));

    // Calculate yaw (horizontal rotation)
    float yaw = atan2f(direction.x, direction.z);

    // Calculate pitch (vertical rotation)
    // Adjust the direction with respect to camera's up vector
    Vector3 right = Vector3CrossProduct(cameraUp, direction);
    direction = Vector3CrossProduct(direction, right); // Re-orthogonalize direction

    float pitch = asinf(direction.y);

    return (Vector2){ yaw, pitch };
}

Vector2 CalculateBillboardAnglesYP_(Vector3 objectPosition, Vector3 cameraPosition) {
    Vector3 direction = Vector3Subtract(cameraPosition, objectPosition);

    // Calculate spherical coordinates
    float radius = Vector3Length(direction);
    float yaw = atan2f(direction.x, direction.z);
    float pitch = asinf(direction.y / radius);

    return (Vector2){ -yaw, -pitch };
}
Matrix CalculateBillboardMatrix_(Vector3 objectPosition, Vector3 cameraPosition) {
    // Calculate normalized direction from object to camera
    Vector3 direction = Vector3Normalize(Vector3Subtract(cameraPosition, objectPosition));
    
    // Calculate angle to rotate the object towards the camera
    float angle = atan2f(direction.x, direction.z);
    
    // Create rotation matrix
    Matrix rotationMatrix = MatrixRotateY(-angle);
    
    return rotationMatrix;
}
void CalculateBillboardAngles(Vector3 objectPosition, Vector3 cameraPosition, float *angleX, float *angleY, float *angleZ) {
    // Calculate normalized direction from object to camera
    Vector3 direction = Vector3Normalize(Vector3Subtract(cameraPosition, objectPosition));

    // Calculate angles to rotate the object towards the camera
    *angleX = atan2f(direction.y, sqrtf(direction.x * direction.x + direction.z * direction.z));
    *angleY = atan2f(direction.x, direction.z);
    *angleZ = asinf(direction.y);
}
// Function to calculate angle to rotate object towards camera
float CalculateBillboardAngle_(Vector3 objectPosition, Vector3 cameraPosition) {
    // Calculate normalized direction from object to camera
    Vector3 direction = Vector3Normalize(Vector3Subtract(cameraPosition, objectPosition));

    // Calculate angle to rotate the object towards the camera
    float angle = atan2f(direction.x, direction.z);

    return angle;
}

// Function to calculate rotation matrix to orient object towards camera
Matrix CalculateBillboardMatrix(Vector3 objectPosition, Vector3 cameraPosition) {
    // Calculate normalized direction from object to camera
    Vector3 direction = Vector3Normalize(Vector3Subtract(cameraPosition, objectPosition));

        // Flatten the direction (set Y to 0)

        // Calculate rotation matrix to make the object face the camera
 //       Vector3 direction = (Vector3Subtract(cameraPosition, objectPosition)
    // Calculate rotation matrix to orient object towards camera
    // We need to align the local forward direction of the object with the direction to the camera
    // The matrix representing this rotation can be calculated using QuaternionToMatrix
    Matrix rotationMatrix = QuaternionToMatrix(QuaternionFromVector3ToVector3((Vector3){ 0.0f, 0.0f, 1.0f }, direction));

    return rotationMatrix;
}

int main(void)
{
    lr_result_t result = lr_init(&lr, BUFFER_SIZE, cells);

    ignite(clk, win, cam);

    // Get char pressed (unicode character) on the queue
    int       key = GetCharPressed();
    lr_data_t data;

    // Check if more characters have been pressed on the same frame
    if(key == 'i') { 
	    edit_mode = true;

// Calculate direction from camera to object
        Vector3 direction = Vector3Subtract(cam.state.camera.position, cam.state.camera.target);

        // Flatten the direction (set Y to 0)
        direction.y = 0;

        // Calculate rotation matrix to make the object face the camera
        Matrix rotationMatrix = MatrixLookAt(Vector3Zero(), direction, cam.state.camera.up);
        rotationMatrix = MatrixInvert(rotationMatrix);
//            Matrix view = GetCameraMatrix(cam.state.camera);
//	Matrix matView = MatrixLookAt(cam.state.camera.position, cam.state.camera.target, cam.state.camera.up);
//
//
//// Calculate the transformation matrix for the text
//Matrix transform = MatrixLookAt(Vector3Zero(), Vector3Subtract(cam.state.camera.target, cam.state.camera.position), cam.state.camera.up);
//transform = MatrixInvert(transform); // Invert the matrix to face the camera
cursor_matrix[cursor_index] = rotationMatrix;


	    cursor_position[cursor_index] = cam.state.camera.target;
	    cursor_cam_position[cursor_index] = cam.state.camera.position;
	    cursor_cam_up[cursor_index] = cam.state.camera.up;
	    current_cursor = &cursor_position[cursor_index];
current_cursor_index  = cursor_index;
	    cursor_index += 1;
    }

    if(edit_mode) { 
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
    if(IsKeyPressed(KEY_ESCAPE)) {
	    edit_mode = false;
    }
    }

    ClearBackground(RAYWHITE);
    DrawGrid(100, 1.0f);
     DrawCube(cam.state.camera.target, 1.0f, 1.0f, 1.f, PURPLE);
    DrawCubeWires(cam.state.camera.target, 1.0f, 1.0f, 1.0f, DARKPURPLE);
//    // Matrix rotationMatrix = GetCameraViewMatrix(&cam.state.camera);
//
//    // rlMultMatrixf(MatrixToFloat(rotationMatrix));
//    rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
//    rlRotatef(90.0f, 0.0f, 0.0f, -1.0f);

        // Get camera matrix
//        Matrix view = GetCameraMatrix(cam.state.camera);
//
//        // Set object rotation to face camera
//        Matrix modelMatrix = MatrixIdentity();
//        modelMatrix = MatrixLookAt(cam.state.camera.target, cam.state.camera.position, (Vector3){ 0, 1, 0 });
//
//        // Combine camera matrix and object model matrix
//        Matrix modelView = MatrixMultiply(modelMatrix, view);
//
    //    for (int index = 0; index < 10; index++) {
    //        react(Text, txt,
    //              _({.content   = "/usr/bin",
    //                 .pos       = {0, 0, txt.state.size.z * index},
    //                 .font_size = 5.0f}));
    //    }
    float width = 0;
    //    for(int index = 0; index < TextLength(text); index++) {
    //
    lr_dump(&lr);

    if(lr.owners != NULL) {
    struct lr_cell *owner_cell;

    for(owner_cell = lr_last_cell(&lr); owner_cell >= lr.owners; owner_cell--) {

    rlPushMatrix();

    lr_read_string(&lr, text, lr_owner(owner_cell->data));
    Vector3 text_poss = cursor_position[(size_t)owner_cell->data];
    Vector3 cam_pos= cursor_cam_position[(size_t)owner_cell->data];
    Vector3 cam_up= cursor_cam_up[(size_t)owner_cell->data];
    Vector3 text_pos = {0};
    Matrix text_matrix = cursor_matrix[(size_t)owner_cell->data];
    printf("text: %s\n on (%f; %f; %f)", text, text_poss.x, text_poss.y, text_poss.z);
      //  float angle = CalculateBillboardAngle(text_poss, cam_pos);

        float angleY, angleZ, angleX;

        CalculateBillboardAngles(text_poss, cam_pos, &angleX, &angleY, &angleZ);
	
        Vector2 angles = CalculateBillboardAnglesYP(text_poss, cam_pos, cam_up);
        //Matrix rotationMatrix = CalculateBillboardMatrix(text_poss, cam_pos);

        rlTranslatef(text_poss.x, text_poss.y, text_poss.z);

       // rlMultMatrixf(MatrixToFloat(text_matrix));
        rlRotatef(RAD2DEG * angles.x, 0, 1, 0); // Rotate around Y-axis (yaw)
        rlRotatef(RAD2DEG * angles.y, 1, 0, 0); // Rotate around X-axis (pitch)
       //rlRotatef(RAD2DEG * angleY, 0, 1, 0); // Rotate around Y-axis
        //rlRotatef(RAD2DEG * angleX, 1, 0, 0); // Rotate around X-axis
        //rlRotatef(RAD2DEG * angleZ, 0, 0, 1); // Rotate around Z-axis
					     //
      //  rlMultMatrixf(MatrixToFloat(rotationMatrix)); // Apply rotation matrix

    Font fnt = GetFontDefault();

    // Extract float values from the view matrix
    float viewMatrixFloat[16];
    memcpy(viewMatrixFloat, &text_matrix, sizeof(text_matrix));

 //   rlMultMatrixf(viewMatrixFloat); // Pass the float array to rlMultMatrixf

//	rlMutestltMatrixf(MatrixToFloat(cursor_matrix[owner_cell->data]));
//

    
//rlMultMatrixf(MatrixToFloat(text_matrix)); // Convert Matrix to float array and apply

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

