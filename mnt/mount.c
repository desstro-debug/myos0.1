#include "mount.h"
#include "string.h"
#include "terminal.h"
#include "vfs.h"

static mnt_record g_mounts[MNT_MAX_MOUNTS];
static unsigned int g_mount_count;

const char *mnt_type_name(mnt_device_type type) {
    switch (type) {
        case MNT_DEVICE_RAM:
            return "ram";
        case MNT_DEVICE_USB:
            return "usb";
        case MNT_DEVICE_HDD:
            return "hdd";
        case MNT_DEVICE_CDROM:
            return "cdrom";
        default:
            return "unknown";
    }
}

void mnt_init(void) {
    g_mount_count = 0;
    mnt_mount("ram0", "mnt/ram", MNT_DEVICE_RAM);
}

int mnt_mount(const char *device, const char *path, mnt_device_type type) {
    fs_entry *entry;
    mnt_record *record;
    unsigned int slot;

    if (device == 0 || path == 0) {
        return -1;
    }

    entry = vfs_find(path);
    if (entry == 0 || entry->is_directory == 0) {
        return -1;
    }

    for (unsigned int i = 0; i < g_mount_count; ++i) {
        if (g_mounts[i].mounted != 0 && strcmp(g_mounts[i].path, path) == 0) {
            return -1;
        }
    }

    slot = g_mount_count;
    for (unsigned int i = 0; i < g_mount_count; ++i) {
        if (g_mounts[i].mounted == 0) {
            slot = i;
            break;
        }
    }
    if (slot == g_mount_count) {
        if (g_mount_count >= MNT_MAX_MOUNTS) {
            return -1;
        }
        g_mount_count++;
    }

    record = &g_mounts[slot];
    strncpy(record->device, device, MNT_NAME_SIZE - 1);
    record->device[MNT_NAME_SIZE - 1] = '\0';
    strncpy(record->path, path, MNT_NAME_SIZE - 1);
    record->path[MNT_NAME_SIZE - 1] = '\0';
    record->type = type;
    record->mounted = 1;
    return 0;
}

int mnt_unmount(const char *path) {
    for (unsigned int i = 0; i < g_mount_count; ++i) {
        if (g_mounts[i].mounted != 0 && strcmp(g_mounts[i].path, path) == 0) {
            g_mounts[i].mounted = 0;
            return 0;
        }
    }
    return -1;
}

void mnt_list(void) {
    terminal_write_line("Mounted devices:");
    for (unsigned int i = 0; i < g_mount_count; ++i) {
        if (g_mounts[i].mounted == 0) {
            continue;
        }
        terminal_write("  ");
        terminal_write(g_mounts[i].device);
        terminal_write(" -> /");
        terminal_write(g_mounts[i].path);
        terminal_write(" [");
        terminal_write(mnt_type_name(g_mounts[i].type));
        terminal_write_line("]");
    }
}
