/*
 * BlackBox DPU - Network Client
 * HTTP POST implementation for real cloud data transfer
 */

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

// Initialize network client (call once at startup)
bool network_client_init(void);

// Cleanup network client
void network_client_cleanup(void);

// Send compressed data to cloud server via HTTP POST
// Returns true on success, false on failure
bool network_send_data(const uint8_t* data, uint32_t length);

// Send status update (JSON) to cloud server
bool network_send_status(const char* json_status);

#endif // NETWORK_CLIENT_H
