#include "terminal.h"

#define VGA_WIDTH 80U
#define VGA_HEIGHT 25U
#define VGA_MEMORY 0x000B8000U
#define TAB_WIDTH 4U

static volatile u16 *const vga = (volatile u16 *)VGA_MEMORY;
static u32 terminal_row = 0;
static u32 terminal_col = 0;

static void terminal_scroll(void) {
    for (u32 y = 1; y < VGA_HEIGHT; ++y) {
        for (u32 x = 0; x < VGA_WIDTH; ++x) {
            vga[(y - 1) * VGA_WIDTH + x] = vga[y * VGA_WIDTH + x];
        }
    }

    for (u32 x = 0; x < VGA_WIDTH; ++x) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (u16)(' ' | (0x0F << 8));
    }

    terminal_row = VGA_HEIGHT - 1;
    terminal_col = 0;
}

void terminal_init(void) {
    terminal_clear();
    terminal_row = 0;
    terminal_col = 0;
}

void terminal_clear(void) {
    for (u32 i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga[i] = (u16)(' ' | (0x0F << 8));
    }
    terminal_row = 0;
    terminal_col = 0;
}

void terminal_put_char(char c) {
    if (c == '\n') {
        terminal_col = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }

    if (c == '\r') {
        terminal_col = 0;
        return;
    }

    if (c == '\t') {
        for (u32 i = 0; i < TAB_WIDTH; ++i) {
            terminal_put_char(' ');
        }
        return;
    }

    if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
            vga[terminal_row * VGA_WIDTH + terminal_col] = (u16)(' ' | (0x0F << 8));
        }
        return;
    }

    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }

    vga[terminal_row * VGA_WIDTH + terminal_col] = (u16)(c | (0x0F << 8));
    terminal_col++;

    if (terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

void terminal_write(const char *str) {
    while (*str != '\0') {
        terminal_put_char(*str++);
    }
}

void terminal_write_line(const char *str) {
    terminal_write(str);
    terminal_write("\n");
}

void terminal_put_hex32(u32 value) {
    static const char digits[] = "0123456789ABCDEF";
    char buffer[9];
    for (int i = 7; i >= 0; --i) {
        buffer[i] = digits[value & 0xF];
        value >>= 4;
    }
    buffer[8] = '\0';
    terminal_write("0x");
    terminal_write(buffer);
}
