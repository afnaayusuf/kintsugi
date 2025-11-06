#include "realistic_drive_sim.h"
#include "telemetry_sender.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Driving simulation state
typedef struct {
    double elapsed_hours;           // Total elapsed time in hours
    double speed_kph;               // Current speed
    double fuel_level_pct;          // Fuel level (starts at 100%)
    double engine_temp_c;           // Engine temperature
    double battery_voltage;         // Battery voltage
    double throttle_pct;            // Throttle position
    double brake_pct;               // Brake position
    int gear;                       // Current gear
    double rpm;                     // Engine RPM
    double ambient_temp_c;          // Outside temperature
    int driving_mode;               // 0=city, 1=highway, 2=idle
} DriveState;

static DriveState state = {0};

// Helper: Random float between min and max
static double random_range(double min, double max) {
    return min + ((double)rand() / RAND_MAX) * (max - min);
}

// Helper: Smooth transition towards target
static double smooth_approach(double current, double target, double rate) {
    double diff = target - current;
    if (fabs(diff) < rate) return target;
    return current + (diff > 0 ? rate : -rate);
}

void init_realistic_drive_simulation(void) {
    srand(time(NULL));
    
    // Initialize starting state
    state.elapsed_hours = 0.0;
    state.speed_kph = 0.0;
    state.fuel_level_pct = 100.0;       // Full tank
    state.engine_temp_c = 25.0;         // Ambient start
    state.battery_voltage = 12.6;       // Fully charged
    state.throttle_pct = 0.0;
    state.brake_pct = 0.0;
    state.gear = 0;                     // Neutral
    state.rpm = 800;                    // Idle RPM
    state.ambient_temp_c = 25.0;
    state.driving_mode = 0;             // Start in city mode
}

void update_realistic_drive_simulation(MMITTelemetryPacket* telemetry, double delta_seconds) {
    state.elapsed_hours += delta_seconds / 3600.0;
    
    // Determine driving mode based on time (simulate realistic 10-hour drive)
    double hour = fmod(state.elapsed_hours, 10.0);
    
    if (hour < 2.0) {
        // Hours 0-2: City driving (stop and go)
        state.driving_mode = 0;
    } else if (hour < 7.0) {
        // Hours 2-7: Highway driving (steady speed)
        state.driving_mode = 1;
    } else if (hour < 7.5) {
        // Hours 7-7.5: Rest stop (idle)
        state.driving_mode = 2;
    } else {
        // Hours 7.5-10: City driving back
        state.driving_mode = 0;
    }
    
    // Update based on driving mode
    double target_speed = 0.0;
    double target_throttle = 0.0;
    double target_brake = 0.0;
    
    switch (state.driving_mode) {
        case 0: // City driving
            // Simulate stop-and-go traffic
            if (rand() % 100 < 30) {
                // 30% chance to be stopped/slowing
                target_speed = random_range(0, 30);
                target_throttle = 0;
                target_brake = random_range(20, 60);
            } else {
                // Accelerating or cruising
                target_speed = random_range(30, 60);
                target_throttle = random_range(20, 50);
                target_brake = 0;
            }
            break;
            
        case 1: // Highway driving
            // Steady cruise with occasional speed changes
            target_speed = random_range(100, 120);
            target_throttle = random_range(30, 45);
            target_brake = 0;
            
            // Occasional slowdowns for traffic
            if (rand() % 100 < 10) {
                target_speed = random_range(70, 90);
                target_throttle = random_range(10, 20);
            }
            break;
            
        case 2: // Idle (rest stop)
            target_speed = 0;
            target_throttle = 0;
            target_brake = 100;
            break;
    }
    
    // Smooth transitions
    state.speed_kph = smooth_approach(state.speed_kph, target_speed, delta_seconds * 2.0);
    state.throttle_pct = smooth_approach(state.throttle_pct, target_throttle, delta_seconds * 10.0);
    state.brake_pct = smooth_approach(state.brake_pct, target_brake, delta_seconds * 15.0);
    
    // Calculate gear based on speed
    if (state.speed_kph < 5) {
        state.gear = 0;
    } else if (state.speed_kph < 20) {
        state.gear = 1;
    } else if (state.speed_kph < 40) {
        state.gear = 2;
    } else if (state.speed_kph < 60) {
        state.gear = 3;
    } else if (state.speed_kph < 80) {
        state.gear = 4;
    } else if (state.speed_kph < 100) {
        state.gear = 5;
    } else {
        state.gear = 6;
    }
    
    // Calculate RPM based on speed and gear
    if (state.gear == 0) {
        state.rpm = 800 + state.throttle_pct * 20; // Idle to 2800 RPM
    } else {
        state.rpm = 1000 + (state.speed_kph / state.gear) * 40;
        state.rpm += state.throttle_pct * 10; // Throttle adds RPM
    }
    
    // Clamp RPM
    if (state.rpm < 600) state.rpm = 600;
    if (state.rpm > 7000) state.rpm = 7000;
    
    // Engine temperature - increases with RPM and throttle
    double target_temp = 85.0; // Normal operating temp
    if (state.rpm > 3000) {
        target_temp = 85.0 + (state.rpm - 3000) * 0.01;
    }
    if (state.driving_mode == 2) {
        target_temp = 75.0; // Cools down at idle
    }
    state.engine_temp_c = smooth_approach(state.engine_temp_c, target_temp, delta_seconds * 0.5);
    
    // Fuel consumption (realistic)
    // City: ~10L/100km, Highway: ~6L/100km, Idle: ~0.8L/hour
    double fuel_consumption_rate = 0.0;
    if (state.driving_mode == 0) {
        // City driving
        fuel_consumption_rate = (state.speed_kph * 10.0 / 100.0) / 3600.0; // L/second
    } else if (state.driving_mode == 1) {
        // Highway driving
        fuel_consumption_rate = (state.speed_kph * 6.0 / 100.0) / 3600.0;
    } else {
        // Idle
        fuel_consumption_rate = 0.8 / 3600.0;
    }
    
    // Assume 50L tank capacity
    double tank_capacity_liters = 50.0;
    double fuel_used = fuel_consumption_rate * delta_seconds;
    state.fuel_level_pct -= (fuel_used / tank_capacity_liters) * 100.0;
    if (state.fuel_level_pct < 0) state.fuel_level_pct = 0;
    
    // Battery voltage - slight fluctuations
    double target_battery = 12.4 + (state.rpm / 7000.0) * 1.8; // 12.4V to 14.2V
    if (state.driving_mode == 2) {
        target_battery = 12.2; // Drains slowly at idle
    }
    state.battery_voltage = smooth_approach(state.battery_voltage, target_battery, delta_seconds * 0.1);
    
    // Ambient temperature - simulate day/night cycle
    double time_of_day = fmod(state.elapsed_hours, 24.0);
    state.ambient_temp_c = 20.0 + 10.0 * sin((time_of_day / 24.0) * 2 * M_PI - M_PI / 2);
    
    // Fill telemetry structure (matching MMITTelemetryPacket fields)
    telemetry->speed_kph = state.speed_kph;
    telemetry->rpm = state.rpm;
    telemetry->throttle_pct = state.throttle_pct;
    telemetry->brake_pct = state.brake_pct;
    telemetry->gear = state.gear;
    telemetry->battery_voltage = state.battery_voltage;
    telemetry->engine_temp_c = state.engine_temp_c;
    telemetry->fuel_level_pct = state.fuel_level_pct;
    telemetry->ambient_temp_c = state.ambient_temp_c;
    
    // GPS (simulate movement)
    telemetry->gps_lat = 37.7749 + (state.elapsed_hours * 0.01);
    telemetry->gps_lon = -122.4194 + (state.elapsed_hours * 0.01);
    
    // Environmental - humidity varies with temperature
    telemetry->humidity_pct = 50.0 + (state.ambient_temp_c - 20.0) * 1.5 + random_range(-5, 5);
    if (telemetry->humidity_pct < 20.0) telemetry->humidity_pct = 20.0;
    if (telemetry->humidity_pct > 90.0) telemetry->humidity_pct = 90.0;
    
    // Wheel speeds (matching field names: wheel_fl, wheel_fr, wheel_rl, wheel_rr)
    double wheel_variation = random_range(-0.5, 0.5);
    telemetry->wheel_fl = state.speed_kph + wheel_variation;
    telemetry->wheel_fr = state.speed_kph + wheel_variation;
    telemetry->wheel_rl = state.speed_kph + wheel_variation;
    telemetry->wheel_rr = state.speed_kph + wheel_variation;
    
    // System stats (set by caller, but initialize here)
    telemetry->cpu_usage_pct = 0.0;
    telemetry->ram_usage_pct = 0.0;
    telemetry->network_latency_ms = 0.0;
    
    // Status flags
    telemetry->abs_active = false;
    telemetry->traction_control = true;
    
    // Add some realistic noise
    telemetry->speed_kph += random_range(-0.5, 0.5);
    telemetry->engine_temp_c += random_range(-0.3, 0.3);
    telemetry->battery_voltage += random_range(-0.05, 0.05);
}

double get_simulation_elapsed_hours(void) {
    return state.elapsed_hours;
}

double get_simulation_fuel_level(void) {
    return state.fuel_level_pct;
}
