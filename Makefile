CC := gcc
LD := ld
OBJCOPY := objcopy
CFLAGS := -m32 -ffreestanding -fno-builtin -fno-stack-protector -fno-pie -O2 -Wall -Wextra -I kernel
LDFLAGS := -m elf_i386 -nostdlib -T kernel/linker.ld -z noexecstack

BUILD_DIR := build
KERNEL_SRCS := kernel/kernel.c kernel/terminal.c kernel/keyboard.c kernel/shell.c
KERNEL_OBJS := $(BUILD_DIR)/kernel.o $(BUILD_DIR)/terminal.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/shell.o
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin

.PHONY: all clean

all: $(KERNEL_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/kernel.o: kernel/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/terminal.o: kernel/terminal.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/keyboard.o: kernel/keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/shell.o: kernel/shell.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KERNEL_ELF): $(KERNEL_OBJS) kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)
