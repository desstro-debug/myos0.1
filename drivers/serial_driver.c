#include "serial_driver.h"
#include "kernel.h"

static int serial_transmit_ready(void) {
    return (inb(SERIAL_COM1 + 5) & 0x20) != 0;
}

int serial_driver_init(void) {
    outb(SERIAL_COM1 + 1, 0x00);
    outb(SERIAL_COM1 + 3, 0x80);
    outb(SERIAL_COM1 + 0, 0x03);
    outb(SERIAL_COM1 + 1, 0x00);
    outb(SERIAL_COM1 + 3, 0x03);
    outb(SERIAL_COM1 + 2, 0xC7);
    outb(SERIAL_COM1 + 4, 0x0B);
    return 0;
}

int serial_driver_read(unsigned int offset, unsigned char *buffer, unsigned int size) {
    (void)offset;
    (void)buffer;
    (void)size;
    return 0;
}

int serial_driver_write(unsigned int offset, const unsigned char *buffer, unsigned int size) {
    (void)offset;
    if (buffer == 0) {
        return -1;
    }
    for (unsigned int i = 0; i < size; ++i) {
        unsigned int timeout = 1000000;
        while (!serial_transmit_ready() && timeout != 0) {
            timeout--;
        }
        if (timeout == 0) {
            return -1;
        }
        outb(SERIAL_COM1, buffer[i]);
    }
    return (int)size;
}
