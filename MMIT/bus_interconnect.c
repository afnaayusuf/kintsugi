/*
 * Bus Interconnect Module - Implementation
 * Network-on-Chip and bus transaction model
 */

#include "bus_interconnect.h"

/* ============================================================================
 * INTERCONNECT / BUS TRANSACTION MODEL
 * ============================================================================ */

uint32_t bus_read(BlackBoxSoC* soc, uint32_t addr) {
    // Handle register reads
    if (addr >= ZSTD_REGS_BASE && addr < ZSTD_REGS_BASE + 0x1000) {
        switch (addr) {
            case ZSTD_STATUS_REG: return soc->zstd.status_reg;
            case ZSTD_COMP_SIZE_REG: return soc->zstd.compressed_size;
            default: return 0;
        }
    } else if (addr >= DMA_REGS_BASE && addr < DMA_REGS_BASE + 0x1000) {
        int channel = (addr - DMA_REGS_BASE) / 0x20;
        int offset = (addr - DMA_REGS_BASE) % 0x20;
        if (channel < 4 && offset == 0x04) {
            return soc->dma.channels[channel].status_reg;
        }
    }
    
    // Memory access
    uint8_t* ptr = memory_translate(&soc->memory, addr);
    if (ptr) {
        soc->noc_stats.memory_accesses += 4;
        return *(uint32_t*)ptr;
    }
    
    return 0;
}

void bus_write(BlackBoxSoC* soc, uint32_t addr, uint32_t data) {
    // Handle register writes with side effects
    if (addr >= ZSTD_REGS_BASE && addr < ZSTD_REGS_BASE + 0x1000) {
        switch (addr) {
            case ZSTD_CTRL_REG:
                soc->zstd.ctrl_reg = data;
                if (data & ZSTD_CTRL_START) {
                    zstd_start_compression(soc);
                }
                break;
            case ZSTD_SRC_ADDR_REG: soc->zstd.src_addr = data; break;
            case ZSTD_DST_ADDR_REG: soc->zstd.dst_addr = data; break;
            case ZSTD_LENGTH_REG: soc->zstd.length = data; break;
            case ZSTD_LEVEL_REG: soc->zstd.level = data; break;
        }
    } else if (addr >= DMA_REGS_BASE && addr < DMA_REGS_BASE + 0x1000) {
        int channel = (addr - DMA_REGS_BASE) / 0x20;
        int offset = (addr - DMA_REGS_BASE) % 0x20;
        
        if (channel < 4) {
            DMAChannel* ch = &soc->dma.channels[channel];
            switch (offset) {
                case 0x00:  // CTRL
                    ch->ctrl_reg = data;
                    if (data & DMA_CTRL_FANOUT_EN) {
                        ch->fanout_enabled = true;
                    }
                    if (data & DMA_CTRL_START) {
                        dma_start_transfer(soc, channel);
                    }
                    break;
                case 0x08: ch->src_addr = data; break;
                case 0x0C: ch->dst_addr = data; break;
                case 0x10: ch->length = data; break;
            }
        }
    } else if (addr >= ETH_MAC_REGS_BASE && addr < ETH_MAC_REGS_BASE + 0x10000) {
        switch (addr) {
            case ETH_TX_BUF_ADDR: soc->eth_mac.tx_buf_addr = data; break;
            case ETH_TX_BUF_LEN: soc->eth_mac.tx_buf_len = data; break;
            case ETH_CTRL_REG:
                soc->eth_mac.ctrl_reg = data;
                if (data & 0x01) {  // TX Start
                    ethernet_transmit_data(soc);
                }
                break;
        }
    } else if (addr >= PCIE_REGS_BASE && addr < PCIE_REGS_BASE + 0x100000) {
        switch (addr) {
            case NVME_WRITE_BUF_ADDR: soc->nvme.write_buf_addr = data; break;
            case NVME_WRITE_BUF_LEN: soc->nvme.write_buf_len = data; break;
            case NVME_CTRL_REG:
                soc->nvme.ctrl_reg = data;
                if (data & 0x01) {  // Write command
                    nvme_write_data(soc);
                }
                break;
        }
    } else {
        // Memory write
        uint8_t* ptr = memory_translate(&soc->memory, addr);
        if (ptr) {
            *(uint32_t*)ptr = data;
            soc->noc_stats.memory_accesses += 4;
        }
    }
}
