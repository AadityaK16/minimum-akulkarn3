#include "minemu/block.h"
#include "minemu/platform.h"

static uint32_t minemu_block_transfer(uint32_t command, uint32_t unit, uint32_t lba,
                                      uint32_t sector_count, uint32_t dma_paddr) {
    uint32_t cpsr;
    __asm__ volatile("mrs %0, cpsr\ncpsid i" : "=r"(cpsr) : : "memory");

    MINEMU_BLOCK->unit = unit;
    MINEMU_BLOCK->lba = lba;
    MINEMU_BLOCK->sector_count = sector_count;
    MINEMU_BLOCK->dma_paddr = dma_paddr;
    MINEMU_BLOCK->command = command;
    while ((MINEMU_BLOCK->status & MINEMU_BLOCK_STATUS_COMPLETE) == 0U) {
    }

    uint32_t status = MINEMU_BLOCK->status;
    uint32_t error = (status & MINEMU_BLOCK_STATUS_ERROR) != 0U
                         ? MINEMU_BLOCK->error
                         : MINEMU_BLOCK_ERROR_NONE;
    MINEMU_BLOCK->ack = MINEMU_BLOCK_ACK;
    if ((cpsr & UINT32_C(0x80)) == 0U) {
        __asm__ volatile("cpsie i" : : : "memory");
    }
    return error;
}

uint32_t minemu_block_read(uint32_t unit, uint32_t lba, uint32_t sector_count,
                           uint32_t dma_paddr) {
    return minemu_block_transfer(MINEMU_BLOCK_COMMAND_READ, unit, lba, sector_count, dma_paddr);
}

uint32_t minemu_block_write(uint32_t unit, uint32_t lba, uint32_t sector_count,
                            uint32_t dma_paddr) {
    return minemu_block_transfer(MINEMU_BLOCK_COMMAND_WRITE, unit, lba, sector_count, dma_paddr);
}
