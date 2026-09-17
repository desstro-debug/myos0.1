#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include "driver.h"

int keyboard_driver_init(void);
int keyboard_driver_read(unsigned int offset, unsigned char *buffer, unsigned int size);
int keyboard_driver_write(unsigned int offset, const unsigned char *buffer, unsigned int size);

#endif
