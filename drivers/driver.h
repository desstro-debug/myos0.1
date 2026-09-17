#ifndef DRIVER_H
#define DRIVER_H

#include "kernel.h"

typedef struct {
    const char *name;
    int (*init)(void);
    int (*read)(unsigned int offset, unsigned char *buffer, unsigned int size);
    int (*write)(unsigned int offset, const unsigned char *buffer, unsigned int size);
} driver_t;

#endif
