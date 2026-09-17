#include "driver_manager.h"
#include "driver.h"
#include "keyboard_driver.h"
#include "serial_driver.h"

#define DRIVER_MAX 8

static driver_t *g_drivers[DRIVER_MAX];
static unsigned int g_driver_count;

static driver_t g_keyboard = {
    "ps2-keyboard",
    keyboard_driver_init,
    keyboard_driver_read,
    keyboard_driver_write
};

static driver_t g_serial = {
    "serial-com1",
    serial_driver_init,
    serial_driver_read,
    serial_driver_write
};

static int register_driver(driver_t *driver) {
    if (g_driver_count >= DRIVER_MAX || driver == 0) {
        return -1;
    }
    if (driver->init() != 0) {
        return -1;
    }
    g_drivers[g_driver_count++] = driver;
    return 0;
}

int drivers_init(void) {
    g_driver_count = 0;
    if (register_driver(&g_keyboard) != 0) {
        return -1;
    }
    if (register_driver(&g_serial) != 0) {
        return -1;
    }
    return 0;
}

unsigned int drivers_count(void) {
    return g_driver_count;
}
