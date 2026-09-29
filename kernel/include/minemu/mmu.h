#ifndef MINEMU_MMU_H
#define MINEMU_MMU_H

#include <stdint.h>

#define MINEMU_PAGE_SIZE UINT32_C(4096)
#define MINEMU_PTE_VALID UINT32_C(1)
#define MINEMU_PTE_WRITABLE UINT32_C(2)
#define MINEMU_PTE_USER UINT32_C(4)
#define MINEMU_PTE_EXECUTABLE UINT32_C(8)
#define MINEMU_PTE_READABLE UINT32_C(16)
#define MINEMU_PTE_ACCESSED UINT32_C(32)
#define MINEMU_PTE_DIRTY UINT32_C(64)
#define MINEMU_PTE_SOFTWARE_MASK UINT32_C(0x00000f80)
#define MINEMU_PTE_PAGE_MASK UINT32_C(0xfffff000)
#define MINEMU_PDE_VALID UINT32_C(1)
#define MINEMU_PDE_PAGE_MASK UINT32_C(0xfffff000)
#define MINEMU_PDE_ALLOWED_MASK (MINEMU_PDE_VALID | MINEMU_PDE_PAGE_MASK)

#define MINEMU_FAULT_CAUSE_MASK UINT32_C(0xff)
#define MINEMU_FAULT_TRANSLATION UINT32_C(1)
#define MINEMU_FAULT_READ_PROTECTION UINT32_C(2)
#define MINEMU_FAULT_WRITE_PROTECTION UINT32_C(3)
#define MINEMU_FAULT_EXECUTE_PROTECTION UINT32_C(4)
#define MINEMU_FAULT_DEVICE_ACCESS UINT32_C(5)
#define MINEMU_FAULT_FROM_USER UINT32_C(0x00000100)
#define MINEMU_FAULT_IS_WRITE UINT32_C(0x00000200)
#define MINEMU_FAULT_IS_FETCH UINT32_C(0x00000400)

static inline void minemu_mmu_set_ttbr0(uint32_t address) {
    __asm__ volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(address) : "memory");
}

static inline void minemu_mmu_set_enabled(uint32_t enabled) {
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(enabled & 1U) : "memory");
}

static inline void minemu_mmu_tlbiall(void) {
    uint32_t ignored = 0;
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 0" : : "r"(ignored) : "memory");
}

static inline void minemu_mmu_set_vbar(uint32_t address) {
    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"(address) : "memory");
}

static inline uint32_t minemu_mmu_dfsr(void) {
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(value));
    return value;
}

static inline uint32_t minemu_mmu_dfar(void) {
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(value));
    return value;
}

#endif
