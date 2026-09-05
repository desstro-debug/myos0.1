# MyOS 0.1 - Minimal Boot Assembly Entry Point
# 
# This is the low-level assembly code that serves as the entry point for the OS.
# It performs minimal initialization:
# - Disables interrupts
# - Calls the kernel main function
# - Halts the CPU in an infinite loop
#
# This is NOT a full bootloader yet. Proper bootloader support will be added later.

.section .text
.globl _start

_start:
    # Disable interrupts
    cli
    
    # Call kernel entry function
    call kernel_main
    
    # Halt the CPU in an infinite loop
    # If kernel_main returns, we halt here
    hlt
    jmp .  # Jump to self in case hlt doesn't halt
