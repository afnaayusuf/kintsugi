/*
 * Ethernet MAC Module - Header
 * Ethernet MAC controller for BlackBox DPU
 */

#ifndef ETHERNET_MAC_H
#define ETHERNET_MAC_H

#include "blackbox_common.h"
#include "memory.h"

/* ============================================================================
 * ETHERNET MAC FUNCTIONS
 * ============================================================================ */

void ethernet_transmit_data(BlackBoxSoC* soc);

#endif // ETHERNET_MAC_H
