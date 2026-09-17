#ifndef MOUNT_H
#define MOUNT_H

#define MNT_MAX_MOUNTS 8
#define MNT_NAME_SIZE 32

typedef enum {
    MNT_DEVICE_RAM = 0,
    MNT_DEVICE_USB,
    MNT_DEVICE_HDD,
    MNT_DEVICE_CDROM
} mnt_device_type;

typedef struct {
    char device[MNT_NAME_SIZE];
    char path[MNT_NAME_SIZE];
    mnt_device_type type;
    unsigned char mounted;
} mnt_record;

void mnt_init(void);
int mnt_mount(const char *device, const char *path, mnt_device_type type);
int mnt_unmount(const char *path);
void mnt_list(void);
const char *mnt_type_name(mnt_device_type type);

#endif
