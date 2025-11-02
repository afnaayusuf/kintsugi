/*
 * BlackBox DPU - Network Configuration
 * Configure cloud server connection settings
 */

#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

// Cloud server configuration (laptop acting as server)
// CHANGE THIS to your laptop's IP address
#define CLOUD_SERVER_IP     "192.168.1.100"  // Update with your laptop's local IP
#define CLOUD_SERVER_PORT   8080

// API endpoints
#define UPLOAD_ENDPOINT     "/api/upload"
#define STATUS_ENDPOINT     "/api/status"

// WebSocket configuration (optional for future)
#define WEBSOCKET_PORT      8081

// Connection settings
#define HTTP_TIMEOUT_SEC    10
#define MAX_RETRIES         3
#define RETRY_DELAY_MS      1000

#endif // NETWORK_CONFIG_H
