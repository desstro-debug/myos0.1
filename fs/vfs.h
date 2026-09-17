#ifndef VFS_H
#define VFS_H

#include "kernel.h"

#define FS_MAX_NAME 32
#define FS_MAX_ENTRIES 64
#define FS_MAX_FILE_DATA 256
#define FS_MOUNT_PROC "proc"
#define FS_MOUNT_MNT  "mnt"
#define FS_MOUNT_VAR  "var"

#define FS_TYPE_FILE 0
#define FS_TYPE_DIR  1

#define FS_FLAG_SYSTEM 0x01
#define FS_FLAG_RUNTIME 0x02
#define FS_FLAG_TEMP 0x04
#define FS_FLAG_PROTECTED 0x08
#define FS_FLAG_HIDDEN 0x10

#define FS_UID_ROOT 0
#define FS_UID_SYSTEM 1
#define FS_UID_USER 1000
#define FS_GID_ROOT 0
#define FS_GID_SYSTEM 1
#define FS_GID_USER 1000

typedef struct {
    unsigned int uid;
    unsigned int gid;
} fs_credentials;

typedef struct {
    char name[FS_MAX_NAME];
    unsigned int size;
    unsigned int start_sector;
    unsigned char is_directory;
    unsigned char type_flags;
    unsigned int owner_uid;
    unsigned int owner_gid;
    unsigned short mode;
    char data[FS_MAX_FILE_DATA];
} fs_entry;

typedef struct {
    fs_entry entries[FS_MAX_ENTRIES];
    unsigned int count;
} fs_filesystem;

void vfs_init(void);
void vfs_install_default_layout(void);
int vfs_create_file(const char *name, unsigned int size, unsigned int start_sector);
int vfs_create_dir(const char *name);
fs_entry *vfs_find(const char *name);
int vfs_append_file(const char *name, const char *data);
int vfs_can_read(const fs_entry *entry, fs_credentials credentials);
int vfs_can_write(const fs_entry *entry, fs_credentials credentials);
int vfs_remove(const char *name, fs_credentials credentials);
void vfs_list(void);

#endif
