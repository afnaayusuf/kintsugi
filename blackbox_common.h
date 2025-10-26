/*
 * BlackBox DPU - Common Definitions Header
 * Contains memory map, register definitions, and data structures
 */

#ifndef BLACKBOX_COMMON_H
#define BLACKBOX_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* ============================================================================
 * MEMORY MAP DEFINITIONS (From Section 4.1)
 * ============================================================================ */
#define BOOT_ROM_BASE           0x00000000
#define BOOT_ROM_SIZE           (128 * 1024)

#define SBM_BASE                0x04000000  // Shared Buffer Memory
#define SBM_SIZE                (4 * 1024 * 1024)

#define APU_L2_CACHE_BASE       0x08000000
#define APU_L2_CACHE_SIZE       (1 * 1024 * 1024)

#define RPU_TCM_BASE            0x08100000
#define RPU_TCM_SIZE            (64 * 1024)

#define DRAM_BASE               0x80000000
#define DRAM_SIZE               (512 * 1024 * 1024)  // 512MB for simulation

#define ZSTD_REGS_BASE          0xFF800000
#define DMA_REGS_BASE           0xFF810000
#define PCIE_REGS_BASE          0xFF900000
#define ETH_MAC_REGS_BASE       0xFFA00000
#define PERIPH_REGS_BASE        0xFFF00000

/* ============================================================================
 * HARDWARE REGISTER DEFINITIONS
 * ============================================================================ */

// Zstandard Accelerator Registers
#define ZSTD_CTRL_REG           (ZSTD_REGS_BASE + 0x00)
#define ZSTD_STATUS_REG         (ZSTD_REGS_BASE + 0x04)
#define ZSTD_SRC_ADDR_REG       (ZSTD_REGS_BASE + 0x08)
#define ZSTD_DST_ADDR_REG       (ZSTD_REGS_BASE + 0x0C)
#define ZSTD_LENGTH_REG         (ZSTD_REGS_BASE + 0x10)
#define ZSTD_COMP_SIZE_REG      (ZSTD_REGS_BASE + 0x14)
#define ZSTD_LEVEL_REG          (ZSTD_REGS_BASE + 0x18)

#define ZSTD_CTRL_START         (1 << 0)
#define ZSTD_CTRL_RESET         (1 << 1)
#define ZSTD_STATUS_BUSY        (1 << 0)
#define ZSTD_STATUS_DONE        (1 << 1)
#define ZSTD_STATUS_ERROR       (1 << 2)

// DMA Controller Registers (4 channels)
#define DMA_CH0_CTRL            (DMA_REGS_BASE + 0x000)
#define DMA_CH0_STATUS          (DMA_REGS_BASE + 0x004)
#define DMA_CH0_SRC_ADDR        (DMA_REGS_BASE + 0x008)
#define DMA_CH0_DST_ADDR        (DMA_REGS_BASE + 0x00C)
#define DMA_CH0_LENGTH          (DMA_REGS_BASE + 0x010)

#define DMA_CH1_CTRL            (DMA_REGS_BASE + 0x020)
#define DMA_CH2_CTRL            (DMA_REGS_BASE + 0x040)
#define DMA_CH3_CTRL            (DMA_REGS_BASE + 0x060)

#define DMA_CTRL_START          (1 << 0)
#define DMA_CTRL_FANOUT_EN      (1 << 4)  // Enable fan-out/tee mode
#define DMA_STATUS_BUSY         (1 << 0)
#define DMA_STATUS_DONE         (1 << 1)

// Ethernet MAC Registers
#define ETH_CTRL_REG            (ETH_MAC_REGS_BASE + 0x00)
#define ETH_STATUS_REG          (ETH_MAC_REGS_BASE + 0x04)
#define ETH_TX_BUF_ADDR         (ETH_MAC_REGS_BASE + 0x08)
#define ETH_TX_BUF_LEN          (ETH_MAC_REGS_BASE + 0x0C)

// NVMe/PCIe Registers
#define NVME_CTRL_REG           (PCIE_REGS_BASE + 0x00)
#define NVME_STATUS_REG         (PCIE_REGS_BASE + 0x04)
#define NVME_WRITE_BUF_ADDR     (PCIE_REGS_BASE + 0x08)
#define NVME_WRITE_BUF_LEN      (PCIE_REGS_BASE + 0x0C)

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */
typedef struct BlackBoxSoC BlackBoxSoC;
typedef struct Event Event;
typedef struct EventQueue EventQueue;
typedef struct MemoryModel MemoryModel;
typedef struct ZstdAccelerator ZstdAccelerator;
typedef struct DMAChannel DMAChannel;
typedef struct DMAEngine DMAEngine;
typedef struct EthernetMAC EthernetMAC;
typedef struct NVMeController NVMeController;
typedef struct NoCStatistics NoCStatistics;
typedef struct SensorChannel SensorChannel;
typedef struct APUCore APUCore;
typedef struct RPUCore RPUCore;
typedef struct LogIndex LogIndex;
typedef struct EventMarker EventMarker;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

// Event-driven simulation event
struct Event {
    uint64_t timestamp;
    void (*callback)(void* context);
    void* context;
    struct Event* next;
};

// Event queue
struct EventQueue {
    Event* head;
    uint64_t current_time;
};

// Memory model - represents all addressable memory
struct MemoryModel {
    uint8_t* boot_rom;
    uint8_t* sbm;           // Shared Buffer Memory
    uint8_t* apu_l2_cache;
    uint8_t* rpu_tcm;
    uint8_t* dram;
};

// Zstandard hardware accelerator model
struct ZstdAccelerator {
    uint32_t ctrl_reg;
    uint32_t status_reg;
    uint32_t src_addr;
    uint32_t dst_addr;
    uint32_t length;
    uint32_t compressed_size;
    uint32_t level;
    
    // Internal state
    bool busy;
    uint64_t completion_time;
};

// DMA Channel descriptor
struct DMAChannel {
    uint32_t ctrl_reg;
    uint32_t status_reg;
    uint32_t src_addr;
    uint32_t dst_addr;
    uint32_t length;
    
    bool busy;
    bool fanout_enabled;
    uint32_t fanout_dst_addr;  // Secondary destination for channel 3
};

// DMA Engine (4 channels)
struct DMAEngine {
    DMAChannel channels[4];
};

// Ethernet MAC model
struct EthernetMAC {
    uint32_t ctrl_reg;
    uint32_t status_reg;
    uint32_t tx_buf_addr;
    uint32_t tx_buf_len;
    
    // Statistics
    uint64_t bytes_transmitted;
    uint32_t packets_transmitted;
};

// NVMe controller model
struct NVMeController {
    uint32_t ctrl_reg;
    uint32_t status_reg;
    uint32_t write_buf_addr;
    uint32_t write_buf_len;
    
    // Statistics
    uint64_t bytes_written;
    uint32_t writes_completed;
    
    // Virtual storage (file-backed)
    FILE* storage_file;
};

// Network-on-Chip interconnect statistics
struct NoCStatistics {
    uint64_t total_transactions;
    uint64_t nvme_path_bytes;
    uint64_t ethernet_path_bytes;
    uint64_t memory_accesses;
};

/* ============================================================================
 * SENSOR CHANNEL MANAGEMENT (DAW Analogy - "Tracks")
 * ============================================================================ */

// Channel state enumeration
typedef enum {
    CHANNEL_OFF = 0,
    CHANNEL_ON = 1,
    CHANNEL_RECORDING = 2,
    CHANNEL_FROZEN = 3
} ChannelState;

// Sensor channel descriptor (like a DAW track)
struct SensorChannel {
    uint32_t channel_id;
    char name[32];
    ChannelState state;
    
    // Health monitoring
    float health_score;          // 0.0 to 1.0
    uint32_t stagnation_counter;
    float last_value;
    
    // Configuration
    uint32_t sample_rate;        // Hz
    uint8_t bit_depth;           // 16, 24, 32 bits
    bool adaptive_precision;
    
    // Statistics
    uint64_t samples_recorded;
    uint64_t freeze_start_time;
};

// Event marker (DAW-style bookmarks)
struct EventMarker {
    uint64_t timestamp;
    char label[64];
    char metadata[256];  // JSON payload
    struct EventMarker* next;
};

// Log index entry for timestamp queries
struct LogIndex {
    uint64_t timestamp_start;
    uint64_t timestamp_end;
    uint64_t file_offset;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    struct LogIndex* next;
};

/* ============================================================================
 * APU & RPU CORES (Heterogeneous Processing)
 * ============================================================================ */

// APU - Application Processing Unit
struct APUCore {
    bool initialized;
    bool local_auth_token_valid;
    uint64_t auth_token_expiry;
    
    // Configuration management
    bool allow_remote_config;
    bool device_record_enable;
    
    // Query processing
    uint32_t pending_queries;
};

// RPU - Real-time Processing Unit  
struct RPUCore {
    bool initialized;
    bool running;
    
    // Real-time processing plugins
    bool filter_enabled;
    bool normalize_enabled;
    bool compress_dynamics;
    
    // Sensor health monitoring
    uint32_t monitored_channels;
    float health_threshold;
};

// Cloud sync state
typedef struct {
    bool connected;
    uint64_t last_sync_timestamp;  // Watermark
    uint64_t backlog_bytes;
    bool redemption_in_progress;
} CloudSyncState;

// Complete SoC model
struct BlackBoxSoC {
    MemoryModel memory;
    ZstdAccelerator zstd;
    DMAEngine dma;
    EthernetMAC eth_mac;
    NVMeController nvme;
    NoCStatistics noc_stats;
    EventQueue event_queue;
    
    // Heterogeneous cores
    APUCore apu;
    RPUCore rpu;
    
    // Sensor management
    SensorChannel* channels;
    uint32_t num_channels;
    
    // Event markers & indexing
    EventMarker* markers;
    LogIndex* log_index;
    
    // Cloud sync
    CloudSyncState cloud_sync;
    
    // Configuration
    bool verbose;
};

#endif // BLACKBOX_COMMON_H
