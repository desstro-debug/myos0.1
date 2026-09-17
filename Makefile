CC := gcc
UEFI_CC := clang
LD := ld
OBJCOPY := objcopy
CFLAGS := -m64 -ffreestanding -fno-builtin -fno-stack-protector -fno-pie -fno-omit-frame-pointer -mno-red-zone -mcmodel=small -O2 -Wall -Wextra -I proc/kernel -I proc/terminal -I proc/keyboard -I proc/shell -I mnt -I lib -I fs -I drivers
LDFLAGS := -m elf_x86_64 -nostdlib -T proc/kernel/linker.ld -z noexecstack

BUILD_DIR := build
UEFI_SRC := uefi/bootloader.c
UEFI_CFLAGS := --target=x86_64-pc-win32 -ffreestanding -fshort-wchar -fno-stack-protector -fno-builtin -fno-pie -mno-red-zone -O2 -Wall -Wextra -I build -I uefi -I proc/kernel
.PHONY: all uefi efi-tree secureboot-keys secureboot-sign secureboot-verify secureboot-efi-tree clean
UEFI_EFI := $(BUILD_DIR)/BOOTX64.EFI
KERNEL_SRCS := proc/kernel/kernel.c proc/terminal/terminal.c proc/keyboard/keyboard.c proc/shell/shell.c fs/vfs.c lib/string.c mnt/mount.c drivers/driver_manager.c drivers/keyboard_driver.c drivers/serial_driver.c
KERNEL_OBJS := $(BUILD_DIR)/kernel.o $(BUILD_DIR)/terminal.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/vfs.o $(BUILD_DIR)/string.o $(BUILD_DIR)/mount.o $(BUILD_DIR)/driver_manager.o $(BUILD_DIR)/keyboard_driver.o $(BUILD_DIR)/serial_driver.o
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin

all: $(KERNEL_BIN) $(UEFI_EFI)

secureboot-keys:
	./scripts/secureboot.sh init

secureboot-sign: all
	./scripts/secureboot.sh sign

secureboot-verify:
	./scripts/secureboot.sh verify

secureboot-efi-tree: secureboot-sign
	mkdir -p build/esp/EFI/BOOT build/esp/EFI/MYOS
	cp build/BOOTX64.SIGNED.EFI build/esp/EFI/BOOT/BOOTX64.EFI
	cp $(KERNEL_ELF) build/esp/EFI/MYOS/kernel.elf

uefi: $(UEFI_EFI)

efi-tree: all
	mkdir -p build/esp/EFI/BOOT build/esp/EFI/MYOS
	cp $(UEFI_EFI) build/esp/EFI/BOOT/BOOTX64.EFI
	cp $(KERNEL_ELF) build/esp/EFI/MYOS/kernel.elf

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/kernel_digest.h: $(KERNEL_ELF) scripts/make_kernel_digest.sh | $(BUILD_DIR)
	./scripts/make_kernel_digest.sh $(KERNEL_ELF) $@

$(BUILD_DIR)/uefi.o: $(UEFI_SRC) uefi/efi.h proc/kernel/boot_info.h $(BUILD_DIR)/kernel_digest.h | $(BUILD_DIR)
	$(UEFI_CC) $(UEFI_CFLAGS) -c -o $@ $<

$(UEFI_EFI): $(BUILD_DIR)/uefi.o uefi/efi.ld | $(BUILD_DIR)
	$(LD) -m i386pep -nostdlib --image-base 0x0 --subsystem 10 --major-subsystem-version 2 --minor-subsystem-version 0 -T uefi/efi.ld -o $@ $(BUILD_DIR)/uefi.o

$(BUILD_DIR)/kernel.o: proc/kernel/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/terminal.o: proc/terminal/terminal.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/keyboard.o: proc/keyboard/keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/shell.o: proc/shell/shell.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/vfs.o: fs/vfs.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/string.o: lib/string.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/mount.o: mnt/mount.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/driver_manager.o: drivers/driver_manager.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/keyboard_driver.o: drivers/keyboard_driver.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/serial_driver.o: drivers/serial_driver.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KERNEL_ELF): $(KERNEL_OBJS) proc/kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)
