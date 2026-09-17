#include "vfs.h"
#include "terminal.h"
#include "string.h"

static fs_filesystem g_fs;

static int vfs_add_entry(const char *name, unsigned int size, unsigned int start_sector, unsigned char is_directory, unsigned char flags) {
    if (g_fs.count >= FS_MAX_ENTRIES) {
        return -1;
    }

    fs_entry *e = &g_fs.entries[g_fs.count++];
    int i = 0;
    while (name[i] != '\0' && i < FS_MAX_NAME - 1) {
        e->name[i] = name[i];
        i++;
    }
    e->name[i] = '\0';
    e->size = size;
    e->start_sector = start_sector;
    e->is_directory = is_directory;
    e->type_flags = flags;
    e->owner_uid = FS_UID_ROOT;
    e->owner_gid = FS_GID_ROOT;
    e->mode = is_directory ? 0755 : 0644;
    if ((flags & FS_FLAG_PROTECTED) != 0) {
        e->mode = 0000;
    }
    e->data[0] = '\0';
    return 0;
}

void vfs_init(void) {
    g_fs.count = 0;
    vfs_install_default_layout();
}

void vfs_install_default_layout(void) {
    vfs_add_entry(FS_MOUNT_MNT, 0, 0, 1, FS_FLAG_SYSTEM);
    vfs_add_entry(FS_MOUNT_PROC, 0, 0, 1, FS_FLAG_SYSTEM);
    vfs_add_entry(FS_MOUNT_VAR, 0, 0, 1, FS_FLAG_SYSTEM);

    vfs_add_entry("var/log", 0, 0, 1, FS_FLAG_RUNTIME);
    vfs_add_entry("var/log/kernel.log", 0, 0, 0, FS_FLAG_RUNTIME);
    vfs_add_entry("var/cache", 0, 0, 1, FS_FLAG_TEMP);
    vfs_add_entry("var/cache/kernel.cache", 0, 0, 0, FS_FLAG_TEMP);
    vfs_add_entry("var/run", 0, 0, 1, FS_FLAG_RUNTIME);
    vfs_add_entry("var/tmp", 0, 0, 1, FS_FLAG_TEMP);

    vfs_add_entry("proc/version", 0, 0, 0, FS_FLAG_SYSTEM);
    vfs_add_entry("proc/meminfo", 0, 0, 0, FS_FLAG_SYSTEM);
    vfs_add_entry("proc/cpuinfo", 0, 0, 0, FS_FLAG_SYSTEM);
    vfs_add_entry("proc/kernel", 0, 0, 1, FS_FLAG_SYSTEM);
    vfs_add_entry("proc/kernel/cache", 0, 0, 1, FS_FLAG_TEMP);
    vfs_add_entry("proc/terminal", 0, 0, 1, FS_FLAG_SYSTEM);
    vfs_add_entry("proc/keyboard", 0, 0, 1, FS_FLAG_SYSTEM);
    vfs_add_entry("proc/shell", 0, 0, 1, FS_FLAG_SYSTEM);

    vfs_add_entry("boot", 0, 0, 1, FS_FLAG_SYSTEM | FS_FLAG_PROTECTED | FS_FLAG_HIDDEN);
    vfs_add_entry("boot/boot.bin", 0, 0, 0, FS_FLAG_SYSTEM | FS_FLAG_PROTECTED | FS_FLAG_HIDDEN);

    vfs_add_entry("mnt/usb", 0, 0, 1, FS_FLAG_RUNTIME);
    vfs_add_entry("mnt/data", 0, 0, 1, FS_FLAG_RUNTIME);
    vfs_add_entry("mnt/ram", 0, 0, 1, FS_FLAG_RUNTIME);
    vfs_add_entry("mnt/hdd", 0, 0, 1, FS_FLAG_RUNTIME);
    vfs_add_entry("mnt/cdrom", 0, 0, 1, FS_FLAG_RUNTIME);
}

int vfs_create_file(const char *name, unsigned int size, unsigned int start_sector) {
    int result = vfs_add_entry(name, size, start_sector, 0, FS_FLAG_RUNTIME);
    if (result == 0) {
        fs_entry *entry = &g_fs.entries[g_fs.count - 1];
        entry->owner_uid = FS_UID_USER;
        entry->owner_gid = FS_GID_USER;
    }
    return result;
}

int vfs_create_dir(const char *name) {
    int result = vfs_add_entry(name, 0, 0, 1, FS_FLAG_RUNTIME);
    if (result == 0) {
        fs_entry *entry = &g_fs.entries[g_fs.count - 1];
        entry->owner_uid = FS_UID_USER;
        entry->owner_gid = FS_GID_USER;
    }
    return result;
}

fs_entry *vfs_find(const char *name) {
    for (unsigned int i = 0; i < g_fs.count; ++i) {
        if (strcmp(g_fs.entries[i].name, name) == 0) {
            if ((g_fs.entries[i].type_flags & FS_FLAG_HIDDEN) != 0) {
                return 0;
            }
            return &g_fs.entries[i];
        }
    }
    return 0;
}

int vfs_can_read(const fs_entry *entry, fs_credentials credentials) {
    unsigned short permission;

    if (entry == 0 || (entry->type_flags & FS_FLAG_PROTECTED) != 0) {
        return 0;
    }
    if (credentials.uid == FS_UID_ROOT) {
        return 1;
    }
    if (credentials.uid == entry->owner_uid) {
        permission = (entry->mode >> 6) & 07;
    } else if (credentials.gid == entry->owner_gid) {
        permission = (entry->mode >> 3) & 07;
    } else {
        permission = entry->mode & 07;
    }
    return (permission & 04) != 0;
}

int vfs_can_write(const fs_entry *entry, fs_credentials credentials) {
    unsigned short permission;

    if (entry == 0 || (entry->type_flags & FS_FLAG_PROTECTED) != 0) {
        return 0;
    }
    if (credentials.uid == FS_UID_ROOT) {
        return 1;
    }
    if (credentials.uid == entry->owner_uid) {
        permission = (entry->mode >> 6) & 07;
    } else if (credentials.gid == entry->owner_gid) {
        permission = (entry->mode >> 3) & 07;
    } else {
        permission = entry->mode & 07;
    }
    return (permission & 02) != 0;
}

int vfs_remove(const char *name, fs_credentials credentials) {
    unsigned int index;
    fs_entry *entry = vfs_find(name);

    if (entry == 0 || !vfs_can_write(entry, credentials)) {
        return -1;
    }

    index = (unsigned int)(entry - g_fs.entries);
    for (; index + 1 < g_fs.count; ++index) {
        g_fs.entries[index] = g_fs.entries[index + 1];
    }
    g_fs.count--;
    return 0;
}

int vfs_append_file(const char *name, const char *data) {
    fs_entry *entry = vfs_find(name);
    unsigned int offset;
    unsigned int index;

    if (entry == 0 || entry->is_directory != 0) {
        return -1;
    }

    offset = entry->size;
    for (index = 0; data[index] != '\0' && offset < FS_MAX_FILE_DATA - 1; ++index) {
        entry->data[offset++] = data[index];
    }
    entry->data[offset] = '\0';
    entry->size = offset;
    return data[index] == '\0' ? 0 : -1;
}

void vfs_list(void) {
    terminal_write_line("Filesystem root:");
    for (unsigned int i = 0; i < g_fs.count; ++i) {
        if ((g_fs.entries[i].type_flags & FS_FLAG_HIDDEN) != 0) {
            continue;
        }
        terminal_write("  - ");
        terminal_write(g_fs.entries[i].name);
        terminal_write(g_fs.entries[i].is_directory ? " [dir]" : " [file]");
        terminal_write("\n");
    }
}
