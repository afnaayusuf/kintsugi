/*
 * SoC Core Module - Implementation
 * High-level SoC orchestration and data flow
 */

#include "soc_core.h"
#ifdef __unix__
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#endif

/* ============================================================================
 * APU & RPU INITIALIZATION
 * ============================================================================ */

void apu_init(APUCore* apu) {
    apu->initialized = true;
    apu->local_auth_token_valid = false;
    apu->auth_token_expiry = 0;
    apu->allow_remote_config = false;  // Local-first security
    apu->device_record_enable = true;
    apu->pending_queries = 0;
}

void rpu_init(RPUCore* rpu) {
    rpu->initialized = true;
    rpu->running = true;
    rpu->filter_enabled = false;
    rpu->normalize_enabled = false;
    rpu->compress_dynamics = false;
    rpu->monitored_channels = 0;
    rpu->health_threshold = 0.3f;  // Flag below 30%
}

bool apu_validate_config_request(APUCore* apu, bool is_local) {
    // Local-first security model
    if (is_local) {
        return true;  // Always allow local configuration
    }
    
    // Remote requests require valid auth token
    if (!apu->allow_remote_config) {
        return false;
    }
    
    return apu->local_auth_token_valid;
}

void rpu_monitor_sensor_health(RPUCore* rpu, SensorChannel* channel, float value) {
    // Multi-factor sensor health analysis
    bool stagnant = (value == channel->last_value);
    bool out_of_bounds = (value < -1000.0f || value > 1000.0f);
    
    if (stagnant) {
        channel->stagnation_counter++;
    } else {
        channel->stagnation_counter = 0;
    }
    
    // Calculate health score
    float score = 1.0f;
    if (channel->stagnation_counter > 100) score -= 0.4f;
    if (out_of_bounds) score -= 0.5f;
    
    channel->health_score = score;
    channel->last_value = value;
    
    // Flag unreliable sensor
    if (score < rpu->health_threshold) {
        channel->state = CHANNEL_FROZEN;
    }
}

/* ============================================================================
 * SENSOR CHANNEL MANAGEMENT
 * ============================================================================ */

void sensor_channel_init(SensorChannel* channel, uint32_t id, const char* name) {
    channel->channel_id = id;
    strncpy(channel->name, name, sizeof(channel->name) - 1);
    channel->state = CHANNEL_ON;
    channel->health_score = 1.0f;
    channel->stagnation_counter = 0;
    channel->last_value = 0.0f;
    channel->sample_rate = 1000;  // 1kHz default
    channel->bit_depth = 32;
    channel->adaptive_precision = false;
    channel->samples_recorded = 0;
    channel->freeze_start_time = 0;
}

void sensor_channel_set_state(SensorChannel* channel, ChannelState state, uint64_t timestamp) {
    if (state == CHANNEL_FROZEN) {
        channel->freeze_start_time = timestamp;
    }
    channel->state = state;
}

float sensor_channel_get_health(SensorChannel* channel) {
    return channel->health_score;
}

/* ============================================================================
 * DYNAMIC SENSOR MANAGEMENT & LIVE DISPLAY
 * ============================================================================
 */

// Add a sensor channel dynamically. New channels are appended after existing ones.
void sensor_channel_add(BlackBoxSoC* soc, const char* name) {
    uint32_t old_count = soc->num_channels;
    uint32_t new_count = old_count + 1;
    SensorChannel* new_buf = (SensorChannel*)realloc(soc->channels, new_count * sizeof(SensorChannel));
    if (!new_buf) return; // allocation failed
    soc->channels = new_buf;
    soc->num_channels = new_count;
    sensor_channel_init(&soc->channels[new_count - 1], new_count - 1, name);
}

// Ensure minimum baseline channels exist (used at init)
void sensor_ensure_minimum(BlackBoxSoC* soc, uint32_t min_count) {
    if (soc->num_channels >= min_count) return;
    SensorChannel* new_buf = (SensorChannel*)realloc(soc->channels, min_count * sizeof(SensorChannel));
    if (!new_buf) return;
    soc->channels = new_buf;
    for (uint32_t i = soc->num_channels; i < min_count; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "Unused_%u", i);
        sensor_channel_init(&soc->channels[i], i, name);
        sensor_channel_set_state(&soc->channels[i], CHANNEL_OFF, 0);
    }
    soc->num_channels = min_count;
}

// Live in-place channel display using ANSI escape sequences.
// This prints a fixed block of lines and overwrites them on subsequent calls.
static int g_last_display_lines = 0;
void soc_display_channels(BlackBoxSoC* soc) {
    // Count lines: one header + one per channel
    int lines = 2 + (int)soc->num_channels; // header + blank + channels

    // If we've printed before, move cursor up to overwrite the block
    if (g_last_display_lines > 0) {
        // Move cursor up by g_last_display_lines
        printf("\x1b[%dA", g_last_display_lines);
    }

    // Print header
    printf("Sensor Channels:\n");
    printf("(Press Ctrl-C to interrupt)\n");

    // Print each channel line
    for (uint32_t i = 0; i < soc->num_channels; ++i) {
        SensorChannel* ch = &soc->channels[i];
        const char* state_str = (ch->state == CHANNEL_ON) ? "ON" :
                                (ch->state == CHANNEL_FROZEN) ? "FROZEN" :
                                (ch->state == CHANNEL_RECORDING) ? "REC" : "OFF";
        // Clear line and print
        printf("\r\x1b[K  CH%u [%-16s]: %4s  (Health: %6.1f%%)\n",
               ch->channel_id, ch->name, state_str, ch->health_score * 100.0f);
    }

    fflush(stdout);
    g_last_display_lines = lines;
}

// Simple POSIX non-blocking input poll. Reads a line if available and
// dispatches to soc_handle_command(). On Windows this function does nothing.
void soc_poll_input(BlackBoxSoC* soc) {
#ifdef __unix__
    struct timeval tv = {0, 0};
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    int rv = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    if (rv > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
        char buf[256];
        if (fgets(buf, sizeof(buf), stdin)) {
            // strip newline
            buf[strcspn(buf, "\r\n")] = 0;
            soc_handle_command(soc, buf);
        }
    }
#else
    (void)soc;
#endif
}

// Default command handler — records the command as an event marker and echoes it.
void soc_handle_command(BlackBoxSoC* soc, const char* cmd) {
    if (!cmd || strlen(cmd) == 0) return;
    printf("\n[CMD] %s\n", cmd);
    // Simple command parsing: add <name>, list, help
    if (strncmp(cmd, "add ", 4) == 0) {
        const char* name = cmd + 4;
        sensor_channel_add(soc, name);
        printf("Added sensor '%s' as CH%u\n", name, soc->num_channels - 1);
        soc_display_channels(soc);
    } else if (strcmp(cmd, "list") == 0) {
        printf("Channels (%u):\n", soc->num_channels);
        for (uint32_t i = 0; i < soc->num_channels; ++i) {
            SensorChannel* ch = &soc->channels[i];
            printf("  CH%u: %s (%s)\n", ch->channel_id, ch->name,
                   ch->state == CHANNEL_ON ? "ON" : "OFF");
        }
    } else if (strcmp(cmd, "help") == 0) {
        printf("Commands:\n  add <name>   - add a sensor channel\n  list         - list channels\n  help         - show this help\n");
    }
    // Add an event marker so command history is visible in logs
    add_event_marker(soc, "CMD", cmd);
}

/* ============================================================================
 * EVENT MARKERS & INDEXING
 * ============================================================================ */

void add_event_marker(BlackBoxSoC* soc, const char* label, const char* metadata) {
    EventMarker* marker = (EventMarker*)malloc(sizeof(EventMarker));
    marker->timestamp = soc->event_queue.current_time;
    strncpy(marker->label, label, sizeof(marker->label) - 1);
    strncpy(marker->metadata, metadata, sizeof(marker->metadata) - 1);
    marker->next = soc->markers;
    soc->markers = marker;
    
    if (soc->verbose) {
        printf("[%lu ns] EVENT MARKER: %s - %s\n", 
               marker->timestamp, label, metadata);
    }
}

void add_log_index_entry(BlackBoxSoC* soc, uint64_t ts_start, uint64_t ts_end, 
                         uint64_t offset, uint32_t comp_size) {
    LogIndex* entry = (LogIndex*)malloc(sizeof(LogIndex));
    entry->timestamp_start = ts_start;
    entry->timestamp_end = ts_end;
    entry->file_offset = offset;
    entry->compressed_size = comp_size;
    entry->next = soc->log_index;
    soc->log_index = entry;
}

LogIndex* query_log_by_timestamp(BlackBoxSoC* soc, uint64_t timestamp) {
    LogIndex* current = soc->log_index;
    while (current) {
        if (timestamp >= current->timestamp_start && 
            timestamp <= current->timestamp_end) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* ============================================================================
 * CLOUD SYNC & NETWORK BACKLOG
 * ============================================================================ */

void cloud_sync_init(CloudSyncState* sync) {
    sync->connected = false;
    sync->last_sync_timestamp = 0;
    sync->backlog_bytes = 0;
    sync->redemption_in_progress = false;
}

void cloud_sync_update_watermark(CloudSyncState* sync, uint64_t timestamp) {
    sync->last_sync_timestamp = timestamp;
}

void cloud_sync_handle_reconnect(BlackBoxSoC* soc) {
    if (!soc->cloud_sync.connected) {
        printf("[%lu ns] Cloud reconnected - starting backlog redemption\n",
               soc->event_queue.current_time);
        soc->cloud_sync.connected = true;
        soc->cloud_sync.redemption_in_progress = true;
        add_event_marker(soc, "Backlog-Start", 
                        "{\"event\": \"cloud_reconnect\"}");
    }
}

bool apu_request_controller_permission(APUCore* apu) {
    // Simulate asking for permission from the pilot/controller
    // In a real system, this would involve a more complex interaction
    printf("APU: Requesting controller permission for data transfer...\n");
    printf("Controller: Permission GRANTED.\n");
    return true;
}

bool read_marker_key(char* buffer, size_t len) {
    FILE* f = fopen("marker.key", "r");
    if (!f) {
        return false;
    }
    fgets(buffer, len, f);
    fclose(f);
    // Remove trailing newline
    buffer[strcspn(buffer, "\r\n")] = 0;
    return true;
}

void handle_cloud_transfer_request(BlackBoxSoC* soc, uint64_t timestamp, const char* key) {
    printf("\n[%lu ns] === Received Cloud Transfer Request for timestamp %lu ===\n", 
           soc->event_queue.current_time, timestamp);

    // 1. Validate marker key
    char marker_key[128];
    if (!read_marker_key(marker_key, sizeof(marker_key))) {
        printf("Transfer FAILED: Marker key file not found.\n");
        return;
    }

    if (strcmp(key, marker_key) != 0) {
        printf("Transfer FAILED: Invalid marker key provided.\n");
        return;
    }
    printf("Marker key validation PASSED.\n");

    // 2. Request controller permission
    if (!apu_request_controller_permission(&soc->apu)) {
        printf("Transfer FAILED: Controller denied permission.\n");
        return;
    }

    // 3. Find data block in NVMe log
    LogIndex* log_entry = query_log_by_timestamp(soc, timestamp);
    if (!log_entry) {
        printf("Transfer FAILED: No data log found for the given timestamp.\n");
        return;
    }
    printf("Found data block at offset %lu (size: %u bytes).\n", 
           log_entry->file_offset, log_entry->compressed_size);

    // 4. Read data from NVMe storage
    uint8_t* data_to_transfer = (uint8_t*)malloc(log_entry->compressed_size);
    fseek(soc->nvme.storage_file, log_entry->file_offset, SEEK_SET);
    fread(data_to_transfer, 1, log_entry->compressed_size, soc->nvme.storage_file);

    // 5. Copy data to Ethernet buffer
    uint32_t eth_buf_addr = SBM_BASE + (3 * 1024 * 1024); // Use the dedicated ETH buffer
    uint8_t* eth_buf = memory_translate(&soc->memory, eth_buf_addr);
    memcpy(eth_buf, data_to_transfer, log_entry->compressed_size);
    free(data_to_transfer);

    // 6. Transmit data via Ethernet
    soc->cloud_sync.connected = true;
    bus_write(soc, ETH_TX_BUF_ADDR, eth_buf_addr);
    bus_write(soc, ETH_TX_BUF_LEN, log_entry->compressed_size);
    bus_write(soc, ETH_CTRL_REG, 0x01); // Start transmission
    soc->cloud_sync.connected = false;

    cloud_sync_update_watermark(&soc->cloud_sync, soc->event_queue.current_time);

    printf("[%lu ns] === Cloud Transfer Request Completed Successfully ===\n", 
           soc->event_queue.current_time);
}

/* ============================================================================
 * SOC INITIALIZATION
 * ============================================================================ */

void blackbox_soc_init(BlackBoxSoC* soc, bool verbose) {
    memset(soc, 0, sizeof(BlackBoxSoC));
    soc->verbose = verbose;
    
    // Initialize subsystems
    memory_init(&soc->memory);
    event_queue_init(&soc->event_queue);
    
    // Initialize APU & RPU cores
    apu_init(&soc->apu);
    rpu_init(&soc->rpu);
    
    // Initialize cloud sync
    cloud_sync_init(&soc->cloud_sync);
    
    // Initialize sensor channels: always provide 4 default placeholder channels
    // even when no external sensors are assigned. Additional sensors can be
    // added dynamically with sensor_channel_add().
    uint32_t initial_channels = 4; // default visible baseline
    soc->num_channels = initial_channels;
    soc->channels = (SensorChannel*)calloc(soc->num_channels, sizeof(SensorChannel));
    for (uint32_t i = 0; i < soc->num_channels; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "Unused_%u", i);
        sensor_channel_init(&soc->channels[i], i, name);
        sensor_channel_set_state(&soc->channels[i], CHANNEL_OFF, 0);
    }
    
    // Initialize event markers and log index
    soc->markers = NULL;
    soc->log_index = NULL;
    
    // Open NVMe storage file
    soc->nvme.storage_file = fopen("nvme_storage.bin", "wb");
    
    printf("BlackBox DPU Virtual Platform Initialized\n");
    printf("=========================================\n");
    printf("Heterogeneous Processing:\n");
    printf("  APU (Application):   %s\n", soc->apu.initialized ? "Online" : "Offline");
    printf("  RPU (Real-time):     %s\n", soc->rpu.initialized ? "Online" : "Offline");
    printf("\nMemory Map:\n");
    printf("  Shared Buffer Memory: 0x%08X - 0x%08X (%u MB)\n", 
           SBM_BASE, SBM_BASE + SBM_SIZE - 1, SBM_SIZE / (1024*1024));
    printf("  DRAM: 0x%08X - 0x%08X (%u MB)\n",
           DRAM_BASE, DRAM_BASE + DRAM_SIZE - 1, DRAM_SIZE / (1024*1024));
    printf("\nHardware Accelerators:\n");
    printf("  Zstd Accelerator: 0x%08X\n", ZSTD_REGS_BASE);
    printf("  DMA Engine: 0x%08X (4 channels)\n", DMA_REGS_BASE);
    printf("  NVMe Controller: 0x%08X\n", PCIE_REGS_BASE);
    printf("  Ethernet MAC: 0x%08X\n", ETH_MAC_REGS_BASE);
    printf("\nSensor Channels: %u configured\n", soc->num_channels);
    printf("Security Model: Local-First (remote config %s)\n\n",
        soc->apu.allow_remote_config ? "ENABLED" : "DISABLED");
    // Print initial live display for channels (will be overwritten in-place)
    soc_display_channels(soc);
}

void blackbox_soc_cleanup(BlackBoxSoC* soc) {
    memory_cleanup(&soc->memory);
    if (soc->nvme.storage_file) {
        fclose(soc->nvme.storage_file);
    }
    
    // Clean up sensor channels
    if (soc->channels) {
        free(soc->channels);
    }
    
    // Clean up event markers
    while (soc->markers) {
        EventMarker* marker = soc->markers;
        soc->markers = marker->next;
        free(marker);
    }
    
    // Clean up log index
    while (soc->log_index) {
        LogIndex* entry = soc->log_index;
        soc->log_index = entry->next;
        free(entry);
    }
    
    // Clean up remaining events
    while (soc->event_queue.head) {
        Event* event = soc->event_queue.head;
        soc->event_queue.head = event->next;
        free(event);
    }
}

/* ============================================================================
 * HIGH-LEVEL DATA FLOW ORCHESTRATION (Section 5.1)
 * ============================================================================ */

void blackbox_process_data_block(BlackBoxSoC* soc, uint8_t* input_data, uint32_t data_size) {
    printf("\n[%lu ns] === Starting Dual-Path Logging Pipeline ===\n", 
           soc->event_queue.current_time);
    
    uint64_t pipeline_start = soc->event_queue.current_time;
    
    // Step 1: Copy input data to SBM input buffer
    uint32_t input_buf_addr = SBM_BASE;
    uint8_t* input_buf = memory_translate(&soc->memory, input_buf_addr);
    memcpy(input_buf, input_data, data_size);
    
    // Step 2: Configure and start Zstd compression
    uint32_t comp_output_addr = SBM_BASE + (1024 * 1024);  // 1MB offset in SBM
    
    bus_write(soc, ZSTD_SRC_ADDR_REG, input_buf_addr);
    bus_write(soc, ZSTD_DST_ADDR_REG, comp_output_addr);
    bus_write(soc, ZSTD_LENGTH_REG, data_size);
    bus_write(soc, ZSTD_LEVEL_REG, 3);
    bus_write(soc, ZSTD_CTRL_REG, ZSTD_CTRL_START);
    
    // Process events until compression completes
    while (soc->zstd.busy) {
        event_process_next(&soc->event_queue);
        // Update live channel display while waiting
        soc_display_channels(soc);
        soc_poll_input(soc);
    }
    
    uint32_t compressed_size = bus_read(soc, ZSTD_COMP_SIZE_REG);
    
    // Step 3: Configure DMA for NVMe logging
    uint32_t nvme_buf_addr = SBM_BASE + (2 * 1024 * 1024);  // 2MB offset

    // Configure DMA Channel 2 for NVMe logging
    bus_write(soc, DMA_CH2_CTRL + 0x08, comp_output_addr);  // SRC_ADDR
    bus_write(soc, DMA_CH2_CTRL + 0x0C, nvme_buf_addr);     // DST_ADDR
    bus_write(soc, DMA_CH2_CTRL + 0x10, compressed_size);   // LENGTH
    bus_write(soc, DMA_CH2_CTRL, DMA_CTRL_START);
    
    // Process DMA transfer event
    while (soc->dma.channels[2].busy) {
        event_process_next(&soc->event_queue);
        // Update live channel display while waiting for DMA
        soc_display_channels(soc);
        soc_poll_input(soc);
    }
    
    // Step 4: Add log index entry
    add_log_index_entry(soc, pipeline_start, soc->event_queue.current_time,
                       soc->nvme.bytes_written, compressed_size);
    
    // Step 5: Write to NVMe storage
    bus_write(soc, NVME_WRITE_BUF_ADDR, nvme_buf_addr);
    bus_write(soc, NVME_WRITE_BUF_LEN, compressed_size);
    bus_write(soc, NVME_CTRL_REG, 0x01);  // Start write
    
    printf("[%lu ns] === Local Logging Complete ===\n\n", 
           soc->event_queue.current_time);
}

/* ============================================================================
 * STATISTICS REPORTING
 * ============================================================================ */

void print_statistics(BlackBoxSoC* soc) {
    printf("\n");
    printf("============================================================\n");
    printf("              BlackBox SoC Statistics Report                \n");
    printf("============================================================\n");
    
    printf("\nHeterogeneous Cores Status:\n");
    printf("  APU (Application):    %s\n", soc->apu.initialized ? "Online" : "Offline");
    printf("  RPU (Real-time):      %s\n", soc->rpu.initialized ? "Online" : "Offline");
    printf("  Security Model:       Local-First (%s)\n", 
           soc->apu.allow_remote_config ? "Remote Allowed" : "Remote Blocked");
    
    printf("\nSensor Channels:\n");
    for (uint32_t i = 0; i < soc->num_channels; i++) {
        SensorChannel* ch = &soc->channels[i];
        printf("  CH%u [%-16s]: %s (Health: %.1f%%)\n",
               ch->channel_id, ch->name,
               ch->state == CHANNEL_ON ? "ON" :
               ch->state == CHANNEL_FROZEN ? "FROZEN" :
               ch->state == CHANNEL_RECORDING ? "REC" : "OFF",
               ch->health_score * 100.0f);
    }
    
    printf("\nCompression Statistics:\n");
    printf("  Total input data:     %u bytes\n", soc->zstd.length);
    printf("  Compressed output:    %u bytes\n", soc->zstd.compressed_size);
    printf("  Compression ratio:    %.2f%%\n", 
           (100.0 * soc->zstd.compressed_size) / soc->zstd.length);
    
    printf("\nStorage Path (NVMe):\n");
    printf("  Total writes:         %u\n", soc->nvme.writes_completed);
    printf("  Total bytes written:  %lu bytes\n", soc->nvme.bytes_written);
    
    printf("\nCloud Path (Ethernet):\n");
    printf("  Connection status:    %s\n", soc->cloud_sync.connected ? "Connected" : "Disconnected");
    printf("  Total packets:        %u\n", soc->eth_mac.packets_transmitted);
    printf("  Total bytes sent:     %lu bytes\n", soc->eth_mac.bytes_transmitted);
    printf("  Backlog bytes:        %lu bytes\n", soc->cloud_sync.backlog_bytes);
    printf("  Last sync watermark:  %lu ns\n", soc->cloud_sync.last_sync_timestamp);
    
    printf("\nNetwork-on-Chip Statistics:\n");
    printf("  Total transactions:   %lu\n", soc->noc_stats.total_transactions);
    printf("  Memory accesses:      %lu bytes\n", soc->noc_stats.memory_accesses);
    printf("  NVMe path traffic:    %lu bytes\n", soc->noc_stats.nvme_path_bytes);
    printf("  Ethernet path traffic:%lu bytes\n", soc->noc_stats.ethernet_path_bytes);
    
    printf("\nEvent Markers:\n");
    int marker_count = 0;
    EventMarker* m = soc->markers;
    while (m) {
        marker_count++;
        m = m->next;
    }
    printf("  Total markers:        %d\n", marker_count);
    
    printf("\nLog Index Entries:\n");
    int index_count = 0;
    LogIndex* idx = soc->log_index;
    while (idx) {
        index_count++;
        idx = idx->next;
    }
    printf("  Total index entries:  %d\n", index_count);
    
    printf("\nTiming:\n");
    printf("  Total simulation time: %lu ns\n", soc->event_queue.current_time);
    printf("  Equivalent real-time:  %.2f µs\n", soc->event_queue.current_time / 1000.0);
    
    printf("============================================================\n\n");
}
