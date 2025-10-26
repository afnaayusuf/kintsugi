/*
 * Ethernet MAC Module - Implementation
 * Ethernet MAC controller for BlackBox DPU
 */

#include "ethernet_mac.h"

/* ============================================================================
 * ETHERNET MAC MODEL
 * ============================================================================ */

void ethernet_transmit_data(BlackBoxSoC* soc) {
    EthernetMAC* eth = &soc->eth_mac;
    
    uint8_t* src = memory_translate(&soc->memory, eth->tx_buf_addr);
    if (!src) return;
    
    // Simulate network transmission (write to log file)
    FILE* cloud_log = fopen("cloud_log.bin", "ab");
    if (cloud_log) {
        fwrite(src, 1, eth->tx_buf_len, cloud_log);
        fclose(cloud_log);
    }
    
    eth->bytes_transmitted += eth->tx_buf_len;
    eth->packets_transmitted++;
    
    soc->noc_stats.ethernet_path_bytes += eth->tx_buf_len;
    
    if (soc->verbose) {
        printf("[%lu ns] Ethernet: Transmitted %u bytes to cloud (total: %lu bytes)\n",
               soc->event_queue.current_time, eth->tx_buf_len, eth->bytes_transmitted);
    }
}
