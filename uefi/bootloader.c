#include "efi.h"
#include "boot_info.h"
#include "kernel_digest.h"

#define KERNEL_LOAD_ADDRESS 0x00100000ULL
#define KERNEL_MAX_SIZE (1024ULL * 1024ULL)
#define MEMORY_MAP_BUFFER_SIZE (64ULL * 1024ULL)

static efi_system_table *g_system_table;
static efi_boot_services *g_boot_services;
static efi_handle g_image_handle;
static myos_boot_info g_boot_info;

static const efi_guid g_loaded_image_guid = {{
    0xA1, 0x31, 0x1B, 0x5B, 0x62, 0x95, 0xD2, 0x11,
    0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B
}};

static const efi_guid g_simple_file_system_guid = {{
    0x22, 0x5B, 0x4E, 0x96, 0x59, 0x64, 0xD2, 0x11,
    0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B
}};

static const efi_guid g_graphics_output_protocol_guid = {{
    0xDE, 0xA9, 0x42, 0x90, 0xDC, 0x23, 0x38, 0x4A,
    0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A
}};

static const efi_char16 g_kernel_path[] = {
    '\\', 'E', 'F', 'I', '\\', 'M', 'Y', 'O', 'S', '\\',
    'k', 'e', 'r', 'n', 'e', 'l', '.', 'e', 'l', 'f', 0
};

static const efi_char16 g_banner[] = {
    'M','y','O','S',' ','U','E','F','I',' ','x','8','6','_','6','4','\r','\n',0
};
static const efi_char16 g_loading[] = {
    'L','o','a','d','i','n','g',' ','k','e','r','n','e','l','.','e','l','f','\r','\n',0
};
static const efi_char16 g_ready[] = {
    'K','e','r','n','e','l',' ','l','o','a','d','e','d','.','\r','\n',0
};
static const efi_char16 g_error[] = {
    'U','E','F','I',' ','l','o','a','d','e','r',' ','e','r','r','o','r','\r','\n',0
};
static const efi_char16 g_error_loaded_image[] = {
    'L','o','a','d','e','d','I','m','a','g','e',' ','p','r','o','t','o','c','o','l',' ','e','r','r','o','r','\r','\n',0
};
static const efi_char16 g_error_file_system[] = {
    'S','i','m','p','l','e','F','S',' ','p','r','o','t','o','c','o','l',' ','e','r','r','o','r','\r','\n',0
};
static const efi_char16 g_error_volume[] = {
    'F','S',' ','v','o','l','u','m','e',' ','o','p','e','n',' ','e','r','r','o','r','\r','\n',0
};
static const efi_char16 g_error_path[] = {
    'k','e','r','n','e','l',' ','p','a','t','h',' ','o','p','e','n',' ','e','r','r','o','r','\r','\n',0
};
static const efi_char16 g_error_memory[] = {
    'k','e','r','n','e','l',' ','m','e','m','o','r','y',' ','a','l','l','o','c','a','t','i','o','n',' ','e','r','r','o','r','\r','\n',0
};
static const efi_char16 g_error_read[] = {
    'k','e','r','n','e','l',' ','r','e','a','d',' ','e','r','r','o','r','\r','\n',0
};
static const efi_char16 g_error_elf[] = {
    'i','n','v','a','l','i','d',' ','E','L','F',' ','k','e','r','n','e','l','\r','\n',0
};
static const efi_char16 g_error_elf_header[] = {
    'i','n','v','a','l','i','d',' ','E','L','F',' ','h','e','a','d','e','r','\r','\n',0
};
static const efi_char16 g_error_elf_segments[] = {
    'i','n','v','a','l','i','d',' ','E','L','F',' ','s','e','g','m','e','n','t','s','\r','\n',0
};
static const efi_char16 g_error_digest[] = {
    'k','e','r','n','e','l',' ','h','a','s','h',' ','m','i','s','m','a','t','c','h','\r','\n',0
};
static const efi_char16 g_framebuffer_ready[] = {
    'F','r','a','m','e','b','u','f','f','e','r',' ','r','e','a','d','y','\r','\n',0
};

static void print_text(const efi_char16 *text) {
    if (g_system_table != 0 && g_system_table->con_out != 0) {
        g_system_table->con_out->output_string(g_system_table->con_out, (efi_char16 *)text);
    }
}

static int status_failed(efi_status status) {
    return status != EFI_SUCCESS;
}

typedef struct {
    unsigned char identity[16];
    unsigned short type;
    unsigned short machine;
    unsigned int version;
    unsigned long long entry;
    unsigned long long program_header_offset;
    unsigned long long section_header_offset;
    unsigned int flags;
    unsigned short header_size;
    unsigned short program_header_size;
    unsigned short program_header_count;
    unsigned short section_header_size;
    unsigned short section_header_count;
    unsigned short string_table_index;
} elf64_header;

typedef struct {
    unsigned int type;
    unsigned int flags;
    unsigned long long offset;
    unsigned long long virtual_address;
    unsigned long long physical_address;
    unsigned long long file_size;
    unsigned long long memory_size;
    unsigned long long alignment;
} elf64_program_header;

static void memory_copy(void *destination, const void *source, unsigned long long size);
static void memory_zero(void *destination, unsigned long long size);

static const unsigned int sha256_round_constants[64] = {
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
    0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
    0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
    0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
    0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
    0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
    0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
    0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
    0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
    0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

static unsigned int rotate_right(unsigned int value, unsigned int count) {
    return (value >> count) | (value << (32 - count));
}

static unsigned int sha_ch(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (~x & z);
}

static unsigned int sha_majority(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

static void sha256_transform(unsigned int state[8], const unsigned char block[64]) {
    unsigned int words[64];
    unsigned int a, b, c, d, e, f, g, h;
    unsigned int index;

    for (index = 0; index < 16; ++index) {
        words[index] = ((unsigned int)block[index * 4] << 24) |
            ((unsigned int)block[index * 4 + 1] << 16) |
            ((unsigned int)block[index * 4 + 2] << 8) |
            (unsigned int)block[index * 4 + 3];
    }
    for (index = 16; index < 64; ++index) {
        unsigned int s0 = rotate_right(words[index - 15], 7) ^
            rotate_right(words[index - 15], 18) ^ (words[index - 15] >> 3);
        unsigned int s1 = rotate_right(words[index - 2], 17) ^
            rotate_right(words[index - 2], 19) ^ (words[index - 2] >> 10);
        words[index] = words[index - 16] + s0 + words[index - 7] + s1;
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];
    for (index = 0; index < 64; ++index) {
        unsigned int sum1 = rotate_right(e, 6) ^ rotate_right(e, 11) ^ rotate_right(e, 25);
        unsigned int temporary1 = h + sum1 + sha_ch(e, f, g) + sha256_round_constants[index] + words[index];
        unsigned int sum0 = rotate_right(a, 2) ^ rotate_right(a, 13) ^ rotate_right(a, 22);
        unsigned int temporary2 = sum0 + sha_majority(a, b, c);
        h = g; g = f; f = e; e = d + temporary1;
        d = c; c = b; b = a; a = temporary1 + temporary2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

static void sha256_buffer(const unsigned char *data, efi_uintn size, unsigned char digest[32]) {
    unsigned int state[8] = {
        0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
        0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
    };
    unsigned char block[64];
    efi_uintn full_blocks = size / 64;
    efi_uintn remainder = size % 64;
    unsigned long long bit_length = size * 8;
    unsigned int index;

    for (index = 0; index < full_blocks; ++index) {
        sha256_transform(state, data + index * 64);
    }
    memory_zero(block, sizeof(block));
    memory_copy(block, data + full_blocks * 64, remainder);
    block[remainder] = 0x80;
    if (remainder >= 56) {
        sha256_transform(state, block);
        memory_zero(block, sizeof(block));
    }
    for (index = 0; index < 8; ++index) {
        block[63 - index] = (unsigned char)(bit_length >> (index * 8));
    }
    sha256_transform(state, block);
    for (index = 0; index < 8; ++index) {
        digest[index * 4] = (unsigned char)(state[index] >> 24);
        digest[index * 4 + 1] = (unsigned char)(state[index] >> 16);
        digest[index * 4 + 2] = (unsigned char)(state[index] >> 8);
        digest[index * 4 + 3] = (unsigned char)state[index];
    }
}

static int digest_matches(const unsigned char actual[32]) {
    unsigned int index;
    unsigned char difference = 0;
    for (index = 0; index < 32; ++index) {
        difference |= actual[index] ^ myos_kernel_sha256[index];
    }
    return difference == 0;
}

static void memory_copy(void *destination, const void *source, unsigned long long size) {
    unsigned char *out = destination;
    const unsigned char *in = source;
    while (size-- != 0) {
        *out++ = *in++;
    }
}

static void memory_zero(void *destination, unsigned long long size) {
    unsigned char *out = destination;
    while (size-- != 0) {
        *out++ = 0;
    }
}

static int elf_is_valid(const elf64_header *header, efi_uintn size) {
    if (size < sizeof(elf64_header) || header->identity[0] != 0x7F ||
        header->identity[1] != 'E' || header->identity[2] != 'L' ||
        header->identity[3] != 'F') {
        return 0;
    }
    if (header->identity[4] != 2 || header->identity[5] != 1 ||
        header->machine != 0x3E || header->type != 2 ||
        header->program_header_size != sizeof(elf64_program_header)) {
        return 0;
    }
    return header->program_header_count != 0;
}

static int elf_program_is_valid(const elf64_program_header *program, efi_uintn size) {
    if (program->type != 1 || program->memory_size < program->file_size) {
        return 0;
    }
    if (program->offset > size || program->file_size > size - program->offset) {
        return 0;
    }
    if (program->virtual_address + program->memory_size < program->virtual_address) {
        return 0;
    }
    return 1;
}

static void boot_info_init(void) {
    unsigned char *bytes = (unsigned char *)&g_boot_info;
    unsigned long long index;

    for (index = 0; index < sizeof(g_boot_info); ++index) {
        bytes[index] = 0;
    }
    g_boot_info.magic = MYOS_BOOT_INFO_MAGIC;
    g_boot_info.version = MYOS_BOOT_INFO_VERSION;
    g_boot_info.size = sizeof(g_boot_info);
    g_boot_info.loader_image = (unsigned long long)g_image_handle;
    g_boot_info.config_path = "\\EFI\\MYOS\\myos.cfg";
}

static void query_framebuffer(void) {
    efi_graphics_output *graphics = 0;
    efi_graphics_output_mode *mode;
    efi_status status;

    status = g_boot_services->locate_protocol(
        (efi_guid *)&g_graphics_output_protocol_guid,
        0,
        (void **)&graphics
    );
    if (status_failed(status) || graphics == 0 || graphics->mode == 0 || graphics->mode->info == 0) {
        return;
    }

    mode = graphics->mode;
    g_boot_info.framebuffer.address = mode->framebuffer_base;
    g_boot_info.framebuffer.size = mode->framebuffer_size;
    g_boot_info.framebuffer.width = mode->info->horizontal_resolution;
    g_boot_info.framebuffer.height = mode->info->vertical_resolution;
    g_boot_info.framebuffer.pixels_per_scanline = mode->info->pixels_per_scanline;
    g_boot_info.framebuffer.pixel_format = mode->info->pixel_format;
    g_boot_info.flags |= MYOS_BOOT_INFO_FLAG_FRAMEBUFFER;
    print_text(g_framebuffer_ready);
}

static efi_status open_kernel_file(efi_file **kernel_file) {
    efi_loaded_image *loaded_image = 0;
    efi_simple_file_system *file_system = 0;
    efi_file *root = 0;
    efi_status status;

    status = g_boot_services->handle_protocol(
        g_image_handle,
        (efi_guid *)&g_loaded_image_guid,
        (void **)&loaded_image
    );
    if (status_failed(status) || loaded_image == 0) {
        print_text(g_error_loaded_image);
        return status;
    }
    status = g_boot_services->handle_protocol(
        loaded_image->device_handle,
        (efi_guid *)&g_simple_file_system_guid,
        (void **)&file_system
    );
    if (status_failed(status) || file_system == 0) {
        print_text(g_error_file_system);
        return status;
    }

    status = file_system->open_volume(file_system, &root);
    if (status_failed(status) || root == 0) {
        print_text(g_error_volume);
        return status;
    }

    status = root->open(root, kernel_file, (efi_char16 *)g_kernel_path, EFI_READ_ONLY, 0);
    root->close(root);
    if (status_failed(status)) {
        print_text(g_error_path);
    }
    return status;
}

static efi_status load_kernel(efi_physical_address *kernel_address, efi_uintn *kernel_size) {
    efi_file *kernel_file = 0;
    void *file_buffer = 0;
    elf64_header *header;
    elf64_program_header *program_headers;
    efi_status status;
    efi_uintn read_size;
    efi_uintn pages;
    unsigned long long image_start = ~0ULL;
    unsigned long long image_end = 0;
    unsigned long long entry_point;
    unsigned short index;

    status = open_kernel_file(&kernel_file);
    if (status_failed(status) || kernel_file == 0) {
        return status;
    }

    status = g_boot_services->allocate_pool(EFI_LOADER_DATA, KERNEL_MAX_SIZE, &file_buffer);
    if (status_failed(status)) {
        kernel_file->close(kernel_file);
        print_text(g_error_memory);
        return status;
    }

    read_size = KERNEL_MAX_SIZE;
    status = kernel_file->read(kernel_file, &read_size, file_buffer);
    kernel_file->close(kernel_file);
    if (status_failed(status)) {
        g_boot_services->free_pool(file_buffer);
        print_text(g_error_read);
        return status;
    }

    header = (elf64_header *)file_buffer;
    {
        unsigned char digest[32];
        sha256_buffer((const unsigned char *)file_buffer, read_size, digest);
        if (!digest_matches(digest)) {
            g_boot_services->free_pool(file_buffer);
            print_text(g_error_digest);
            return 1;
        }
    }
    if (!elf_is_valid(header, read_size) ||
        header->program_header_offset +
        (unsigned long long)header->program_header_count * header->program_header_size > read_size) {
        g_boot_services->free_pool(file_buffer);
        print_text(g_error_elf_header);
        return 1;
    }

    program_headers = (elf64_program_header *)((unsigned char *)file_buffer + header->program_header_offset);
    for (index = 0; index < header->program_header_count; ++index) {
        elf64_program_header *program = &program_headers[index];
        if (program->type != 1) {
            continue;
        }
        if (!elf_program_is_valid(program, read_size)) {
            g_boot_services->free_pool(file_buffer);
            print_text(g_error_elf_segments);
            return 1;
        }
        if (program->virtual_address < image_start) {
            image_start = program->virtual_address;
        }
        if (program->virtual_address + program->memory_size > image_end) {
            image_end = program->virtual_address + program->memory_size;
        }
    }

    if (image_start == ~0ULL || image_end <= image_start ||
        image_start != KERNEL_LOAD_ADDRESS || image_end - image_start > KERNEL_MAX_SIZE) {
        g_boot_services->free_pool(file_buffer);
        print_text(g_error_elf_segments);
        return 1;
    }

    pages = (image_end - image_start + 4095) / 4096;
    *kernel_address = image_start;
    status = g_boot_services->allocate_pages(
        EFI_ALLOCATE_ADDRESS,
        EFI_LOADER_DATA,
        pages,
        kernel_address
    );
    if (status_failed(status)) {
        print_text(g_error_memory);
        g_boot_services->free_pool(file_buffer);
        return status;
    }

    memory_zero((void *)image_start, image_end - image_start);
    for (index = 0; index < header->program_header_count; ++index) {
        elf64_program_header *program = &program_headers[index];
        if (program->type == 1) {
            memory_copy((void *)program->virtual_address,
                (unsigned char *)file_buffer + program->offset, program->file_size);
        }
    }
    entry_point = header->entry;
    g_boot_services->free_pool(file_buffer);

    *kernel_size = image_end - image_start;
    g_boot_info.kernel_address = *kernel_address;
    g_boot_info.kernel_size = *kernel_size;
    if (entry_point < image_start || entry_point >= image_end) {
        print_text(g_error_elf);
        return 1;
    }
    return EFI_SUCCESS;
}

static efi_status exit_boot_services(void) {
    efi_uintn map_size = MEMORY_MAP_BUFFER_SIZE;
    efi_uintn map_key = 0;
    efi_uintn descriptor_size = 0;
    unsigned int descriptor_version = 0;
    void *memory_map = 0;
    efi_status status;
    unsigned int attempt;

    status = g_boot_services->allocate_pool(EFI_LOADER_DATA, map_size, &memory_map);
    if (status_failed(status)) {
        return status;
    }

    for (attempt = 0; attempt < 3; ++attempt) {
        map_size = MEMORY_MAP_BUFFER_SIZE;
        status = g_boot_services->get_memory_map(
            &map_size, memory_map, &map_key, &descriptor_size, &descriptor_version
        );
        if (status_failed(status)) {
            g_boot_services->free_pool(memory_map);
            return status;
        }
        status = g_boot_services->exit_boot_services(g_image_handle, map_key);
        if (!status_failed(status)) {
            g_boot_info.memory_map.address = (unsigned long long)memory_map;
            g_boot_info.memory_map.size = map_size;
            g_boot_info.memory_map.descriptor_size = descriptor_size;
            g_boot_info.memory_map.descriptor_version = descriptor_version;
            g_boot_info.flags |= MYOS_BOOT_INFO_FLAG_MEMORY_MAP;
            return status;
        }
        if (status != EFI_BUFFER_TOO_SMALL && status != EFI_INVALID_PARAMETER) {
            break;
        }
    }
    g_boot_services->free_pool(memory_map);
    return status;
}

efi_status EFIAPI efi_main(efi_handle image_handle, efi_system_table *system_table) {
    efi_physical_address kernel_address = 0;
    efi_uintn kernel_size = 0;
    efi_status status;

    g_image_handle = image_handle;
    g_system_table = system_table;
    g_boot_services = system_table->boot_services;
    boot_info_init();

    print_text(g_banner);
    print_text(g_loading);
    status = load_kernel(&kernel_address, &kernel_size);
    if (status_failed(status) || kernel_size == 0) {
        print_text(g_error);
        return status;
    }

    query_framebuffer();
    print_text(g_ready);
    status = exit_boot_services();
    if (status_failed(status)) {
        print_text(g_error);
        return status;
    }

    ((efi_kernel_entry)(unsigned long long)kernel_address)(&g_boot_info);
    return EFI_SUCCESS;
}
