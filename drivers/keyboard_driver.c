#include "keyboard_driver.h"
#include "kernel.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

int keyboard_driver_init(void) {
    while ((inb(KEYBOARD_STATUS_PORT) & 0x01) != 0) {
        (void)inb(KEYBOARD_DATA_PORT);
    }
    return 0;
}

int keyboard_driver_read(unsigned int offset, unsigned char *buffer, unsigned int size) {
    (void)offset;
    if (buffer == 0 || size == 0 || (inb(KEYBOARD_STATUS_PORT) & 0x01) == 0) {
        return 0;
    }
    buffer[0] = inb(KEYBOARD_DATA_PORT);
    return 1;
}

int keyboard_driver_write(unsigned int offset, const unsigned char *buffer, unsigned int size) {
    (void)offset;
    (void)buffer;
    (void)size;
    return -1;
}
