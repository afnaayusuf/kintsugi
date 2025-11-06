#ifndef REALISTIC_DRIVE_SIM_H
#define REALISTIC_DRIVE_SIM_H

#include "blackbox_common.h"
#include "telemetry_sender.h"

/**
 * Initialize the realistic 10-hour driving simulation
 */
void init_realistic_drive_simulation(void);

/**
 * Update simulation and generate realistic telemetry data
 * @param telemetry Output telemetry structure to fill
 * @param delta_seconds Time elapsed since last update
 */
void update_realistic_drive_simulation(MMITTelemetryPacket* telemetry, double delta_seconds);

/**
 * Get total elapsed simulation time in hours
 */
double get_simulation_elapsed_hours(void);

/**
 * Get current fuel level percentage
 */
double get_simulation_fuel_level(void);

#endif // REALISTIC_DRIVE_SIM_H
