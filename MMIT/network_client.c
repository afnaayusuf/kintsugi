/*
 * BlackBox DPU - Network Client Implementation
 * Real HTTP communication using libcurl
 */

#include "network_client.h"
#include "network_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __unix__
#include <curl/curl.h>
#else
// Stub for Windows development
#endif

static bool g_network_initialized = false;

bool network_client_init(void) {
#ifdef __unix__
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "Network: Failed to initialize libcurl\n");
        return false;
    }
    g_network_initialized = true;
    printf("Network: Client initialized (server: %s:%d)\n", 
           CLOUD_SERVER_IP, CLOUD_SERVER_PORT);
    return true;
#else
    printf("Network: Stub mode (libcurl not available on Windows)\n");
    g_network_initialized = true;
    return true;
#endif
}

void network_client_cleanup(void) {
#ifdef __unix__
    if (g_network_initialized) {
        curl_global_cleanup();
        g_network_initialized = false;
    }
#endif
}

bool network_send_data(const uint8_t* data, uint32_t length) {
    if (!g_network_initialized) {
        fprintf(stderr, "Network: Client not initialized\n");
        return false;
    }

#ifdef __unix__
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Network: Failed to create CURL handle\n");
        return false;
    }

    // Build full URL
    char url[256];
    snprintf(url, sizeof(url), "http://%s:%d%s", 
             CLOUD_SERVER_IP, CLOUD_SERVER_PORT, UPLOAD_ENDPOINT);

    // Configure request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)length);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT_SEC);
    
    // Set content type header
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    bool success = false;
    if (res == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            printf("Network: Successfully uploaded %u bytes to cloud\n", length);
            success = true;
        } else {
            fprintf(stderr, "Network: Server returned HTTP %ld\n", response_code);
        }
    } else {
        fprintf(stderr, "Network: Transfer failed - %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
#else
    // Windows stub - just log
    printf("Network: [STUB] Would upload %u bytes to %s:%d%s\n", 
           length, CLOUD_SERVER_IP, CLOUD_SERVER_PORT, UPLOAD_ENDPOINT);
    return true;
#endif
}

bool network_send_status(const char* json_status) {
    if (!g_network_initialized) {
        return false;
    }

#ifdef __unix__
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    char url[256];
    snprintf(url, sizeof(url), "http://%s:%d%s", 
             CLOUD_SERVER_IP, CLOUD_SERVER_PORT, STATUS_ENDPOINT);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_status);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json_status));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT_SEC);
    
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    bool success = (res == CURLE_OK);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
#else
    printf("Network: [STUB] Would send status: %s\n", json_status);
    return true;
#endif
}
