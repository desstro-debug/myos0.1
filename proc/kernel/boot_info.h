#ifndef MYOS_BOOT_INFO_H
#define MYOS_BOOT_INFO_H

#define MYOS_BOOT_INFO_VERSION 1
#define MYOS_BOOT_INFO_MAGIC 0x4D594F53424F4F54ULL
#define MYOS_BOOT_INFO_FLAG_MEMORY_MAP 0x01
#define MYOS_BOOT_INFO_FLAG_FRAMEBUFFER 0x02
#define MYOS_BOOT_INFO_FLAG_CONFIG 0x04
#define MYOS_BOOT_INFO_FLAG_RECOVERY 0x08

#define MYOS_PIXEL_RGBR 0
#define MYOS_PIXEL_BGRR 1
#define MYOS_PIXEL_BITMASK 2
#define MYOS_PIXEL_BLT_ONLY 3

typedef struct myos_memory_map_info {
    unsigned long long address;
    unsigned long long size;
    unsigned long long descriptor_size;
    unsigned int descriptor_version;
    unsigned int reserved;
} myos_memory_map_info;

typedef struct myos_framebuffer_info {
    unsigned long long address;
    unsigned long long size;
    unsigned int width;
    unsigned int height;
    unsigned int pixels_per_scanline;
    unsigned int pixel_format;
} myos_framebuffer_info;

typedef struct myos_boot_info {
    unsigned long long magic;
    unsigned int version;
    unsigned int size;
    unsigned int flags;
    unsigned int reserved;
    myos_memory_map_info memory_map;
    myos_framebuffer_info framebuffer;
    unsigned long long loader_image;
    unsigned long long kernel_address;
    unsigned long long kernel_size;
    const char *config_path;
} myos_boot_info;

#endif
