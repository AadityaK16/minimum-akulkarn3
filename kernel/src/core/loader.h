#ifndef LOADER_H
#define LOADER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "minemu/boot.h"

#define USER_STACK_TOP UINT32_C(0x00800000)

bool map_system_rom(const struct minemu_boot_info *boot_info);

const struct minemu_module_record *find_module(const struct minemu_boot_info *boot_info, const char *name, size_t name_length);

bool load_user_module(const struct minemu_boot_info *boot_info, const struct minemu_module_record *module);

bool user_range_ok(uint32_t vaddr, uint32_t length, uint32_t required_flags);

#endif