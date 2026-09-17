# MyOS 0.1

MyOS is a freestanding x86_64 operating-system project with a native UEFI boot path. The system is intentionally modular: the loader establishes a strict handoff contract, while the kernel, drivers, VFS, and runtime services evolve behind it.

## Current boot flow

```text
UEFI firmware
		-> BOOTX64.EFI
		-> validate kernel.elf SHA-256
		-> validate and load ELF64 PT_LOAD segments
		-> discover GOP framebuffer
		-> collect the UEFI memory map
		-> ExitBootServices
		-> kernel_main(BootInfo *)
```

The loader is tested with OVMF/QEMU. It rejects modified kernel payloads before execution and passes framebuffer, memory-map, loader, and kernel metadata through [proc/kernel/boot_info.h](proc/kernel/boot_info.h).

## Build

Requirements: GCC, Clang, GNU ld, NASM-compatible x86_64 tools, and QEMU/OVMF for runtime testing.

```bash
make clean
make
make efi-tree
```

Artifacts:

- `build/BOOTX64.EFI` - unsigned UEFI application
- `build/kernel.elf` - ELF64 kernel image
- `build/kernel.bin` - raw diagnostic payload
- `build/esp/` - test EFI system-partition tree

Run the EFI tree with OVMF:

```bash
qemu-system-x86_64 \
		-bios /usr/share/qemu/OVMF.fd \
		-drive format=raw,file=fat:rw:build/esp \
		-display none -serial stdio
```

## Secure Boot

The repository includes a development key workflow in [scripts/secureboot.sh](scripts/secureboot.sh). Private keys stay local and are ignored by Git.

```bash
make secureboot-keys
make secureboot-sign
make secureboot-verify
make secureboot-efi-tree
```

The EFI loader contains the generated SHA-256 digest of `kernel.elf`. Signing the EFI image therefore anchors the exact kernel version it accepts. Actual firmware enrollment of PK, KEK, and db certificates must be performed in an isolated OVMF variable store or deliberately configured physical firmware. See [docs/security.md](docs/security.md).

## Source layout

- [uefi/](uefi/) - UEFI ABI, PE32+ loader, ELF validation, GOP, BootInfo, and kernel integrity checks
- [proc/kernel/](proc/kernel/) - x86_64 kernel entry, linker contract, and shared boot structures
- [proc/terminal/](proc/terminal/) - VGA text terminal
- [proc/keyboard/](proc/keyboard/) - PS/2 scan-code input
- [proc/shell/](proc/shell/) - interactive command shell
- [drivers/](drivers/) - driver registry, PS/2 driver, and COM1 serial driver
- [fs/](fs/) - in-memory VFS with UID/GID and protected entries
- [mnt/](mnt/) - mount registry for RAM and future physical devices
- [var/](var/) - runtime logs, cache, state, and temporary data
- [lib/](lib/) - freestanding string helpers
- [docs/](docs/) - architecture and security notes

## Runtime model

The VFS creates `/mnt`, `/proc`, and `/var` during kernel initialization. Kernel logs use `/var/log/kernel.log`; kernel cache uses `/var/cache/kernel.cache`; RAM storage mounts at `/mnt/ram`. The current storage layer is memory-backed, so persistence across reboot is a planned filesystem backend.

UID 0 is root for ordinary VFS entries. Protected hidden boot records cannot be read, changed, or removed through the MyOS VFS, including by root. This policy does not restrict the host operating system from modifying the project or disk.

## Development principles

- Keep UEFI and kernel ABI changes explicit and reviewable.
- Validate external data before using it as a pointer, size, offset, or entry point.
- Keep private signing keys outside version control.
- Test boot behavior in OVMF before claiming a loader change is complete.
