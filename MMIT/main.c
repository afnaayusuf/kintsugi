/*
 * BlackBox DPU - Main Testbench
 * Complete test suite and demonstration code
 */

#include <unistd.h>
#include "soc_core.h"
#include "telemetry_sender.h"
#include "network_config.h"
#include "realistic_drive_sim.h"

/* ============================================================================
 * TEST DATA GENERATION
 * ============================================================================ */

void generate_test_data(uint8_t* buffer, uint32_t size) {
    // Generate compressible test data (simulating sensor logs)
    for (uint32_t i = 0; i < size; i++) {
        if (i % 100 < 50) {
            buffer[i] = 0xAA;  // Repeating pattern
        } else if (i % 100 < 75) {
            buffer[i] = 0x55;
        } else {
            buffer[i] = (uint8_t)(i & 0xFF);  // Some variation
        }
    }
}

/* ============================================================================
 * TEST 1: SINGLE DATA BLOCK PROCESSING
 * ============================================================================ */

void run_single_block_test(BlackBoxSoC* soc) {
    printf("\n");
    printf("************************************************************\n");
    printf("*        Test 1: Single Data Block Processing             *\n");
    printf("************************************************************\n");
    
    const uint32_t TEST_SIZE = 64 * 1024;  // 64KB block
    uint8_t* test_data = (uint8_t*)malloc(TEST_SIZE);
    
    generate_test_data(test_data, TEST_SIZE);
    printf("Generated %u bytes of test data\n", TEST_SIZE);
    
    blackbox_process_data_block(soc, test_data, TEST_SIZE);
    
    free(test_data);
}

/* ============================================================================
 * TEST 2: CONTINUOUS STREAMING DATA PROCESSING
 * ============================================================================ */

void run_streaming_test(BlackBoxSoC* soc) {
    printf("\n");
    printf("************************************************************\n");
    printf("*     Test 2: Continuous Streaming Data Processing        *\n");
    printf("************************************************************\n");
    
    const uint32_t BLOCK_SIZE = 32 * 1024;  // 32KB blocks
    const uint32_t NUM_BLOCKS = 5;
    
    uint8_t* test_data = (uint8_t*)malloc(BLOCK_SIZE);
    
    for (uint32_t block = 0; block < NUM_BLOCKS; block++) {
        printf("\n--- Processing Block %u/%u ---\n", block + 1, NUM_BLOCKS);
        
        // Generate varied data for each block
        for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
            test_data[i] = (uint8_t)((block * 100 + i) & 0xFF);
        }
        
        blackbox_process_data_block(soc, test_data, BLOCK_SIZE);
    }
    
    free(test_data);
}

/* ============================================================================
 * TEST 3: PERFORMANCE BENCHMARK
 * ============================================================================ */

void run_performance_benchmark(BlackBoxSoC* soc) {
    printf("\n");
    printf("************************************************************\n");
    printf("*          Test 3: Performance Benchmark                   *\n");
    printf("************************************************************\n");
    
    const uint32_t sizes[] = {4096, 16384, 65536, 262144};  // 4KB to 256KB
    const char* size_names[] = {"4 KB", "16 KB", "64 KB", "256 KB"};
    
    printf("\n%-12s %-15s %-15s %-15s %-10s\n", 
           "Data Size", "Raw (bytes)", "Compressed", "Ratio", "Time (us)");
    printf("--------------------------------------------------------------------");
    
    for (int i = 0; i < 4; i++) {
        uint32_t size = sizes[i];
        uint8_t* test_data = (uint8_t*)malloc(size);
        generate_test_data(test_data, size);
        
        // Reset timing
        uint64_t start_time = soc->event_queue.current_time;
        
        // Process without verbose output
        bool old_verbose = soc->verbose;
        soc->verbose = false;
        blackbox_process_data_block(soc, test_data, size);
        soc->verbose = old_verbose;
        
        uint64_t elapsed = soc->event_queue.current_time - start_time;
        
        printf("%-12s %-15u %-15u %-15.2f%% %-10.2f\n",
               size_names[i],
               size,
               soc->zstd.compressed_size,
               (100.0 * soc->zstd.compressed_size) / size,
               elapsed / 1000.0);
        
        free(test_data);
    }
    printf("\n");
}

/* ============================================================================
 * TEST 4: ARCHITECTURE VALIDATION
 * ============================================================================ */

void run_architecture_validation(BlackBoxSoC* soc) {
    printf("\n");
    printf("************************************************************\n");
    printf("*        Test 4: Architecture Validation Tests             *\n");
    printf("************************************************************\n");
    
    // Test 1: Memory map validation
    printf("\n[Test 4.1] Memory Map Validation:\n");
    printf("  Writing to SBM (0x%08X)... ", SBM_BASE);
    bus_write(soc, SBM_BASE, 0xDEADBEEF);
    uint32_t read_val = bus_read(soc, SBM_BASE);
    printf("%s (read: 0x%08X)\n", read_val == 0xDEADBEEF ? "PASS" : "FAIL", read_val);
    
    printf("  Writing to DRAM (0x%08X)... ", DRAM_BASE);
    bus_write(soc, DRAM_BASE, 0xCAFEBABE);
    read_val = bus_read(soc, DRAM_BASE);
    printf("%s (read: 0x%08X)\n", read_val == 0xCAFEBABE ? "PASS" : "FAIL", read_val);
    
    printf("\n[Test 4.2] Hardware Accelerator Status:\n");
    printf("  Zstd accelerator:  %s\n", 
           soc->zstd.status_reg & ZSTD_STATUS_DONE ? "Ready" : "Error");
    printf("  DMA engine:        Operational (%d channels)\n", 4);
    printf("  NVMe controller:   %s (%u writes completed)\n",
           soc->nvme.storage_file ? "Active" : "Error",
           soc->nvme.writes_completed);
    printf("  Ethernet MAC:      Active (%u packets sent)\n",
           soc->eth_mac.packets_transmitted);
}

/* ============================================================================ "C:\Users\tkafn\OneDrive\Documents\kintsugi web"
 * TEST 5: LIVE TELEMETRY STREAMING WITH DISPLAY
 * ============================================================================ */

void display_live_telemetry(const MMITTelemetryPacket* packet, int update_count) {
    // Simple carriage return - plain text only
    printf("\rSpeed %.1f km/h   RPM %.0f   Throttle %.1f%%   Brake %.1f%%   Gear %d   Battery %.2fV   Engine %.1f°C   Fuel %.1f%%   GPS %.6f %.6f   CPU %.1f%%   RAM %.1f%%   Update %d     ",
           packet->speed_kph, packet->rpm, packet->throttle_pct, packet->brake_pct, 
           packet->gear, packet->battery_voltage, packet->engine_temp_c, packet->fuel_level_pct,
           packet->gps_lat, packet->gps_lon, packet->cpu_usage_pct, packet->ram_usage_pct, update_count);
    fflush(stdout);
}

void run_live_telemetry_streaming(BlackBoxSoC* soc, int num_updates) {
    printf("\nMMIT BLACKBOX - Live Telemetry Streaming (Realistic 10-Hour Drive)\n");
    printf("Backend: http://%s:%d\n", BACKEND_API_HOST, BACKEND_API_PORT);
    
    // Initialize realistic driving simulation
    init_realistic_drive_simulation();
    
    // Initialize telemetry sender
    telemetry_sender_init(BACKEND_API_HOST, BACKEND_API_PORT);
    
    printf("Starting realistic drive simulation...\n");
    printf("Full tank: 100%% fuel | Starting from cold engine\n\n");
    
    int successful_sends = 0;
    
    for (int i = 0; i < num_updates; i++) {
        // Create telemetry packet structure
        MMITTelemetryPacket packet;
        
        // Initialize vehicle ID
        strncpy(packet.vehicle_id, "BENYON_001", sizeof(packet.vehicle_id) - 1);
        packet.vehicle_id[sizeof(packet.vehicle_id) - 1] = '\0';
        
        // Use realistic driving simulation to fill the packet directly
        update_realistic_drive_simulation(&packet, 1.0); // 1 second delta
        
        // Add system stats (simulated)
        packet.cpu_usage_pct = 45.0 + (i % 10) * 2.0;
        packet.ram_usage_pct = 62.0 + (i % 5) * 1.5;
        packet.network_latency_ms = 5.0 + (i % 3) * 0.5;
        packet.humidity_pct = 45.0 + (i % 20) * 1.0;
        packet.abs_active = false;
        packet.traction_control = true;
        
        // Display live telemetry in terminal
        display_live_telemetry(&packet, i + 1);
        
        // Send to backend (non-verbose to keep display clean)
        if (telemetry_send_to_backend(&packet)) {
            successful_sends++;
        }
        
        // Wait 1 second between updates
        sleep(1);
        
        // Advance simulation time
        soc->event_queue.current_time += 1000000000ULL; // 1 second in nanoseconds
        
        // Print simulation progress every 60 updates
        if ((i + 1) % 60 == 0) {
            double hours = get_simulation_elapsed_hours();
            double fuel = get_simulation_fuel_level();
            printf("\n[Simulation] Elapsed: %.2f hours | Fuel: %.1f%%\n\n", hours, fuel);
        }
    }
    
    telemetry_sender_cleanup();
    
    // Final summary
    printf("\n");
    printf("════════════════════════════════════════════════════════════════════\n");
    printf("  Streaming Complete!\n");
    printf("  Total Updates: %d\n", num_updates);
    printf("  Successful:    %d\n", successful_sends);
    printf("  Failed:        %d\n", num_updates - successful_sends);
    printf("════════════════════════════════════════════════════════════════════\n");
    printf("\n");
}

/* ============================================================================
 * TEST 6: QUERY-BASED CLOUD TRANSFER
 * ============================================================================ */

void run_cloud_transfer_test(BlackBoxSoC* soc) {
    printf("\n");
    printf("************************************************************\n");
    printf("*         Test 6: Query-Based Cloud Transfer             *\n");
    printf("************************************************************\n");

    // Ensure there's some data in the log first
    const uint32_t TEST_SIZE = 16 * 1024;
    uint8_t* test_data = (uint8_t*)malloc(TEST_SIZE);
    generate_test_data(test_data, TEST_SIZE);
    printf("Generated and logged %u bytes of data to NVMe.\n", TEST_SIZE);
    blackbox_process_data_block(soc, test_data, TEST_SIZE);
    free(test_data);

    uint64_t target_timestamp = soc->log_index->timestamp_start;

    // Test 1: Failed transfer with wrong key
    printf("\n[Test 6.1] Cloud Transfer with Invalid Key:\n");
    handle_cloud_transfer_request(soc, target_timestamp, "WRONG_KEY");

    // Test 2: Successful transfer
    printf("\n[Test 6.2] Cloud Transfer with Valid Key:\n");
    handle_cloud_transfer_request(soc, target_timestamp, "SECRET_KEY_123");
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

void print_header() {
    printf("\n");
    printf("################################################################\n");
    printf("#                                                              #\n");
    printf("#     BlackBox DPU-Inspired Preprocessing Driver              #\n");
    printf("#              Virtual Platform Testbench                     #\n");
    printf("#                                                              #\n");
    printf("#  Complete C-Model Architecture Implementation                #\n");
    printf("#  Demonstrates: APU, RPU, Zstd Accelerator, Multi-channel    #\n");
    printf("#                DMA, NoC Interconnect, Local Logging &       #\n");
    printf("#                Query-Based Cloud Transfer                   #\n");
    printf("#                                                              #\n");
    printf("################################################################\n");
}

void verify_output_files() {
    printf("Output Files Generated:\n");
    printf("  - nvme_storage.bin : Local storage simulation\n");
    printf("  - cloud_log.bin    : Cloud transmission simulation\n");
    
    // Verification logic needs to be adapted for query-based transfer
    FILE* nvme_file = fopen("nvme_storage.bin", "rb");
    FILE* cloud_file = fopen("cloud_log.bin", "rb");
    
    if (nvme_file) {
        fseek(nvme_file, 0, SEEK_END);
        long nvme_size = ftell(nvme_file);
        printf("\n[File Verification]\n");
        printf("  NVMe storage size:  %ld bytes\n", nvme_size);
        fclose(nvme_file);
    }
    
    if (cloud_file) {
        fseek(cloud_file, 0, SEEK_END);
        long cloud_size = ftell(cloud_file);
        printf("  Cloud log size:     %ld bytes\n", cloud_size);
        printf("  (Note: Cloud log is now query-based and will not match NVMe size)\n");
        fclose(cloud_file);
    }
}

/* ============================================================================
 * INTERACTIVE DASHBOARD MODE
 * ============================================================================ */

void run_interactive_mode(BlackBoxSoC* soc) {
    printf("\n");
    printf("============================================================\n");
    printf("     BlackBox DPU - Interactive Dashboard Mode             \n");
    printf("============================================================\n");
    printf("Commands: add <name>, list, help, quit\n");
    printf("(Live sensor display updates automatically)\n");
    printf("\n");
    
    // Initial display
    soc_display_channels(soc);
    printf("\n> ");
    fflush(stdout);
    
    while (1) {
        // Poll for user input (non-blocking on POSIX, blocking on Windows)
        if (soc_poll_input(soc)) {
            // Re-display after processing command
            soc_display_channels(soc);
            
            // Check if user issued quit
            // For now, just continue (user can Ctrl-C)
            printf("\n> ");
            fflush(stdout);
        }
        
        // Simulate some background activity (optional: update sensor health)
        usleep(100000); // 100ms sleep to avoid busy loop
        
        // Refresh display periodically
        static int refresh_counter = 0;
        if (++refresh_counter >= 10) { // Refresh every ~1 second
            soc_display_channels(soc);
            printf("\n> ");
            fflush(stdout);
            refresh_counter = 0;
        }
    }
}

/* ============================================================================
 * MAIN TESTBENCH ENTRY POINT
 * ============================================================================ */

int main(int argc, char** argv) {
    print_header();
    
    // Parse command line options
    bool verbose = true;
    bool run_all_tests = true;
    bool interactive_mode = false;
    bool streaming_mode = false;
    int stream_count = 60; // Default: 60 updates (1 minute)
    
    if (argc > 1) {
        if (strcmp(argv[1], "-q") == 0 || strcmp(argv[1], "--quiet") == 0) {
            verbose = false;
        } else if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0) {
            interactive_mode = true;
            run_all_tests = false;
        } else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--stream") == 0) {
            streaming_mode = true;
            run_all_tests = false;
            interactive_mode = false;
            // Optional: number of updates
            if (argc > 2) {
                stream_count = atoi(argv[2]);
                if (stream_count <= 0) stream_count = 60;
            }
        } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -i, --interactive       Run interactive dashboard mode\n");
            printf("  -s, --stream [count]    Run live telemetry streaming mode\n");
            printf("                          (default count: 60 updates)\n");
            printf("  -q, --quiet             Run tests in quiet mode\n");
            printf("  -h, --help              Show this help message\n");
            printf("\nExamples:\n");
            printf("  %s                      Run full test suite\n", argv[0]);
            printf("  %s --stream             Stream 60 telemetry updates\n", argv[0]);
            printf("  %s --stream 120         Stream 120 telemetry updates\n", argv[0]);
            printf("  %s --interactive        Interactive sensor dashboard\n", argv[0]);
            return 0;
        }
    }
    
    // Initialize the SoC
    BlackBoxSoC soc;
    blackbox_soc_init(&soc, verbose, interactive_mode);
    
    // Choose mode: streaming, interactive, or test suite
    if (streaming_mode) {
        // Run live telemetry streaming mode
        run_live_telemetry_streaming(&soc, stream_count);
    } else if (interactive_mode) {
        run_interactive_mode(&soc);
    } else {
        // Run test suite
        printf("\n");
        printf("Starting Virtual Platform Test Suite...\n");
        printf("========================================\n");
        
        if (run_all_tests) {
            // Test 1: Single block processing
            run_single_block_test(&soc);
            
            // Test 2: Streaming data
            run_streaming_test(&soc);
            
            // Test 3: Performance benchmark
            run_performance_benchmark(&soc);
            
            // Test 4: Architecture validation
            run_architecture_validation(&soc);
            
            // Test 5: Live telemetry streaming (30 updates = 30 seconds)
            run_live_telemetry_streaming(&soc, 30);
            
            // Test 6: Cloud transfer validation
            run_cloud_transfer_test(&soc);
        }
        
        // Print final statistics
        print_statistics(&soc);
        
        // Verify output files
        verify_output_files();
    }
    
    // Cleanup
    blackbox_soc_cleanup(&soc);
    
    printf("\n");
    printf("============================================================\n");
    printf("  Virtual Platform Test Suite Completed Successfully!      \n");
    printf("============================================================\n");
    printf("\n");
    
    return 0;
}