# BlackBox DPU-Inspired Preprocessing Driver

## Overview

This project implements a **complete C-model virtual platform** for a BlackBox DPU (Data Processing Unit) inspired preprocessing driver. It demonstrates a heterogeneous SoC architecture with hardware acceleration, multi-channel DMA, and dual-path logging capabilities.

## Architecture Highlights

### Core Features
- **Heterogeneous Multi-Core Processing**: APU (Application Processing Unit) + RPU (Real-time Processing Unit)
- **Hardware Zstandard Compression Accelerator**: Dedicated compression engine
- **Multi-Channel DMA Engine**: 4-channel DMA with fan-out/tee capability for simultaneous data distribution
- **Network-on-Chip (NoC) Interconnect**: High-speed data routing between subsystems
- **Dual-Path Logging**: Simultaneous data logging to NVMe storage and Ethernet (cloud)
- **Event-Driven Simulation**: Accurate timing model for hardware behavior

### Memory Map

| Component | Base Address | Size | Description |
|-----------|-------------|------|-------------|
| Boot ROM | 0x00000000 | 128 KB | System boot code |
| Shared Buffer Memory (SBM) | 0x04000000 | 4 MB | Inter-processor data exchange |
| APU L2 Cache | 0x08000000 | 1 MB | Application processor cache |
| RPU TCM | 0x08100000 | 64 KB | Real-time processor tightly-coupled memory |
| DRAM | 0x80000000 | 512 MB | Main system memory |
| Zstd Accelerator | 0xFF800000 | 4 KB | Compression engine registers |
| DMA Controller | 0xFF810000 | 4 KB | DMA engine registers |
| PCIe/NVMe | 0xFF900000 | 1 MB | NVMe controller registers |
| Ethernet MAC | 0xFFA00000 | 1 MB | Network controller registers |

## Project Structure

```
MMIT/
├── blackbox_common.h      # Common definitions, memory map, data structures
├── event_queue.h/c        # Event-driven simulation engine
├── memory.h/c             # Memory subsystem model
├── zstd_accelerator.h/c   # Hardware compression accelerator
├── dma_engine.h/c         # Multi-channel DMA controller
├── nvme_controller.h/c    # NVMe storage controller
├── ethernet_mac.h/c       # Ethernet MAC controller
├── bus_interconnect.h/c   # NoC interconnect and bus transactions
├── soc_core.h/c           # High-level SoC orchestration
├── main.c                 # Testbench and demonstration code
├── Makefile               # Build configuration
└── README.md              # This file
```

## Module Descriptions

### 1. **Event Queue** (`event_queue.h/c`)
- Time-ordered priority queue for event-driven simulation
- Schedules hardware completion callbacks
- Manages simulation time advancement

### 2. **Memory Subsystem** (`memory.h/c`)
- Models all addressable memory regions
- Address translation for different memory regions
- Allocates and manages virtual memory buffers

### 3. **Zstandard Accelerator** (`zstd_accelerator.h/c`)
- Hardware compression engine model
- Simplified Run-Length Encoding (RLE) compression for demonstration
- Configurable compression levels
- Latency modeling: ~100ns per byte × compression level

### 4. **DMA Engine** (`dma_engine.h/c`)
- 4-channel DMA controller
- **Fan-out/Tee Mode**: Simultaneous copy to two destinations (dual-path logging)
- Zero-copy data movement
- Latency modeling: ~10ns per byte

### 5. **NVMe Controller** (`nvme_controller.h/c`)
- PCIe/NVMe storage interface
- File-backed virtual storage (`nvme_storage.bin`)
- Write statistics and monitoring

### 6. **Ethernet MAC** (`ethernet_mac.h/c`)
- Network transmission interface
- Cloud logging simulation (`cloud_log.bin`)
- Packet statistics and monitoring

### 7. **Bus Interconnect** (`bus_interconnect.h/c`)
- Memory-mapped register access
- NoC transaction routing
- Statistics collection (bandwidth, transactions)

### 8. **SoC Core** (`soc_core.h/c`)
- High-level data flow orchestration
- Subsystem initialization and cleanup
- Dual-path logging pipeline coordination

### 9. **Main Testbench** (`main.c`)
- Comprehensive test suite
- Performance benchmarks
- Architecture validation tests

## Building the Project

### Prerequisites
- GCC or Clang C compiler
- Make (GNU Make)
- Standard C library

### Compilation

```bash
make
```

This will compile all modules and create the executable `blackbox_dpu`.

### Running

```bash
# Run with verbose output (default)
./blackbox_dpu

# Run quietly (less output)
./blackbox_dpu -q
```

## Test Suite

The testbench includes five comprehensive tests:

### Test 1: Single Data Block Processing
- Processes a 64KB data block
- Demonstrates the local logging pipeline
- Verifies compression, DMA, and NVMe operations

### Test 2: Continuous Streaming Data Processing
- Processes 5 blocks of 32KB each
- Tests sustained throughput for local logging
- Validates memory management

### Test 3: Performance Benchmark
- Tests multiple data sizes (4KB to 256KB)
- Measures compression ratios
- Calculates latency per operation

### Test 4: Architecture Validation
- Memory map validation
- Hardware accelerator status checks

### Test 5: Query-Based Cloud Transfer
- Demonstrates the on-demand cloud data transfer
- Verifies the marker key validation and controller permission flow

## Key Concepts Demonstrated

### 1. **Local-First Logging and Query-Based Cloud Transfer**
Data is first written to:
- **Local NVMe Storage**: For immediate access and redundancy.

Data is transferred to the cloud only upon a valid, authorized request:
- **Cloud via Ethernet**: For remote backup and analysis.

This on-demand transfer requires a valid key from a `marker.key` file and simulated permission from the controller, ensuring a secure, local-first data security model.

### 2. **Hardware Acceleration**
Instead of CPU-based compression, the Zstd hardware accelerator:
- Reduces CPU load
- Provides consistent, predictable latency
- Enables parallel processing while compression occurs

### 3. **Event-Driven Simulation**
Models realistic hardware behavior:
- Asynchronous operations
- Hardware completion callbacks
- Accurate timing simulation

### 4. **Zero-Copy Data Movement**
DMA engine moves data directly between memory regions without CPU involvement, reducing:
- Memory bandwidth consumption
- CPU overhead
- Latency

## Output Files

After running, the following files are generated:

- **`nvme_storage.bin`**: Simulated NVMe storage contents (compressed data).
- **`cloud_log.bin`**: Simulated cloud transmission data. This file is only created after a successful, authorized cloud transfer request and will not be identical to the NVMe storage.

## Performance Characteristics

### Typical Results
- **Compression Ratio**: 30-50% (depending on data pattern)
- **Zstd Latency**: ~100ns/byte × compression level
- **DMA Latency**: ~10ns/byte
- **Dual-Path Overhead**: Minimal (fan-out in hardware)

## Use Cases

This architecture is suitable for:
1. **Automotive Black Box Systems**: Continuous sensor data logging with redundancy
2. **Industrial IoT**: Real-time data collection with local and cloud storage
3. **Network Monitoring**: Packet capture with simultaneous local and remote storage
4. **Embedded Systems**: Any application requiring reliable, redundant data logging

## Security Model: Local-First Configuration

To ensure safety, security, and data integrity, the DPU operates on a strict **local-first configuration policy**. This principle dictates that the DPU's core operational parameters cannot be modified by remote commands from the cloud by default.

*   **Read-Only Remote Access:** The cloud interface is primarily for data retrieval (querying logs) and monitoring status. It is treated as an untrusted source for configuration changes.
*   **Preventing Unauthorized Tampering:** This model prevents a remote attacker or an accidental command from disabling critical sensors, stopping logging, or altering data processing routines. In a vehicle, for example, a remote command should not be able to stop the black box from recording.
*   **Mechanism for Authorized Changes:** Configuration changes require explicit local authorization, which can be granted via:
    *   **Authenticated Local Input:** A trusted local entity, such as the vehicle's main ECU or an authenticated action by the operator (e.g., a pilot or driver), can issue a signed token or command that permits a configuration change for a limited time.
    *   **Direct Physical Access:** For major updates or maintenance, a direct, authenticated connection to a local maintenance port is required.

The APU acts as the gatekeeper, validating all incoming configuration requests and rejecting any that do not originate from or get explicitly authorized by a trusted local source.

## Driver Concept

This implementation serves as a **virtual platform** that models what a real hardware driver would do:

### In Simulation (Current)
- Models hardware behavior in software
- Perfect for algorithm development and testing
- No actual hardware required

### In Production (Real Driver)
- Would run in Linux kernel or bare-metal embedded system
- Direct hardware register access via memory-mapped I/O
- Interrupt handling for hardware events
- Integration with OS I/O subsystem

## Future Enhancements

Potential improvements:
- [ ] Real Zstandard compression library integration
- [ ] PCIe link simulation
- [ ] Network protocol stack (TCP/IP)
- [ ] Power management model
- [ ] Error injection and recovery testing
- [ ] Multi-threaded processing simulation
- [ ] GUI for visualization

## Contributing

This is a demonstration project for understanding DPU architecture and driver development concepts.

## References

- **Zstandard Compression**: https://github.com/facebook/zstd
- **NVMe Specification**: https://nvmexpress.org/
- **DMA Concepts**: Various embedded systems documentation
- **Network-on-Chip**: Academic research papers on SoC interconnects

## License

Educational and demonstration purposes.

## Contact

For questions about this architecture implementation, refer to the project documentation.

---

**Note**: This is a simulation/virtual platform. No actual hardware is accessed. All operations are modeled in software for educational and development purposes.

## Advanced Features & DAW Analogy

This section outlines advanced operational routines and expands the system concept using the analogy of a Digital Audio Workstation (DAW), which is highly appropriate for a multi-channel, time-series data processing architecture like this DPU.

### Core Operational Routines

#### 1. Routine for Timestamp-Specific Data Queries
When a query arrives for data from a specific sensor at a given timestamp, the APU (Application Processing Unit) executes the following routine:
1.  **Log Indexing:** The system maintains a separate index file or a metadata header in each log chunk. This index maps time ranges to file offsets in the `nvme_storage.bin`.
2.  **Seek & Retrieve:** The APU uses the index to perform a fast seek to the relevant data block, avoiding a full scan of the storage file.
3.  **Decompression & Filtering:** The target block is read from NVMe into DRAM and decompressed by the Zstd accelerator.
4.  **Micro-Scan:** The APU then scans the small, decompressed block to pinpoint the exact timestamp and sensor ID, returning the requested data value.

#### 2. Solution for Flagging Malfunctioning Sensors
Flagging a sensor malfunction requires a high degree of confidence. This is handled by the RPU (Real-time Processing Unit) running a dedicated "Sensor Health" task.
1.  **Multi-Factor Justification:** A sensor is flagged only if multiple conditions are met simultaneously. The RPU analyzes incoming data for:
    *   **Stagnation:** The sensor value has not changed for a statistically improbable duration.
    *   **Out-of-Bounds:** The value is physically impossible (e.g., -100°C).
    *   **Anomalous Jitter:** The value is fluctuating in a way that violates its known physical constraints or noise profile.
    *   **Cross-Sensor Discrepancy:** The sensor's value significantly deviates from a group of adjacent, correlated sensors.
2.  **Confidence Score:** Instead of a binary flag, the RPU generates a "health score" (0.0 to 1.0). A score below a configurable threshold (e.g., 0.3) marks the data stream as "unreliable."
3.  **Event Flagging:** When a sensor is flagged, a special "SensorUnreliable" event is injected into the data stream via the DMA, time-stamped like any other data point.

#### 3. Network Backlog and Data Redemption
The dual-path logging architecture is ideal for this.
1.  **Local First:** All data is unconditionally written to the local `nvme_storage.bin`. This is the source of truth.
2.  **Cloud Sync Watermark:** The Ethernet MAC controller (or a process on the APU managing it) maintains a "high watermark" of the last timestamp successfully transmitted to the cloud. This is stored persistently.
3.  **Connection Loss:** If the network connection is lost, the Ethernet path simply stops consuming data. The local NVMe logging continues uninterrupted.
4.  **Re-establishment:** When the connection returns, the cloud sync process reads the watermark, seeks to that position in the `nvme_storage.bin`, and begins streaming the "lost" data. It injects a "Backlog-Start" and "Backlog-End" marker into the cloud stream to label the redeemed data segment.

#### 4. Event Bookmarking (DAW-Style Markers)
This is analogous to placing markers on a DAW timeline.
1.  **Marker Injection:** An external trigger (e.g., an API call to the APU, a physical button press) creates a "marker" event.
2.  **Timestamp & Label:** The APU packages this marker with a precise timestamp and a JSON payload (e.g., `{"event": "incident_reported", "severity": "high"}`).
3.  **DMA Injection:** This marker is treated as a high-priority data packet and is injected directly into the Shared Buffer Memory for logging by the DMA engine, just like sensor data. When retrieved, analysis software can overlay these markers on the data timeline, providing context for the surrounding sensor readings.

### The DAW Analogy: Advanced Feature Concepts

We can extend the DAW analogy to unlock powerful, real-time processing capabilities, leveraging the DPU's specific hardware.

#### **1. Sensor Channels as "Tracks"**
*   Each sensor input is a "track" in our DAW. A voice conversation is an "audio track," while a temperature sensor is an "instrument track."

#### **2. Data Mixer & Busses for Routing**
*   The NoC and DMA engine act as a sophisticated "mixer." We can create "busses" by configuring DMA channels to aggregate multiple sensor tracks (e.g., a "vibration bus" that groups all accelerometers).

#### **3. Real-time Processing "Plugins" (Leveraging the RPU)**
*   The RPU is perfect for running real-time "plugins" on our data tracks before they are logged.
    *   **Amplifier/Normalizer Plugin:** For a sensor with a very low output range, the RPU can apply a real-time gain or normalize its values to a standard range (e.g., 0.0 to 1.0) before it's sent to the DMA.
    *   **Filter/EQ Plugin:** The RPU can apply digital filters (Low-Pass, High-Pass) to a track. For a voice conversation track, it could apply a high-pass filter to remove low-frequency vehicle rumble.
    *   **Dynamics/Compressor Plugin:** For a "jerk" sensor, the RPU can "compress" the dynamic range to make sharp events more prominent in the final log.

#### **4. "Preset Racks" for Sensors**
*   A "preset rack" is a pre-configured chain of these RPU plugins for a specific sensor type. For example, a "Wind-Noise-Reduction" rack for a microphone might include a high-pass filter and a noise gate. These configurations can be loaded onto the RPU from the APU.

#### **5. "Automation" for Dynamic Control**
*   In a DAW, you can automate volume, panning, etc. Here, we can automate sensor parameters. An "incident" marker could trigger an automation event that instructs the RPU to increase the sample rate (the "track's resolution") for all sensors on the "vibration bus."

#### **6. "Sidechaining" for Correlated Analysis**
*   This is a key DAW technique. We can use one sensor track to control a plugin on another.
    *   **Example:** The output of a "shock" sensor (accelerometer) could be "sidechained" to the voice conversation track. When a large shock is detected, it could trigger a "ducker" plugin on the voice track, slightly lowering its volume to prevent clipping from a driver's exclamation.

#### **7. "Takes & Comping" for Redundancy and Smart Fusion**
*   If you have two redundant temperature sensors ("two takes" of the same performance), the RPU can "comp" them into a single, more reliable track. It can compare the two inputs in real-time and discard anomalous readings from one, or average them to create a smoother, composite track. This is a more advanced version of the malfunction detection.

### Channel State and Data Management

This extends the DAW analogy to how data streams ("tracks") are managed, recorded, and stored, focusing on efficiency and control.

*   **Hierarchical State Flags (On/Off/Record):** Instead of wastefully logging zero values for inactive sensors, the system uses state-change markers. This is a core principle for data storage efficiency.
    *   **Master Record Flag:** A global `DEVICE_RECORD_ENABLE` flag acts as the main "Record" button for the entire DPU. If this is off, no data is processed or logged.
    *   **Channel Active Flags:** Each sensor "track" has its own `CHANNEL_ON` / `CHANNEL_OFF` state. When a sensor is turned off, a single `CHANNEL_OFF` marker is written to the log. No further data is recorded for that channel until a `CHANNEL_ON` marker appears. This is far more efficient than storing null data.

*   **Channel Freezing (Mute/Solo/Freeze Analogy):**
    *   A channel can be temporarily "Frozen" by an external request or internal logic. This is equivalent to the "Freeze" or "Mute" function in a DAW.
    *   When frozen, no new data from that sensor is written to the log. A `CHANNEL_FREEZE_START` and `CHANNEL_FREEZE_END` marker is logged instead. This is useful for temporarily ignoring a sensor during specific events without losing its configured state.

*   **Adaptive Precision (Bit-Depth Reduction):**
    *   To further optimize storage, the system can dynamically change the "bit-depth" of the sensor data. The APU can configure the RPU to convert sensor readings to a lower precision format if full precision is not required.
    *   **Example:** A temperature sensor might be recorded at 32-bit float (FP32) precision by default, but for long-term archival or during low-variance periods, the system can reduce it to 16-bit float (FP16), halving the storage requirement for that sensor's data. This is analogous to an audio engineer choosing between 24-bit and 16-bit audio for different tracks.

    ## Interactive terminal behavior

    When running the program headless on a Raspberry Pi (via SSH) the runtime provides a live-updating terminal display for sensor channels. The implementation reserves a small block at the top of the terminal for the baseline channels (the first 4 channels — Unused_0..Unused_3) and updates that region in-place using ANSI cursor movement sequences.

    As you dynamically add real sensors at runtime (via the interactive command interface), new channel lines appear below the baseline block and continue to update in-place. The program keeps a prompt area below the channel display so you can type commands while the simulation runs. Currently entered commands are echoed and recorded as event markers; custom command parsing will be added per your spec.

    Notes about terminal behavior:
    - The display uses ANSI escape sequences and works best in Unix-like terminals (ssh, xterm, gnome-terminal). Modern Windows PowerShell/Windows Terminal supports ANSI codes but may produce small visual artifacts when mixed with other stdout logging.
    - To reduce output collisions, consider running in the quieter mode (use the `-q` flag in `main` if implemented) or redirecting verbose logs to a file.

    ## Running on Raspberry Pi (ARM/Linux)

    Windows `.exe` files will not run on Raspberry Pi. Build the project on the Pi or cross-compile for ARM. Example build steps on Raspberry Pi OS:

    ```bash
    sudo apt update
    sudo apt install build-essential
    gcc -Wall -Wextra -std=c11 -O2 event_queue.c memory.c zstd_accelerator.c \
        dma_engine.c nvme_controller.c ethernet_mac.c bus_interconnect.c \
        soc_core.c main.c -o blackbox_dpu
    ```

    Then run the program over an SSH session. The top-of-terminal sensor display will remain visible and update in-place; type commands below the display to interact.
