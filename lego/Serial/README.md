# Serial

`Serial` component provides an abstraction layer for serial communication. It allows for configuring the serial port (e.g. setting the baud rate, delimiter), as well as sending and receiving data via the serial port. It uses a [linked_ring](https://github.com/fefa4ka/linked_ring) buffer to handle data transmission and reception. The component also allows for setting callbacks for serial events such as data receive and transmit.

## Usage

```c
/* UART communication */
struct lr_cell     cells[BUFFER_SIZE] = {0};
struct linked_ring buffer             = {cells, BUFFER_SIZE};

Serial(uart, _({
    .handler  = &hw(uart),
    .baudrate = BAUDRATE,
    .buffer   = &buffer,
    .on       = {
        .receive_block = read_command, /* Read and execute command */
        .receive       = read_symbol   /* Echo input */
    }
}));
```

In this example, the `uart` component is declared and configured with a baud rate of `BAUDRATE` and a buffer of `buffer` which is created using a `linked_ring` structure. The `on` field in the properties is used to set callbacks for different serial events. In this case, `read_command` function will be called when a block of data is received and `read_symbol` function will be called when a single symbol is received. The `Serial_write` and `Serial_write_string` functions can be used to send data through the serial port. The `Serial_read` function can be used to read data from the serial port.
