/*
 * Zstandard Accelerator Module - Header
 * Hardware compression accelerator for BlackBox DPU
 */

#ifndef ZSTD_ACCELERATOR_H
#define ZSTD_ACCELERATOR_H

#include "blackbox_common.h"
#include "memory.h"
#include "event_queue.h"

/* ============================================================================
 * ZSTD ACCELERATOR FUNCTIONS
 * ============================================================================ */

uint32_t simple_compress(uint8_t* src, uint32_t src_len, uint8_t* dst, uint32_t level);
void zstd_start_compression(BlackBoxSoC* soc);

#endif // ZSTD_ACCELERATOR_H
