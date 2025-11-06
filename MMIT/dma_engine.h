/*
 * DMA Engine Module - Header
 * Multi-channel DMA controller with fan-out capability
 */

#ifndef DMA_ENGINE_H
#define DMA_ENGINE_H

#include "blackbox_common.h"
#include "memory.h"
#include "event_queue.h"

/* ============================================================================
 * DMA ENGINE FUNCTIONS
 * ============================================================================ */

void dma_start_transfer(BlackBoxSoC* soc, int channel);

#endif // DMA_ENGINE_H
