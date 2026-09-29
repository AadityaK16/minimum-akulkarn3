#ifndef MINEMU_BOOT_H
#define MINEMU_BOOT_H

#include <stdint.h>

#define MINEMU_BOOT_INFO_MAGIC UINT32_C(0x4d424f4f)
#define MINEMU_ABI_VERSION UINT16_C(1)
#define MINEMU_BOOT_INFO_VADDR UINT32_C(0xc0007000)
#define MINEMU_SEGMENT_READABLE UINT32_C(1)
#define MINEMU_SEGMENT_WRITABLE UINT32_C(2)
#define MINEMU_SEGMENT_EXECUTABLE UINT32_C(4)

struct minemu_boot_info {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint32_t system_rom_base;
    uint32_t image_size;
    uint32_t module_table_offset;
    uint32_t module_count;
    uint32_t direct_map_vaddr;
    uint32_t direct_map_paddr;
    uint32_t direct_map_size;
    uint32_t flags;
    uint32_t reserved[6];
};

struct minemu_module_record {
    uint32_t name_offset;
    uint32_t name_length;
    uint32_t segment_table_offset;
    uint32_t segment_count;
    uint32_t entry_vaddr;
    uint32_t flags;
    uint32_t reserved[2];
};

struct minemu_module_segment {
    uint32_t data_offset;
    uint32_t virtual_address;
    uint32_t file_size;
    uint32_t memory_size;
    uint32_t flags;
    uint32_t reserved[3];
};

_Static_assert(sizeof(struct minemu_boot_info) == 64, "boot-info size");
_Static_assert(sizeof(struct minemu_module_record) == 32, "module-record size");
_Static_assert(sizeof(struct minemu_module_segment) == 32, "module-segment size");

#endif
