#ifndef KERNEL_C
#define KERNEL_C

#include "kernel.h"
#include "terminal.h"
#include "keyboard.h"
#include "shell.h"

/**
 * kernel_main - Main entry point for the kernel
 * 
 * Called from the assembly boot code after minimal setup.
 * Initializes all kernel subsystems and starts the shell.
 * 
 * This function should never return. If it does, the assembly
 * boot code will halt the CPU.
 */
void kernel_main(void) {
    /* Initialize the VGA text-mode terminal */
    terminal_init();
    
    /* Clear the screen and display startup message */
    terminal_clear();
    terminal_print("MyOS 0.1 - Educational x86 Operating System\n");
    terminal_print("Boot sequence initiated...\n\n");
    
    /* Initialize keyboard subsystem (currently a placeholder) */
    keyboard_init();
    terminal_print("Keyboard subsystem initialized.\n");
    
    /* Initialize and start the shell */
    terminal_print("\nStarting shell...\n");
    shell_start();
    
    /* If shell_start returns, we're done. Halt indefinitely. */
    /* (The assembly boot code will halt the CPU) */
}

#endif
