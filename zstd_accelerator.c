/*
 * Zstandard Accelerator Module - Implementation
 * Hardware compression accelerator for BlackBox DPU
 */

#include "zstd_accelerator.h"

/* ============================================================================
 * SIMPLE ZSTANDARD COMPRESSION MODEL
 * (Simplified for simulation - real HW would use full Zstd algorithm)
 * ============================================================================ */

uint32_t simple_compress(uint8_t* src, uint32_t src_len, uint8_t* dst, uint32_t level) {
    // Simplified compression model: Run-Length Encoding for demonstration
    uint32_t dst_idx = 0;
    uint32_t i = 0;
    
    while (i < src_len) {
        uint8_t value = src[i];
        uint32_t count = 1;
        
        while (i + count < src_len && src[i + count] == value && count < 255) {
            count++;
        }
        
        if (count > 3 || value == 0xFF) {
            dst[dst_idx++] = 0xFF;  // Escape byte
            dst[dst_idx++] = value;
            dst[dst_idx++] = (uint8_t)count;
        } else {
            for (uint32_t j = 0; j < count; j++) {
                dst[dst_idx++] = value;
            }
        }
        
        i += count;
    }
    
    return dst_idx;
}

/* ============================================================================
 * ZSTANDARD ACCELERATOR MODEL
 * ============================================================================ */

typedef struct {
    BlackBoxSoC* soc;
} ZstdCompletionContext;

void zstd_completion_callback(void* context) {
    ZstdCompletionContext* ctx = (ZstdCompletionContext*)context;
    BlackBoxSoC* soc = ctx->soc;
    
    soc->zstd.status_reg &= ~ZSTD_STATUS_BUSY;
    soc->zstd.status_reg |= ZSTD_STATUS_DONE;
    soc->zstd.busy = false;
    
    if (soc->verbose) {
        printf("[%lu ns] Zstd: Compression complete. %u -> %u bytes (%.1f%% ratio)\n",
               soc->event_queue.current_time,
               soc->zstd.length,
               soc->zstd.compressed_size,
               (100.0 * soc->zstd.compressed_size) / soc->zstd.length);
    }
    
    free(ctx);
}

void zstd_start_compression(BlackBoxSoC* soc) {
    ZstdAccelerator* zstd = &soc->zstd;
    
    if (zstd->busy) return;
    
    uint8_t* src = memory_translate(&soc->memory, zstd->src_addr);
    uint8_t* dst = memory_translate(&soc->memory, zstd->dst_addr);
    
    if (!src || !dst) {
        zstd->status_reg |= ZSTD_STATUS_ERROR;
        return;
    }
    
    // Perform compression
    zstd->compressed_size = simple_compress(src, zstd->length, dst, zstd->level);
    
    zstd->busy = true;
    zstd->status_reg |= ZSTD_STATUS_BUSY;
    zstd->status_reg &= ~(ZSTD_STATUS_DONE | ZSTD_STATUS_ERROR);
    
    // Model compression latency: ~100ns per byte at level 1
    uint64_t latency = zstd->length * 100 * zstd->level;
    
    ZstdCompletionContext* ctx = (ZstdCompletionContext*)malloc(sizeof(ZstdCompletionContext));
    ctx->soc = soc;
    
    event_schedule(&soc->event_queue, latency, zstd_completion_callback, ctx);
    
    if (soc->verbose) {
        printf("[%lu ns] Zstd: Starting compression (src=0x%08X, dst=0x%08X, len=%u, level=%u)\n",
               soc->event_queue.current_time, zstd->src_addr, zstd->dst_addr, 
               zstd->length, zstd->level);
    }
}
