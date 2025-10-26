/*
 * Memory Subsystem Module - Header
 * Memory model for BlackBox DPU
 */

#ifndef MEMORY_H
#define MEMORY_H

#include "blackbox_common.h"

/* ============================================================================
 * MEMORY SUBSYSTEM FUNCTIONS
 * ============================================================================ */

void memory_init(MemoryModel* mem);
void memory_cleanup(MemoryModel* mem);
uint8_t* memory_translate(MemoryModel* mem, uint32_t addr);
// Return how many contiguous bytes remain in the memory region starting at addr
uint32_t memory_get_region_remaining(MemoryModel* mem, uint32_t addr);

#endif // MEMORY_H
