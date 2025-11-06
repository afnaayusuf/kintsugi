/*
 * BlackBox DPU - Telemetry Sender Implementation
 * Sends formatted JSON telemetry to FastAPI backend
 */

#include "telemetry_sender.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __unix__
#include <curl/curl.h>
#else
// Stub for Windows
#endif

static char g_backend_url[256] = {0};
static int g_backend_port = 8000;
static bool g_sender_initialized = false;

bool telemetry_sender_init(const char* backend_url, int backend_port) {
#ifdef __unix__
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "Telemetry Sender: Failed to initialize libcurl\n");
        return false;
    }
#endif
    
    snprintf(g_backend_url, sizeof(g_backend_url), "%s", backend_url);
    g_backend_port = backend_port;
    g_sender_initialized = true;
    
    printf("Telemetry Sender: Initialized (backend: %s:%d)\n", backend_url, backend_port);
    return true;
}

void telemetry_sender_cleanup(void) {
#ifdef __unix__
    if (g_sender_initialized) {
        curl_global_cleanup();
        g_sender_initialized = false;
    }
#endif
}

bool telemetry_send_to_backend(const MMITTelemetryPacket* packet) {
    if (!g_sender_initialized) {
        fprintf(stderr, "Telemetry Sender: Not initialized\n");
        return false;
    }

#ifdef __unix__
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Telemetry Sender: Failed to create CURL handle\n");
        return false;
    }

    // Get current timestamp in ISO format
    time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", tm_info);
    
    // Build JSON payload matching backend TelemetryData model
    char json_payload[2048];
    snprintf(json_payload, sizeof(json_payload),
        "{"
        "\"vehicle_id\":\"%s\","
        "\"timestamp\":\"%sZ\","
        "\"telemetry\":{"
            "\"speed_kph\":%.2f,"
            "\"rpm\":%.2f,"
            "\"throttle_pct\":%.2f,"
            "\"brake_pct\":%.2f,"
            "\"gear\":%d,"
            "\"battery_voltage\":%.2f,"
            "\"engine_temp_c\":%.2f,"
            "\"fuel_level_pct\":%.2f,"
            "\"gps\":{\"lat\":%.6f,\"lon\":%.6f},"
            "\"ambient_temp_c\":%.2f,"
            "\"humidity_pct\":%.2f,"
            "\"wheel_speed\":{"
                "\"front_left\":%.2f,"
                "\"front_right\":%.2f,"
                "\"rear_left\":%.2f,"
                "\"rear_right\":%.2f"
            "}"
        "},"
        "\"system\":{"
            "\"cpu_usage_pct\":%.2f,"
            "\"ram_usage_pct\":%.2f,"
            "\"network_latency_ms\":%.2f,"
            "\"last_sync\":\"%sZ\""
        "},"
        "\"status\":{"
            "\"ABS_active\":%s,"
            "\"traction_control\":%s,"
            "\"DTC\":[]"
        "}"
        "}",
        packet->vehicle_id,
        timestamp,
        packet->speed_kph,
        packet->rpm,
        packet->throttle_pct,
        packet->brake_pct,
        packet->gear,
        packet->battery_voltage,
        packet->engine_temp_c,
        packet->fuel_level_pct,
        packet->gps_lat,
        packet->gps_lon,
        packet->ambient_temp_c,
        packet->humidity_pct,
        packet->wheel_fl,
        packet->wheel_fr,
        packet->wheel_rl,
        packet->wheel_rr,
        packet->cpu_usage_pct,
        packet->ram_usage_pct,
        packet->network_latency_ms,
        timestamp,
        packet->abs_active ? "true" : "false",
        packet->traction_control ? "true" : "false"
    );

    // Build full URL
    char url[512];
    snprintf(url, sizeof(url), "http://%s:%d/api/v1/telemetry/%s/update",
             g_backend_url, g_backend_port, packet->vehicle_id);

    // Configure request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json_payload));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    
    // Set content type header
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    bool success = false;
    static int error_count = 0;
    if (res == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            success = true;
            error_count = 0;  // Reset error count on success
        } else {
            if (error_count < 3) {  // Only show first 3 errors
                fprintf(stderr, "Telemetry Sender: Server returned HTTP %ld\n", response_code);
                error_count++;
            }
        }
    } else {
        if (error_count < 3) {  // Only show first 3 errors
            fprintf(stderr, "Telemetry Sender: Transfer failed - %s\n", curl_easy_strerror(res));
            error_count++;
            if (error_count == 3) {
                fprintf(stderr, "Telemetry Sender: Further errors will be suppressed\n");
            }
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
#else
    // Windows stub - just log
    printf("Telemetry Sender: [STUB] Would send telemetry for vehicle %s\n", packet->vehicle_id);
    printf("  Speed: %.2f km/h, RPM: %.2f, Gear: %d\n", 
           packet->speed_kph, packet->rpm, packet->gear);
    return true;
#endif
}

void mmit_sensors_to_telemetry(BlackBoxSoC* soc, MMITTelemetryPacket* packet, const char* vehicle_id) {
    // Initialize packet
    memset(packet, 0, sizeof(MMITTelemetryPacket));
    snprintf(packet->vehicle_id, sizeof(packet->vehicle_id), "%s", vehicle_id);
    
    // Simulate realistic driving scenario based on time
    uint64_t time_sec = soc->event_queue.current_time / 1000000000ULL;
    uint64_t time_ms = soc->event_queue.current_time / 1000000;
    
    // Create varied driving patterns
    int scenario = (int)(time_sec / 10) % 5; // Change scenario every 10 seconds
    
    switch (scenario) {
        case 0: // City driving - moderate speed with variations
            packet->speed_kph = 40.0f + (float)(time_sec % 20) + (float)((time_ms % 100) / 100.0f);
            packet->rpm = 1500.0f + (float)(time_sec % 800) + (float)((time_ms % 100) * 2);
            packet->throttle_pct = 30.0f + (float)(time_sec % 25);
            packet->brake_pct = (time_sec % 5 == 0) ? 40.0f : 5.0f;
            packet->gear = 3;
            break;
            
        case 1: // Highway cruising - steady high speed
            packet->speed_kph = 100.0f + (float)((time_ms % 50) / 10.0f);
            packet->rpm = 3000.0f + (float)((time_ms % 200));
            packet->throttle_pct = 50.0f + (float)((time_ms % 10) / 2.0f);
            packet->brake_pct = 0.0f;
            packet->gear = 5;
            break;
            
        case 2: // Acceleration
            packet->speed_kph = 20.0f + (float)(time_sec % 10) * 8.0f;
            packet->rpm = 1000.0f + (float)(time_sec % 10) * 400.0f;
            packet->throttle_pct = 80.0f + (float)((time_ms % 20));
            packet->brake_pct = 0.0f;
            packet->gear = 2 + (int)(time_sec % 4);
            break;
            
        case 3: // Deceleration/Braking
            packet->speed_kph = 80.0f - (float)(time_sec % 10) * 7.0f;
            if (packet->speed_kph < 0) packet->speed_kph = 0;
            packet->rpm = 3500.0f - (float)(time_sec % 10) * 250.0f;
            if (packet->rpm < 800) packet->rpm = 800.0f;
            packet->throttle_pct = 10.0f;
            packet->brake_pct = 60.0f - (float)(time_sec % 10) * 5.0f;
            packet->gear = 4 - (int)(time_sec % 3);
            if (packet->gear < 1) packet->gear = 1;
            break;
            
        case 4: // Idle/Stopped
            packet->speed_kph = 0.0f;
            packet->rpm = 800.0f + (float)((time_ms % 50));
            packet->throttle_pct = 0.0f;
            packet->brake_pct = 100.0f;
            packet->gear = 0; // Neutral
            break;
    }
    
    // Vehicle systems
    packet->battery_voltage = 12.6f - (float)((time_ms % 100) / 1000.0f);
    packet->engine_temp_c = 85.0f + (float)((time_sec % 20) / 2.0f);
    packet->fuel_level_pct = 75.0f - (float)(time_sec / 100.0f); // Slowly decreasing
    if (packet->fuel_level_pct < 10.0f) packet->fuel_level_pct = 75.0f; // Reset
    
    // GPS coordinates (simulating movement around Kochi, Kerala, India)
    float gps_movement = (float)(time_sec % 1000) / 100000.0f;
    packet->gps_lat = 10.0053f + gps_movement;
    packet->gps_lon = 76.3601f + gps_movement * 1.2f;
    
    // Environmental sensors
    packet->ambient_temp_c = 28.0f + (float)((time_sec % 10)) + (float)((time_ms % 100) / 100.0f);
    packet->humidity_pct = 65.0f + (float)((time_sec % 15));
    
    // Wheel speeds (proportional to vehicle speed with small variations)
    float wheel_variation = (float)((time_ms % 10) - 5) / 10.0f;
    packet->wheel_fl = packet->speed_kph + wheel_variation;
    packet->wheel_fr = packet->speed_kph + wheel_variation + 0.5f;
    packet->wheel_rl = packet->speed_kph - wheel_variation;
    packet->wheel_rr = packet->speed_kph - wheel_variation + 0.3f;
    
    // Ensure no negative wheel speeds
    if (packet->wheel_fl < 0) packet->wheel_fl = 0;
    if (packet->wheel_fr < 0) packet->wheel_fr = 0;
    if (packet->wheel_rl < 0) packet->wheel_rl = 0;
    if (packet->wheel_rr < 0) packet->wheel_rr = 0;
    
    // System stats
    packet->cpu_usage_pct = 30.0f + (float)((time_ms % 30));
    packet->ram_usage_pct = 40.0f + (float)((time_ms % 25));
    packet->network_latency_ms = 40.0f + (float)((time_ms % 30));
    
    // Status flags - activate ABS during hard braking
    packet->abs_active = (packet->brake_pct > 50.0f && packet->speed_kph > 30.0f);
    packet->traction_control = true;
}
