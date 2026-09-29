#ifndef MINEMU_BLOCK_H
#define MINEMU_BLOCK_H

#include <stdint.h>

/*
 * Perform one synchronous transfer through the shared block controller.
 * sector_count is a nonzero count of 512-byte sectors. dma_paddr must be a
 * 512-byte-aligned physical RAM address covering the complete transfer.
 * Calls through this interface are serialized on the single CPU and return a
 * MINEMU_BLOCK_ERROR_* value after acknowledging completion.
 */
uint32_t minemu_block_read(uint32_t unit, uint32_t lba, uint32_t sector_count,
                           uint32_t dma_paddr);
uint32_t minemu_block_write(uint32_t unit, uint32_t lba, uint32_t sector_count,
                            uint32_t dma_paddr);

#endif
