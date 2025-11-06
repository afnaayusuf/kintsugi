/*
 * Bus Interconnect Module - Header
 * Network-on-Chip and bus transaction model
 */

#ifndef BUS_INTERCONNECT_H
#define BUS_INTERCONNECT_H

#include "blackbox_common.h"
#include "memory.h"
#include "zstd_accelerator.h"
#include "dma_engine.h"
#include "nvme_controller.h"
#include "ethernet_mac.h"

/* ============================================================================
 * BUS INTERCONNECT FUNCTIONS
 * ============================================================================ */

uint32_t bus_read(BlackBoxSoC* soc, uint32_t addr);
void bus_write(BlackBoxSoC* soc, uint32_t addr, uint32_t data);

#endif // BUS_INTERCONNECT_H
