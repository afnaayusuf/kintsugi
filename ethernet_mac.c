/*
 * Ethernet MAC Module - Implementation
 * Ethernet MAC controller for BlackBox DPU
 * NOW WITH REAL NETWORK COMMUNICATION!
 */

#include "ethernet_mac.h"
#include "network_client.h"

/* ============================================================================
 * ETHERNET MAC MODEL
 * ============================================================================ */

void ethernet_transmit_data(BlackBoxSoC* soc) {
    EthernetMAC* eth = &soc->eth_mac;
    
    uint8_t* src = memory_translate(&soc->memory, eth->tx_buf_addr);
    if (!src) return;
    
    // REAL network transmission via HTTP POST to laptop
    bool success = network_send_data(src, eth->tx_buf_len);
    
    if (success) {
        eth->bytes_transmitted += eth->tx_buf_len;
        eth->packets_transmitted++;
        soc->noc_stats.ethernet_path_bytes += eth->tx_buf_len;
        
        if (soc->verbose) {
            printf("[%lu ns] Ethernet: ✓ Transmitted %u bytes to cloud (total: %lu bytes)\n",
                   soc->event_queue.current_time, eth->tx_buf_len, eth->bytes_transmitted);
        }
    } else {
        if (soc->verbose) {
            printf("[%lu ns] Ethernet: ✗ Failed to transmit %u bytes to cloud\n",
                   soc->event_queue.current_time, eth->tx_buf_len);
        }
    }
    
    // Also keep local backup file for redundancy
    FILE* cloud_log = fopen("cloud_log.bin", "ab");
    if (cloud_log) {
        fwrite(src, 1, eth->tx_buf_len, cloud_log);
        fclose(cloud_log);
    }
}
