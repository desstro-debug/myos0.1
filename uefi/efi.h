#ifndef MYOS_UEFI_H
#define MYOS_UEFI_H

#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0
#define EFI_INVALID_PARAMETER 2
#define EFI_BUFFER_TOO_SMALL 5
#define EFI_LOADER_DATA 4
#define EFI_ALLOCATE_ADDRESS 2
#define EFI_READ_ONLY 1
#define EFI_OPEN_PROTOCOL 0x00000002

typedef unsigned long long efi_status;
typedef unsigned long long efi_uintn;
typedef unsigned long long efi_physical_address;
typedef unsigned long long efi_virtual_address;
typedef unsigned long long efi_handle;
typedef unsigned short efi_char16;
typedef unsigned char efi_boolean;
typedef struct { unsigned char bytes[16]; } efi_guid;
typedef struct { unsigned long long value; } efi_time;

typedef struct efi_table_header {
    unsigned long long signature;
    unsigned int revision;
    unsigned int header_size;
    unsigned int crc32;
    unsigned int reserved;
} efi_table_header;

typedef struct efi_file efi_file;
typedef efi_status (EFIAPI *efi_file_open)(efi_file *, efi_file **, efi_char16 *, unsigned long long, unsigned long long);
typedef efi_status (EFIAPI *efi_file_close)(efi_file *);
typedef efi_status (EFIAPI *efi_file_read)(efi_file *, efi_uintn *, void *);
typedef efi_status (EFIAPI *efi_file_get_info)(efi_file *, efi_guid *, efi_uintn *, void *);

typedef struct efi_file {
    unsigned long long revision;
    efi_file_open open;
    efi_file_close close;
    void *delete_file;
    efi_file_read read;
    void *write;
    void *get_position;
    void *set_position;
    efi_file_get_info get_info;
    void *set_info;
    void *flush;
    void *open_ex;
    void *close_ex;
    void *delete_ex;
    void *read_ex;
    void *write_ex;
    void *flush_ex;
} efi_file;

typedef struct efi_simple_file_system {
    unsigned long long revision;
    efi_status (EFIAPI *open_volume)(struct efi_simple_file_system *, efi_file **);
} efi_simple_file_system;

typedef struct efi_graphics_output_mode_info {
    unsigned int version;
    unsigned int horizontal_resolution;
    unsigned int vertical_resolution;
    unsigned int pixel_format;
    unsigned int pixel_information[4];
    unsigned int pixels_per_scanline;
} efi_graphics_output_mode_info;

typedef struct efi_graphics_output_mode {
    unsigned int max_mode;
    unsigned int mode;
    efi_graphics_output_mode_info *info;
    efi_uintn info_size;
    efi_physical_address framebuffer_base;
    efi_uintn framebuffer_size;
} efi_graphics_output_mode;

typedef struct efi_graphics_output {
    efi_status (EFIAPI *query_mode)(struct efi_graphics_output *, unsigned int, efi_uintn *, efi_graphics_output_mode_info **);
    efi_status (EFIAPI *set_mode)(struct efi_graphics_output *, unsigned int);
    void *blt;
    efi_graphics_output_mode *mode;
} efi_graphics_output;

typedef struct efi_loaded_image {
    unsigned int revision;
    efi_handle parent_handle;
    void *system_table;
    efi_handle device_handle;
    void *file_path;
    void *reserved;
    unsigned int load_options_size;
    void *load_options;
} efi_loaded_image;

typedef efi_status (EFIAPI *efi_output_string)(void *, efi_char16 *);
typedef struct efi_simple_text_output {
    void *reset;
    efi_output_string output_string;
    void *test_string;
    void *query_mode;
    void *set_mode;
    void *set_attribute;
    void *clear_screen;
    void *set_cursor_position;
    void *enable_cursor;
    void *mode;
} efi_simple_text_output;

typedef struct efi_boot_services {
    efi_table_header header;
    void *raise_tpl;
    void *restore_tpl;
    efi_status (EFIAPI *allocate_pages)(unsigned int, unsigned int, efi_uintn, efi_physical_address *);
    efi_status (EFIAPI *free_pages)(efi_physical_address, efi_uintn);
    efi_status (EFIAPI *get_memory_map)(efi_uintn *, void *, unsigned long long *, efi_uintn *, unsigned int *);
    efi_status (EFIAPI *allocate_pool)(unsigned int, efi_uintn, void **);
    efi_status (EFIAPI *free_pool)(void *);
    void *create_event;
    void *set_timer;
    void *wait_for_event;
    void *signal_event;
    void *close_event;
    void *check_event;
    void *install_protocol_interface;
    void *reinstall_protocol_interface;
    void *uninstall_protocol_interface;
    efi_status (EFIAPI *handle_protocol)(efi_handle, efi_guid *, void **);
    void *reserved;
    void *register_protocol_notify;
    void *locate_handle;
    void *locate_device_path;
    void *install_configuration_table;
    void *load_image;
    void *start_image;
    void *exit;
    void *unload_image;
    efi_status (EFIAPI *exit_boot_services)(efi_handle, unsigned long long);
    void *get_next_monotonic_count;
    void *stall;
    void *set_watchdog_timer;
    void *connect_controller;
    void *disconnect_controller;
    void *open_protocol;
    void *close_protocol;
    void *open_protocol_information;
    void *protocols_per_handle;
    void *locate_handle_buffer;
    efi_status (EFIAPI *locate_protocol)(efi_guid *, void *, void **);
} efi_boot_services;

typedef struct efi_system_table {
    efi_table_header header;
    efi_char16 *firmware_vendor;
    unsigned int firmware_revision;
    efi_handle console_in_handle;
    void *con_in;
    efi_handle console_out_handle;
    efi_simple_text_output *con_out;
    efi_handle standard_error_handle;
    efi_simple_text_output *std_err;
    void *runtime_services;
    efi_boot_services *boot_services;
} efi_system_table;

typedef void (EFIAPI *efi_kernel_entry)(void *);

#endif
