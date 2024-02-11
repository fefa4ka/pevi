#include "Serial.h"
#include <eers.h>

#define owner(channel) (lr_owner_t)(props->handler->channel)

WILL_MOUNT(Serial)
{
    props->handler->init(&props->baudrate);
    if (!props->delimiter) {
        props->delimiter = '\r';
    }
}

SHOULD_UPDATE(Serial)
{
    if (props->handler->is_data_received()) {
        state->mode = COMMUNICATION_MODE_RECEIVER;
        return true;
    }

    if (props->handler->is_transmit_ready()) {
        lr_data_t cell_data = 0;
        state->mode         = COMMUNICATION_MODE_TRANSMITTER;

        if (lr_get(props->buffer, &cell_data, owner(transmit)) == OK) {
            state->sending = (uint8_t)cell_data;

            return true;
        }
    }

    return false;
}

WILL_UPDATE_SKIP(Serial);

RELEASE(Serial)
{
    void (*callback)(eer_t *) = 0;

    if (state->mode == COMMUNICATION_MODE_TRANSMITTER) {
        props->handler->transmit(state->sending);

        callback = props->on.transmit;
    } else if (state->mode == COMMUNICATION_MODE_RECEIVER) {
        state->sending = props->handler->receive();
	lr_put(props->buffer, state->sending, owner(receive));

        callback = props->on.receive;
    }

    if (callback) {
        callback(self);
    }
}

DID_MOUNT_SKIP(Serial);

DID_UPDATE(Serial)
{
    void (*callback)(eer_t *) = 0;

    if (state->sending == props->delimiter) {
        if (state->mode == COMMUNICATION_MODE_RECEIVER)
            callback = props->on.receive_block;
        else
            callback = props->on.transmit_block;

        if (callback)
            callback(self);
    }

    state->sending = 0;
}
