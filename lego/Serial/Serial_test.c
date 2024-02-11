#include <Serial.h>
#include <eer_app.h>
#include <unit.h>
#include <string.h>


#define BUFFER_SIZE 32
const char *from_bus_message = "buffer\r";
const char *to_bus_message   = "bus\r";
char        transmitted_message[BUFFER_SIZE];
char        received_message[BUFFER_SIZE];
uint8_t     sending_index = 0;
uint8_t     receive_index = 0;

void uart_init(void *baudrate) { eer_profiler_count(uart_init); }

bool uart_is_data_received()
{
    eer_profiler_count(uart_is_data_received);

    return from_bus_message[sending_index] != 0;
}

bool uart_is_transmit_ready()
{
    eer_profiler_count(uart_is_transmit_ready);

    return true;
}

void uart_transmit(uint8_t data)
{
    eer_profiler_count(uart_transmit);

    transmitted_message[receive_index++] = data;
}

uint8_t uart_receive() { return from_bus_message[sending_index++]; }

struct eer_serial_handler uart_periphery = {
    .init              = uart_init,
    .is_data_received  = uart_is_data_received,
    .is_transmit_ready = uart_is_transmit_ready,
    .transmit          = uart_transmit,
    .receive           = uart_receive,
};

struct lr_cell     cells[BUFFER_SIZE] = {0};
struct linked_ring buffer             = {cells, BUFFER_SIZE};

uint8_t transmitted_count = 0;
uint8_t is_transmitted = false;
void    symbol_transmit(eer_t *serial) { transmitted_count++; }
void    message_transmit(eer_t *serial) { is_transmitted = true; }

uint8_t received_count = 0;
void    symbol_receive(eer_t *serial) { received_count++; }
void    message_receive(eer_t *serial)
{
    Serial_t *uart = (Serial_t *)serial;

    char *    message_ptr = received_message;
    lr_data_t cell_data   = 0;
    while (Serial_read(uart, &cell_data) == OK) {
        *message_ptr++ = (uint8_t)cell_data;
    }
    *message_ptr = 0;

}


Serial(uart, _({
                 .handler  = &uart_periphery,
                 .buffer   = &buffer,
                 .baudrate = 9600,

                 .on = {
                     .transmit       = symbol_transmit,
                     .transmit_block = message_transmit,
                     .receive        = symbol_receive,
                     .receive_block  = message_receive,
                 },
             }));

test(basic_handler)
{
    Serial_write_string(&uart, to_bus_message);

    ignite(uart);

    terminate;
}

result_t basic_handler()
{
    sleep(1);
    test_assert(is_transmitted, "Transmittion should be complete");
    test_assert(transmitted_count == strlen(to_bus_message), "Should be transmitted %ld bytes, but %d", strlen(to_bus_message), transmitted_count);
    test_assert(received_count == strlen(from_bus_message), "Should be received %ld bytes, but %d", strlen(from_bus_message), received_count);
    test_assert(strcmp(received_message, from_bus_message) == 0,
                "Message from bus should be \"buffer\", but \"%s\"", received_message);
    test_assert(strcmp(transmitted_message, to_bus_message) == 0,
                "Message from bus should be \"buffer\", but \"%s\"", transmitted_message);
    return OK;
}
