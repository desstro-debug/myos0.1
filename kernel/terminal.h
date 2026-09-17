#ifndef TERMINAL_H
#define TERMINAL_H

#include "kernel.h"

void terminal_init(void);
void terminal_clear(void);
void terminal_put_char(char c);
void terminal_write(const char *str);
void terminal_write_line(const char *str);
void terminal_put_hex32(u32 value);

#endif
