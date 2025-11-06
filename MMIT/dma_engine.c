/*
 * DMA Engine Module - Implementation
 * Multi-channel DMA controller with fan-out capability
 */

#include "dma_engine.h"

/* ============================================================================
 * DMA ENGINE MODEL
 * ============================================================================ */

typedef struct {
    BlackBoxSoC* soc;
    int channel;
} DMACompletionContext;

void dma_completion_callback(void* context) {
    DMACompletionContext* ctx = (DMACompletionContext*)context;
    BlackBoxSoC* soc = ctx->soc;
    DMAChannel* ch = &soc->dma.channels[ctx->channel];
    
    ch->status_reg &= ~DMA_STATUS_BUSY;
    ch->status_reg |= DMA_STATUS_DONE;
    ch->busy = false;
    
    if (soc->verbose) {
        printf("[%lu ns] DMA Ch%d: Transfer complete (%u bytes)\n",
               soc->event_queue.current_time, ctx->channel, ch->length);
    }
    
    free(ctx);
}

void dma_start_transfer(BlackBoxSoC* soc, int channel) {
    DMAChannel* ch = &soc->dma.channels[channel];
    
    if (ch->busy) return;
    
    uint8_t* src = memory_translate(&soc->memory, ch->src_addr);
    uint8_t* dst = memory_translate(&soc->memory, ch->dst_addr);

    if (!src || !dst) {
        if (soc->verbose) {
            printf("[%lu ns] DMA Ch%d: Invalid source or destination address (src=0x%08X dst=0x%08X)\n",
                   soc->event_queue.current_time, channel, ch->src_addr, ch->dst_addr);
        }
        return;
    }

    // Check region remaining to avoid out-of-bounds copies
    uint32_t src_rem = memory_get_region_remaining(&soc->memory, ch->src_addr);
    uint32_t dst_rem = memory_get_region_remaining(&soc->memory, ch->dst_addr);
    uint32_t to_copy = ch->length;
    if (to_copy > src_rem || to_copy > dst_rem) {
        uint32_t allowed = src_rem < dst_rem ? src_rem : dst_rem;
        if (allowed == 0) {
            if (soc->verbose) {
                printf("[%lu ns] DMA Ch%d: No space to transfer (allowed=0)\n",
                       soc->event_queue.current_time, channel);
            }
            return;
        }
        if (soc->verbose) {
            printf("[%lu ns] DMA Ch%d: Truncating transfer from %u to %u bytes to avoid OOB\n",
                   soc->event_queue.current_time, channel, to_copy, allowed);
        }
        to_copy = allowed;
    }

    // Perform DMA transfer (possibly truncated)
    memcpy(dst, src, to_copy);

    // If fan-out is enabled (for dual-path logging)
    if (ch->fanout_enabled && ch->fanout_dst_addr != 0) {
        uint8_t* fanout_dst = memory_translate(&soc->memory, ch->fanout_dst_addr);
        if (fanout_dst) {
            uint32_t fanout_rem = memory_get_region_remaining(&soc->memory, ch->fanout_dst_addr);
            uint32_t fanout_copy = to_copy;
            if (fanout_copy > fanout_rem) {
                fanout_copy = fanout_rem;
                if (soc->verbose) {
                    printf("[%lu ns] DMA Ch%d: Fan-out truncated to %u bytes\n",
                           soc->event_queue.current_time, channel, fanout_copy);
                }
            }
            if (fanout_copy > 0) memcpy(fanout_dst, src, fanout_copy);

            if (soc->verbose) {
                printf("[%lu ns] DMA Ch%d: Fan-out copy to 0x%08X (len=%u)\n",
                       soc->event_queue.current_time, channel, ch->fanout_dst_addr, fanout_copy);
            }
        }
    }

    ch->busy = true;
    ch->status_reg |= DMA_STATUS_BUSY;
    ch->status_reg &= ~DMA_STATUS_DONE;

    // Model DMA latency: ~10ns per byte (use actual copied bytes)
    uint64_t latency = (uint64_t)to_copy * 10ULL;

    DMACompletionContext* ctx = (DMACompletionContext*)malloc(sizeof(DMACompletionContext));
    ctx->soc = soc;
    ctx->channel = channel;

    event_schedule(&soc->event_queue, latency, dma_completion_callback, ctx);

    soc->noc_stats.total_transactions++;
    soc->noc_stats.memory_accesses += to_copy;

    if (soc->verbose) {
        printf("[%lu ns] DMA Ch%d: Starting transfer (src=0x%08X, dst=0x%08X, len=%u)\n",
               soc->event_queue.current_time, channel, ch->src_addr,
               ch->dst_addr, to_copy);
    }
}
