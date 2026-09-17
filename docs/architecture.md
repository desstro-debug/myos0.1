# MyOS architecture

This project keeps the bootloader and kernel as protected core components. They must remain stable and compatible with the BIOS and the boot contract.

The current migration target is x86_64 UEFI. The default build produces an ELF64 kernel and a PE32+ `BOOTX64.EFI` application. The old BIOS loader remains available only through the explicit `legacy` make target until the UEFI loader can load the kernel and pass a complete boot information structure.

The rest of the project is organized as modular building blocks:

- lib/: reusable helpers and string utilities
- drivers/: hardware drivers
- fs/: virtual file system abstraction
- proc/kernel/: protected kernel entry and linker contract
- proc/terminal/: VGA terminal subsystem
- proc/keyboard/: keyboard input subsystem
- proc/shell/: interactive shell subsystem
- var/log/: runtime logs with explicit file paths
- var/cache/: disposable runtime cache, including kernel cache
- docs/: architecture and design notes
- scripts/: build and tooling scripts

The philosophy is:

- bootloader and kernel are fixed baseline
- drivers can evolve independently
- filesystem can be replaced or extended without altering the boot contract
- runtime logs and cache use named VFS paths instead of implicit destinations
- userland and tools can be added later without touching the protected core
