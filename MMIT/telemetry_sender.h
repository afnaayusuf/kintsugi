/*
 * BlackBox DPU - Telemetry Sender to Backend API
 * Formats sensor data as JSON and sends to FastAPI backend
 */

#ifndef TELEMETRY_SENDER_H
#define TELEMETRY_SENDER_H

#include "blackbox_common.h"
#include <stdint.h>
#include <stdbool.h>

// Telemetry data structure matching backend model
typedef struct {
    char vehicle_id[32];
    
    // Telemetry values from MMIT sensors
    float speed_kph;
    float rpm;
    float throttle_pct;
    float brake_pct;
    int gear;
    float battery_voltage;
    float engine_temp_c;
    float fuel_level_pct;
    
    // GPS
    float gps_lat;
    float gps_lon;
    
    // Environmental
    float ambient_temp_c;
    float humidity_pct;
    
    // Wheel speeds
    float wheel_fl;
    float wheel_fr;
    float wheel_rl;
    float wheel_rr;
    
    // System stats
    float cpu_usage_pct;
    float ram_usage_pct;
    float network_latency_ms;
    
    // Status
    bool abs_active;
    bool traction_control;
} MMITTelemetryPacket;

// Initialize telemetry sender with backend URL
bool telemetry_sender_init(const char* backend_url, int backend_port);

// Send telemetry packet to backend API
bool telemetry_send_to_backend(const MMITTelemetryPacket* packet);

// Cleanup telemetry sender
void telemetry_sender_cleanup(void);

// Convert MMIT sensor channels to telemetry packet
void mmit_sensors_to_telemetry(BlackBoxSoC* soc, MMITTelemetryPacket* packet, const char* vehicle_id);

#endif // TELEMETRY_SENDER_H
