/*
 * NVMe Controller Module - Header
 * NVMe storage controller for BlackBox DPU
 */

#ifndef NVME_CONTROLLER_H
#define NVME_CONTROLLER_H

#include "blackbox_common.h"
#include "memory.h"

/* ============================================================================
 * NVME CONTROLLER FUNCTIONS
 * ============================================================================ */

void nvme_write_data(BlackBoxSoC* soc);

#endif // NVME_CONTROLLER_H
