#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include "driver.h"

#define SERIAL_COM1 0x3F8

int serial_driver_init(void);
int serial_driver_read(unsigned int offset, unsigned char *buffer, unsigned int size);
int serial_driver_write(unsigned int offset, const unsigned char *buffer, unsigned int size);

#endif
