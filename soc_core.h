/*
 * SoC Core Module - Header
 * High-level SoC orchestration and data flow
 */

#ifndef SOC_CORE_H
#define SOC_CORE_H

#include "blackbox_common.h"
#include "memory.h"
#include "event_queue.h"
#include "zstd_accelerator.h"
#include "dma_engine.h"
#include "nvme_controller.h"
#include "ethernet_mac.h"
#include "bus_interconnect.h"

/* ============================================================================
 * SOC CORE FUNCTIONS
 * ============================================================================ */

void blackbox_soc_init(BlackBoxSoC* soc, bool verbose);
void blackbox_soc_cleanup(BlackBoxSoC* soc);
void blackbox_process_data_block(BlackBoxSoC* soc, uint8_t* input_data, uint32_t data_size);
void print_statistics(BlackBoxSoC* soc);

/* ============================================================================
 * APU & RPU FUNCTIONS
 * ============================================================================ */

void apu_init(APUCore* apu);
void rpu_init(RPUCore* rpu);
bool apu_validate_config_request(APUCore* apu, bool is_local);
void rpu_monitor_sensor_health(RPUCore* rpu, SensorChannel* channel, float value);

/* ============================================================================
 * SENSOR CHANNEL MANAGEMENT
 * ============================================================================ */

void sensor_channel_init(SensorChannel* channel, uint32_t id, const char* name);
void sensor_channel_set_state(SensorChannel* channel, ChannelState state, uint64_t timestamp);
float sensor_channel_get_health(SensorChannel* channel);
void sensor_channel_add(BlackBoxSoC* soc, const char* name);
void sensor_ensure_minimum(BlackBoxSoC* soc, uint32_t min_count);
void soc_display_channels(BlackBoxSoC* soc);

// Poll for interactive input (non-blocking). On POSIX this will check stdin
// and call soc_handle_command when a full line is received. On Windows this is
// a no-op.
void soc_poll_input(BlackBoxSoC* soc);
void soc_handle_command(BlackBoxSoC* soc, const char* cmd);

/* ============================================================================
 * EVENT MARKERS & INDEXING
 * ============================================================================ */

void add_event_marker(BlackBoxSoC* soc, const char* label, const char* metadata);
void add_log_index_entry(BlackBoxSoC* soc, uint64_t ts_start, uint64_t ts_end, uint64_t offset, uint32_t comp_size);
LogIndex* query_log_by_timestamp(BlackBoxSoC* soc, uint64_t timestamp);

/* ============================================================================
 * CLOUD SYNC & NETWORK BACKLOG
 * ============================================================================ */

void cloud_sync_init(CloudSyncState* sync);
void cloud_sync_update_watermark(CloudSyncState* sync, uint64_t timestamp);
void cloud_sync_handle_reconnect(BlackBoxSoC* soc);
bool apu_request_controller_permission(APUCore* apu);
bool read_marker_key(char* buffer, size_t len);
void handle_cloud_transfer_request(BlackBoxSoC* soc, uint64_t timestamp, const char* key);

#endif // SOC_CORE_H
