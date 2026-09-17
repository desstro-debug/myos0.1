#ifndef KERNEL_C
#define KERNEL_C

#include "kernel.h"
#include "terminal.h"
#include "keyboard.h"
#include "shell.h"
#include "vfs.h"
#include "mount.h"
#include "driver_manager.h"
#include "serial_driver.h"
#include "boot_info.h"

void __attribute__((ms_abi)) kernel_main(const myos_boot_info *boot_info) {
    terminal_init();
    terminal_clear();
    terminal_write_line("MyOS 0.1 - educational x86_64 kernel");
    terminal_write_line("Long-mode payload base: 0x00100000");
    terminal_write_line("Initializing file system...");
    vfs_init();
    vfs_append_file("var/log/kernel.log", "kernel: filesystem initialized\n");
    vfs_append_file("var/cache/kernel.cache", "kernel-cache: initialized\n");
    mnt_init();
    vfs_append_file("var/log/kernel.log", "kernel: ram0 mounted at /mnt/ram\n");
    if (drivers_init() == 0) {
        vfs_append_file("var/log/kernel.log", "kernel: drivers initialized\n");
    } else {
        vfs_append_file("var/log/kernel.log", "kernel: driver initialization failed\n");
    }
    serial_driver_write(0, (const unsigned char *)"MyOS kernel: UEFI handoff complete\r\n", 38);
    if (boot_info != 0 && boot_info->magic == MYOS_BOOT_INFO_MAGIC) {
        serial_driver_write(0, (const unsigned char *)"BootInfo: valid\r\n", 18);
    }
    terminal_write_line("Initializing keyboard driver...");
    keyboard_init();
    vfs_append_file("var/log/kernel.log", "kernel: keyboard initialized\n");
    terminal_write_line("Keyboard ready.");
    terminal_write_line("Starting terminal shell...");
    shell_start();

    for (;;) {
        __asm__ volatile("hlt");
    }
}

#endif
