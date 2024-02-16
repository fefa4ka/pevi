#include "input.h"
#include <raylib.h>

static int key_pressed = 0;

void *key_init(void *baudrate) { return 0; }

bool key_is_data_received()
{

    if (key_pressed) {
        return true;
    }


    if (IsKeyPressed(KEY_ENTER)) {
        key_pressed = '\r';
        return true;
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        key_pressed = '\b';
        return true;
    }
    key_pressed = GetCharPressed();
    if (key_pressed) {
        return true;
    }
    return false;
}

bool key_is_transmit_ready() { return true; }

void key_transmit(uint8_t data) {}

unsigned char key_receive()
{
    int key     = key_pressed;
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

