# myos0.1

This project contains a minimal BIOS-compatible bootloader and a 32-bit kernel payload designed for the bootloader's Stage 2 load address at 0x00100000.

## Build

```bash
make
```

The build emits:

- build/kernel.elf
- build/kernel.bin

The raw binary is the payload expected by the loader.

## Kernel contract

- 32-bit protected mode
- flat memory model
- entry point: `kernel_main`
- loaded at physical address `0x00100000`
- no libc or CRT runtime

## Usage

Place the generated `build/kernel.bin` in the Stage 2 region reserved by the bootloader or adapt the loader to read it from disk before booting.