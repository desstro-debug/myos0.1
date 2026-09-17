#ifndef KERNEL_C
#define KERNEL_C

#include "kernel.h"
#include "terminal.h"
#include "keyboard.h"
#include "shell.h"

void kernel_main(void) {
    terminal_init();
    terminal_clear();
    terminal_write_line("MyOS 0.1 - educational x86 kernel");
    terminal_write_line("Protected mode payload loaded at 0x00100000");
    terminal_write_line("Initializing keyboard driver...");
    keyboard_init();
    terminal_write_line("Keyboard ready.");
    terminal_write_line("Starting terminal shell...");
    shell_start();

    for (;;) {
        __asm__ volatile("hlt");
    }
}

#endif
