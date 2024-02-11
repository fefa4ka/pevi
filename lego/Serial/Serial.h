#pragma once

#include <eer.h>
#include <lr.h>

#define Serial(instance, props) eer_withprops(Serial, instance, _(props))
#define Serial_write(instance, message)                                        \
    lr_put(eer_prop(Serial, instance, buffer), message,             \
             (lr_owner_t)(eer_prop(Serial, instance, handler)->transmit))
#define Serial_write_string(instance, message)                                 \
    lr_put_string(                                                           \
        eer_prop(Serial, instance, buffer), (unsigned char *)message,          \
        (lr_owner_t)(eer_prop(Serial, instance, handler)->transmit))
#define Serial_read(instance, lr_data)                                         \
    lr_get(eer_prop(Serial, instance, buffer), lr_data,                       \
            (lr_owner_t)(eer_prop(Serial, instance, handler)->receive))


typedef struct {
    eer_serial_handler_t *handler;
    /* TODO: uart_baudrate_t */
    uint32_t              baudrate;
    uint8_t               delimiter;
    struct linked_ring *  buffer;

    struct {
        void (*receive)(eer_t *instance);
        void (*receive_block)(eer_t *instance);
        void (*transmit)(eer_t *instance);
        void (*transmit_block)(eer_t *instance);
    } on;
} Serial_props_t;

typedef struct {
    enum eer_communication_mode mode;
    uint8_t                     sending; /* Current sending or receiving byte */
} Serial_state_t;

eer_header(Serial);
