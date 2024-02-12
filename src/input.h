#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <hal.h>
#include <eer.h>

void *key_init(void *baudrate);
bool key_is_data_received();
bool key_is_transmit_ready();
void key_transmit(uint8_t data);
unsigned char key_receive();


void read_symbol(eer_t *uart_ptr);
void read_command(eer_t *uart);

extern eer_serial_handler_t eer_keyboard;
