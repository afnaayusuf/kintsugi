# Kintsugi BlackBox DPU# BlackBox DPU-Inspired Preprocessing Driver



> A complete C-model virtual platform for a Data Processing Unit (DPU) inspired preprocessing driver with heterogeneous SoC architecture, hardware acceleration, and dual-path logging.## Overview



[![License](https://img.shields.io/badge/license-MIT-blue.svg)]()This project implements a **complete C-model virtual platform** for a BlackBox DPU (Data Processing Unit) inspired preprocessing driver. It demonstrates a heterogeneous SoC architecture with hardware acceleration, multi-channel DMA, and dual-path logging capabilities.

[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Raspberry%20Pi-lightgrey)]()

[![Language](https://img.shields.io/badge/language-C11-orange)]()**Two operational modes:**

1. **Test Suite Mode** (default): Runs automated tests validating the full architecture

---2. **Interactive Dashboard Mode** (`--interactive`): Live sensor channel display with command input



## 📋 Table of Contents## Architecture Highlights



- [Overview](#overview)### Core Features

- [Key Features](#key-features)- **Heterogeneous Multi-Core Processing**: APU (Application Processing Unit) + RPU (Real-time Processing Unit)

- [Architecture](#architecture)- **Hardware Zstandard Compression Accelerator**: Dedicated compression engine

- [Quick Start](#quick-start)- **Multi-Channel DMA Engine**: 4-channel DMA with fan-out/tee capability for simultaneous data distribution

- [Building & Running](#building--running)- **Network-on-Chip (NoC) Interconnect**: High-speed data routing between subsystems

- [Project Structure](#project-structure)- **Dual-Path Logging**: Simultaneous data logging to NVMe storage and Ethernet (cloud)

- [Operational Modes](#operational-modes)- **Event-Driven Simulation**: Accurate timing model for hardware behavior

- [Test Suite](#test-suite)

- [Advanced Concepts](#advanced-concepts)### Memory Map

- [Security Model](#security-model)

- [Performance](#performance)| Component | Base Address | Size | Description |

- [Hardware Integration](#hardware-integration)|-----------|-------------|------|-------------|

- [Troubleshooting](#troubleshooting)| Boot ROM | 0x00000000 | 128 KB | System boot code |

- [Contributing](#contributing)| Shared Buffer Memory (SBM) | 0x04000000 | 4 MB | Inter-processor data exchange |

| APU L2 Cache | 0x08000000 | 1 MB | Application processor cache |

---| RPU TCM | 0x08100000 | 64 KB | Real-time processor tightly-coupled memory |

| DRAM | 0x80000000 | 512 MB | Main system memory |

## 🎯 Overview| Zstd Accelerator | 0xFF800000 | 4 KB | Compression engine registers |

| DMA Controller | 0xFF810000 | 4 KB | DMA engine registers |

**Kintsugi BlackBox DPU** implements a complete event-driven virtual platform that models a sophisticated Data Processing Unit for real-time sensor data acquisition, compression, and dual-path logging. The architecture demonstrates heterogeneous multi-core processing, hardware acceleration, and a local-first security model suitable for automotive black boxes, industrial IoT, and embedded data logging systems.| PCIe/NVMe | 0xFF900000 | 1 MB | NVMe controller registers |

| Ethernet MAC | 0xFFA00000 | 1 MB | Network controller registers |

### What Makes This Unique?

## Project Structure

- **Dual-Path Logging**: Simultaneous local (NVMe) and cloud (Ethernet) data storage with hardware DMA fan-out

- **Event-Driven Simulation**: Accurate timing model for realistic hardware behavior```

- **Interactive Dashboard**: Live sensor channel monitoring with ANSI terminal UIMMIT/

- **Local-First Security**: Query-based cloud transfer with key validation and controller permission├── blackbox_common.h      # Common definitions, memory map, data structures

- **Zero-Copy DMA**: Direct memory-to-memory transfers without CPU involvement├── event_queue.h/c        # Event-driven simulation engine

- **DAW-Inspired Channel Management**: Treat sensors like audio tracks with mixing, routing, and processing├── memory.h/c             # Memory subsystem model

├── zstd_accelerator.h/c   # Hardware compression accelerator

---├── dma_engine.h/c         # Multi-channel DMA controller

├── nvme_controller.h/c    # NVMe storage controller

## ✨ Key Features├── ethernet_mac.h/c       # Ethernet MAC controller

├── bus_interconnect.h/c   # NoC interconnect and bus transactions

### Core Capabilities├── soc_core.h/c           # High-level SoC orchestration

├── main.c                 # Testbench and demonstration code

| Feature | Description |├── Makefile               # Build configuration

|---------|-------------|└── README.md              # This file

| **Heterogeneous Processing** | APU (Application) + RPU (Real-time) dual-core architecture |```

| **Hardware Compression** | Zstandard accelerator with configurable levels |

| **Multi-Channel DMA** | 4-channel engine with fan-out/tee capability |## Module Descriptions

| **Network-on-Chip** | High-speed interconnect with statistics tracking |

| **Event Markers** | DAW-style bookmarks for incident tagging |### 1. **Event Queue** (`event_queue.h/c`)

| **Sensor Health Scoring** | Multi-factor malfunction detection (0.0-1.0) |- Time-ordered priority queue for event-driven simulation

| **Adaptive Precision** | Dynamic bit-depth reduction for storage optimization |- Schedules hardware completion callbacks

| **Interactive UI** | Live updating terminal dashboard with command prompt |- Manages simulation time advancement



---### 2. **Memory Subsystem** (`memory.h/c`)

- Models all addressable memory regions

## 🏗️ Architecture- Address translation for different memory regions

- Allocates and manages virtual memory buffers

### System Block Diagram

### 3. **Zstandard Accelerator** (`zstd_accelerator.h/c`)

```- Hardware compression engine model

┌─────────────────────────────────────────────────────────────┐- Simplified Run-Length Encoding (RLE) compression for demonstration

│                        BlackBox DPU SoC                      │- Configurable compression levels

├─────────────────────────────────────────────────────────────┤- Latency modeling: ~100ns per byte × compression level

│                                                               │

│  ┌──────────┐        ┌──────────┐                           │### 4. **DMA Engine** (`dma_engine.h/c`)

│  │   APU    │◄──────►│   RPU    │                           │- 4-channel DMA controller

│  │ (App)    │        │ (RT)     │                           │- **Fan-out/Tee Mode**: Simultaneous copy to two destinations (dual-path logging)

│  └────┬─────┘        └────┬─────┘                           │- Zero-copy data movement

│       │                   │                                  │- Latency modeling: ~10ns per byte

│       └───────┬───────────┘                                  │

│               │                                              │### 5. **NVMe Controller** (`nvme_controller.h/c`)

│       ┌───────▼────────────────────┐                        │- PCIe/NVMe storage interface

│       │  Network-on-Chip (NoC)     │                        │- File-backed virtual storage (`nvme_storage.bin`)

│       │      Interconnect          │                        │- Write statistics and monitoring

│       └───────┬────────────────────┘                        │

│               │                                              │### 6. **Ethernet MAC** (`ethernet_mac.h/c`)

│       ┌───────┼────────┬──────────┬──────────┐             │- Network transmission interface

│       │       │        │          │          │             │- Cloud logging simulation (`cloud_log.bin`)

│   ┌───▼───┐ ┌▼────┐ ┌─▼──────┐ ┌─▼──────┐ ┌▼────────┐    │- Packet statistics and monitoring

│   │ Zstd  │ │ DMA │ │ NVMe   │ │Ethernet│ │ Memory  │    │

│   │Accel. │ │ x4  │ │Controller│ │  MAC   │ │Subsystem│    │### 7. **Bus Interconnect** (`bus_interconnect.h/c`)

│   └───────┘ └──┬──┘ └────────┘ └────────┘ └─────────┘    │- Memory-mapped register access

│                │                                             │- NoC transaction routing

│                │ Fan-out/Tee                                 │- Statistics collection (bandwidth, transactions)

│                ├────────────┐                                │

│                │            │                                │### 8. **SoC Core** (`soc_core.h/c`)

│           ┌────▼────┐  ┌───▼────┐                          │- High-level data flow orchestration

│           │  NVMe   │  │ Cloud  │                          │- Subsystem initialization and cleanup

│           │ Storage │  │  Log   │                          │- Dual-path logging pipeline coordination

│           └─────────┘  └────────┘                          │

└─────────────────────────────────────────────────────────────┘### 9. **Main Testbench** (`main.c`)

```- Comprehensive test suite

- Performance benchmarks

### Memory Map- Architecture validation tests



| Component | Base Address | Size | Description |## Building the Project

|-----------|-------------|------|-------------|

| Boot ROM | `0x00000000` | 128 KB | System boot code |### Prerequisites

| Shared Buffer (SBM) | `0x04000000` | 4 MB | Inter-processor data exchange |- GCC or Clang C compiler

| APU L2 Cache | `0x08000000` | 1 MB | Application processor cache |- Make (GNU Make)

| RPU TCM | `0x08100000` | 64 KB | Real-time tightly-coupled memory |- Standard C library

| DRAM | `0x80000000` | 512 MB | Main system memory |

| Zstd Accelerator | `0xFF800000` | 4 KB | Compression registers |### Compilation

| DMA Controller | `0xFF810000` | 4 KB | DMA engine registers |

| NVMe Controller | `0xFF900000` | 1 MB | PCIe/NVMe registers |```bash

| Ethernet MAC | `0xFFA00000` | 1 MB | Network controller registers |make

```

---

This will compile all modules and create the executable `blackbox_dpu`.

## 🚀 Quick Start

### Running

### Prerequisites

**Test Suite Mode (default):**

- **Linux/Raspberry Pi**: GCC 7.5+, Make, POSIX-compliant shell```bash

- **SSH Access** (for Pi): Terminal with ANSI support# Run with verbose output (default)

./blackbox_dpu

### One-Line Install & Run

# Run quietly (less output)

```bash./blackbox_dpu -q

# Clone, build, and launch interactive dashboard

git clone https://github.com/afnaayusuf/Kintsugi.git && cd Kintsugi && make && ./blackbox_dpu --interactive# Show help

```./blackbox_dpu --help

```

---

**Interactive Dashboard Mode:**

## 🔧 Building & Running```bash

# Launch live sensor display with command prompt

### Compilation./blackbox_dpu --interactive

# or

```bash./blackbox_dpu -i

# Standard build```

make

In interactive mode you'll see:

# Clean build- Live updating display of 4 sensor channels (health, state)

make clean && make- Command prompt at the bottom

- Available commands: `add <name>`, `list`, `help`

# Direct GCC compilation (fallback)- Press Ctrl-C to exit

gcc -Wall -Wextra -std=c11 -O2 event_queue.c memory.c zstd_accelerator.c \

    dma_engine.c nvme_controller.c ethernet_mac.c bus_interconnect.c \## Test Suite

    soc_core.c main.c -o blackbox_dpu

```The testbench includes five comprehensive tests:



### Running the Application### Test 1: Single Data Block Processing

- Processes a 64KB data block

#### Test Suite Mode (Default)- Demonstrates the local logging pipeline

- Verifies compression, DMA, and NVMe operations

```bash

# Full verbose output### Test 2: Continuous Streaming Data Processing

./blackbox_dpu- Processes 5 blocks of 32KB each

- Tests sustained throughput for local logging

# Quiet mode (minimal output)- Validates memory management

./blackbox_dpu -q

### Test 3: Performance Benchmark

# Show help- Tests multiple data sizes (4KB to 256KB)

./blackbox_dpu --help- Measures compression ratios

```- Calculates latency per operation



#### Interactive Dashboard Mode### Test 4: Architecture Validation

- Memory map validation

```bash- Hardware accelerator status checks

# Launch live sensor display

./blackbox_dpu --interactive### Test 5: Query-Based Cloud Transfer

# or- Demonstrates the on-demand cloud data transfer

./blackbox_dpu -i- Verifies the marker key validation and controller permission flow

```

## Key Concepts Demonstrated

**Interactive Mode Features:**

- Live 4-channel sensor display (updates in-place)### 1. **Local-First Logging and Query-Based Cloud Transfer**

- Command prompt at bottom of terminalData is first written to:

- Available commands: `add <name>`, `list`, `help`- **Local NVMe Storage**: For immediate access and redundancy.

- Press `Ctrl-C` to exit

Data is transferred to the cloud only upon a valid, authorized request:

#### Running in tmux (Recommended for SSH)- **Cloud via Ethernet**: For remote backup and analysis.



```bashThis on-demand transfer requires a valid key from a `marker.key` file and simulated permission from the controller, ensuring a secure, local-first data security model.

# Start persistent session

tmux new -s kintsugi### 2. **Hardware Acceleration**

./blackbox_dpu --interactiveInstead of CPU-based compression, the Zstd hardware accelerator:

- Reduces CPU load

# Detach: Ctrl+b then d- Provides consistent, predictable latency

# Reattach later: tmux attach -t kintsugi- Enables parallel processing while compression occurs

```

### 3. **Event-Driven Simulation**

#### Background Mode with LoggingModels realistic hardware behavior:

- Asynchronous operations

```bash- Hardware completion callbacks

# Run as daemon- Accurate timing simulation

nohup ./blackbox_dpu > blackbox.log 2>&1 &

### 4. **Zero-Copy Data Movement**

# Monitor outputDMA engine moves data directly between memory regions without CPU involvement, reducing:

tail -f blackbox.log- Memory bandwidth consumption

- CPU overhead

# Check process- Latency

ps aux | grep blackbox_dpu

```## Output Files



---After running, the following files are generated:



## 📁 Project Structure- **`nvme_storage.bin`**: Simulated NVMe storage contents (compressed data).

- **`cloud_log.bin`**: Simulated cloud transmission data. This file is only created after a successful, authorized cloud transfer request and will not be identical to the NVMe storage.

```

Kintsugi/## Performance Characteristics

├── blackbox_common.h       # Core definitions, memory map, structs

├── event_queue.h/c         # Event-driven simulation engine### Typical Results

├── memory.h/c              # Memory subsystem with bounds checking- **Compression Ratio**: 30-50% (depending on data pattern)

├── zstd_accelerator.h/c    # Hardware compression accelerator- **Zstd Latency**: ~100ns/byte × compression level

├── dma_engine.h/c          # 4-channel DMA with fan-out- **DMA Latency**: ~10ns/byte

├── nvme_controller.h/c     # NVMe storage interface- **Dual-Path Overhead**: Minimal (fan-out in hardware)

├── ethernet_mac.h/c        # Ethernet MAC controller

├── bus_interconnect.h/c    # NoC interconnect & bus transactions## Use Cases

├── soc_core.h/c            # High-level SoC orchestration

├── main.c                  # Testbench & interactive modeThis architecture is suitable for:

├── Makefile                # Build configuration1. **Automotive Black Box Systems**: Continuous sensor data logging with redundancy

└── README.md               # This file2. **Industrial IoT**: Real-time data collection with local and cloud storage

```3. **Network Monitoring**: Packet capture with simultaneous local and remote storage

4. **Embedded Systems**: Any application requiring reliable, redundant data logging

### Module Descriptions

## Security Model: Local-First Configuration

| Module | Responsibility |

|--------|---------------|To ensure safety, security, and data integrity, the DPU operates on a strict **local-first configuration policy**. This principle dictates that the DPU's core operational parameters cannot be modified by remote commands from the cloud by default.

| **event_queue** | Time-ordered priority queue, hardware completion callbacks |

| **memory** | Address translation, region management, bounds checking |*   **Read-Only Remote Access:** The cloud interface is primarily for data retrieval (querying logs) and monitoring status. It is treated as an untrusted source for configuration changes.

| **zstd_accelerator** | RLE compression model, latency simulation (~100ns/byte) |*   **Preventing Unauthorized Tampering:** This model prevents a remote attacker or an accidental command from disabling critical sensors, stopping logging, or altering data processing routines. In a vehicle, for example, a remote command should not be able to stop the black box from recording.

| **dma_engine** | Zero-copy transfers, fan-out routing, latency ~10ns/byte |*   **Mechanism for Authorized Changes:** Configuration changes require explicit local authorization, which can be granted via:

| **nvme_controller** | File-backed storage (`nvme_storage.bin`) |    *   **Authenticated Local Input:** A trusted local entity, such as the vehicle's main ECU or an authenticated action by the operator (e.g., a pilot or driver), can issue a signed token or command that permits a configuration change for a limited time.

| **ethernet_mac** | Cloud logging simulation (`cloud_log.bin`) |    *   **Direct Physical Access:** For major updates or maintenance, a direct, authenticated connection to a local maintenance port is required.

| **bus_interconnect** | Memory-mapped I/O, NoC statistics |

| **soc_core** | Sensor management, display, command handling |The APU acts as the gatekeeper, validating all incoming configuration requests and rejecting any that do not originate from or get explicitly authorized by a trusted local source.

| **main** | Test suite, interactive dashboard |

## Driver Concept

---

This implementation serves as a **virtual platform** that models what a real hardware driver would do:

## 🎛️ Operational Modes

### In Simulation (Current)

### 1. Test Suite Mode- Models hardware behavior in software

- Perfect for algorithm development and testing

Runs five comprehensive validation tests:- No actual hardware required



| Test | Description | Validates |### In Production (Real Driver)

|------|-------------|-----------|- Would run in Linux kernel or bare-metal embedded system

| **Test 1** | Single 64KB data block | Compression, DMA, NVMe pipeline |- Direct hardware register access via memory-mapped I/O

| **Test 2** | Streaming (5×32KB blocks) | Sustained throughput, memory mgmt |- Interrupt handling for hardware events

| **Test 3** | Performance benchmark | Compression ratios (4KB-256KB) |- Integration with OS I/O subsystem

| **Test 4** | Architecture validation | Memory map, hardware status |

| **Test 5** | Query-based cloud transfer | Key validation, permission flow |## Future Enhancements



**Output Files:**Potential improvements:

- `nvme_storage.bin` — Local compressed storage- [ ] Real Zstandard compression library integration

- `cloud_log.bin` — Cloud transmission data (query-triggered)- [ ] PCIe link simulation

- [ ] Network protocol stack (TCP/IP)

### 2. Interactive Dashboard Mode- [ ] Power management model

- [ ] Error injection and recovery testing

**Display Format:**- [ ] Multi-threaded processing simulation

```- [ ] GUI for visualization

============================================================

     BlackBox DPU - Interactive Dashboard Mode             ## Contributing

============================================================

Commands: add <name>, list, help, quitThis is a demonstration project for understanding DPU architecture and driver development concepts.

(Live sensor display updates automatically)

## References

Sensor Channels:

(Press Ctrl-C to interrupt)- **Zstandard Compression**: https://github.com/facebook/zstd

  CH0 [IMU_Accelerometer ]:   ON  (Health:   98.5%)- **NVMe Specification**: https://nvmexpress.org/

  CH1 [Temperature_Cabin ]:   ON  (Health:  100.0%)- **DMA Concepts**: Various embedded systems documentation

  CH2 [GPS_Module        ]: FROZEN(Health:   45.2%)- **Network-on-Chip**: Academic research papers on SoC interconnects

  CH3 [Microphone_Front  ]:  REC  (Health:   99.8%)

## License

> add Pressure_Sensor_Rear

[CMD] add Pressure_Sensor_RearEducational and demonstration purposes.

Added sensor 'Pressure_Sensor_Rear' as channel 4

## Contact

> _

```For questions about this architecture implementation, refer to the project documentation.



**Command Reference:**---



| Command | Example | Description |**Note**: This is a simulation/virtual platform. No actual hardware is accessed. All operations are modeled in software for educational and development purposes.

|---------|---------|-------------|

| `add <name>` | `add IMU_Sensor` | Add new sensor channel dynamically |## Advanced Features & DAW Analogy

| `list` | `list` | Show all configured channels |

| `help` | `help` | Display available commands |This section outlines advanced operational routines and expands the system concept using the analogy of a Digital Audio Workstation (DAW), which is highly appropriate for a multi-channel, time-series data processing architecture like this DPU.

| `quit` | `quit` | Exit (or Ctrl-C) |

### Core Operational Routines

---

#### 1. Routine for Timestamp-Specific Data Queries

## 🧪 Test SuiteWhen a query arrives for data from a specific sensor at a given timestamp, the APU (Application Processing Unit) executes the following routine:

1.  **Log Indexing:** The system maintains a separate index file or a metadata header in each log chunk. This index maps time ranges to file offsets in the `nvme_storage.bin`.

### Test Execution Flow2.  **Seek & Retrieve:** The APU uses the index to perform a fast seek to the relevant data block, avoiding a full scan of the storage file.

3.  **Decompression & Filtering:** The target block is read from NVMe into DRAM and decompressed by the Zstd accelerator.

```4.  **Micro-Scan:** The APU then scans the small, decompressed block to pinpoint the exact timestamp and sensor ID, returning the requested data value.

Initialize SoC

    ↓#### 2. Solution for Flagging Malfunctioning Sensors

Test 1: Single Block (64KB)Flagging a sensor malfunction requires a high degree of confidence. This is handled by the RPU (Real-time Processing Unit) running a dedicated "Sensor Health" task.

    → Generate data → Compress → DMA → NVMe1.  **Multi-Factor Justification:** A sensor is flagged only if multiple conditions are met simultaneously. The RPU analyzes incoming data for:

    ↓    *   **Stagnation:** The sensor value has not changed for a statistically improbable duration.

Test 2: Streaming (5×32KB)    *   **Out-of-Bounds:** The value is physically impossible (e.g., -100°C).

    → Loop blocks → Validate memory    *   **Anomalous Jitter:** The value is fluctuating in a way that violates its known physical constraints or noise profile.

    ↓    *   **Cross-Sensor Discrepancy:** The sensor's value significantly deviates from a group of adjacent, correlated sensors.

Test 3: Performance Benchmark2.  **Confidence Score:** Instead of a binary flag, the RPU generates a "health score" (0.0 to 1.0). A score below a configurable threshold (e.g., 0.3) marks the data stream as "unreliable."

    → 4KB, 16KB, 64KB, 256KB → Measure ratios3.  **Event Flagging:** When a sensor is flagged, a special "SensorUnreliable" event is injected into the data stream via the DMA, time-stamped like any other data point.

    ↓

Test 4: Architecture Validation#### 3. Network Backlog and Data Redemption

    → Memory map → Hardware statusThe dual-path logging architecture is ideal for this.

    ↓1.  **Local First:** All data is unconditionally written to the local `nvme_storage.bin`. This is the source of truth.

Test 5: Cloud Transfer2.  **Cloud Sync Watermark:** The Ethernet MAC controller (or a process on the APU managing it) maintains a "high watermark" of the last timestamp successfully transmitted to the cloud. This is stored persistently.

    → Invalid key → FAIL3.  **Connection Loss:** If the network connection is lost, the Ethernet path simply stops consuming data. The local NVMe logging continues uninterrupted.

    → Valid key   → SUCCESS (Ethernet transmission)4.  **Re-establishment:** When the connection returns, the cloud sync process reads the watermark, seeks to that position in the `nvme_storage.bin`, and begins streaming the "lost" data. It injects a "Backlog-Start" and "Backlog-End" marker into the cloud stream to label the redeemed data segment.

    ↓

Statistics Report & Cleanup#### 4. Event Bookmarking (DAW-Style Markers)

```This is analogous to placing markers on a DAW timeline.

1.  **Marker Injection:** An external trigger (e.g., an API call to the APU, a physical button press) creates a "marker" event.

### Expected Results2.  **Timestamp & Label:** The APU packages this marker with a precise timestamp and a JSON payload (e.g., `{"event": "incident_reported", "severity": "high"}`).

3.  **DMA Injection:** This marker is treated as a high-priority data packet and is injected directly into the Shared Buffer Memory for logging by the DMA engine, just like sensor data. When retrieved, analysis software can overlay these markers on the data timeline, providing context for the surrounding sensor readings.

```

Compression Statistics:### The DAW Analogy: Advanced Feature Concepts

  Total input data:     262144 bytes

  Compressed output:    81826 bytesWe can extend the DAW analogy to unlock powerful, real-time processing capabilities, leveraging the DPU's specific hardware.

  Compression ratio:    31.21%

#### **1. Sensor Channels as "Tracks"**

Storage Path (NVMe):*   Each sensor input is a "track" in our DAW. A voice conversation is an "audio track," while a temperature sensor is an "instrument track."

  Total writes:         10

  Total bytes written:  294223 bytes#### **2. Data Mixer & Busses for Routing**

*   The NoC and DMA engine act as a sophisticated "mixer." We can create "busses" by configuring DMA channels to aggregate multiple sensor tracks (e.g., a "vibration bus" that groups all accelerometers).

Cloud Path (Ethernet):

  Connection status:    Disconnected#### **3. Real-time Processing "Plugins" (Leveraging the RPU)**

  Total packets:        1*   The RPU is perfect for running real-time "plugins" on our data tracks before they are logged.

  Total bytes sent:     5102 bytes (query-based)    *   **Amplifier/Normalizer Plugin:** For a sensor with a very low output range, the RPU can apply a real-time gain or normalize its values to a standard range (e.g., 0.0 to 1.0) before it's sent to the DMA.

```    *   **Filter/EQ Plugin:** The RPU can apply digital filters (Low-Pass, High-Pass) to a track. For a voice conversation track, it could apply a high-pass filter to remove low-frequency vehicle rumble.

    *   **Dynamics/Compressor Plugin:** For a "jerk" sensor, the RPU can "compress" the dynamic range to make sharp events more prominent in the final log.

---

#### **4. "Preset Racks" for Sensors**

## 🎓 Advanced Concepts*   A "preset rack" is a pre-configured chain of these RPU plugins for a specific sensor type. For example, a "Wind-Noise-Reduction" rack for a microphone might include a high-pass filter and a noise gate. These configurations can be loaded onto the RPU from the APU.



### DAW-Inspired Sensor Management#### **5. "Automation" for Dynamic Control**

*   In a DAW, you can automate volume, panning, etc. Here, we can automate sensor parameters. An "incident" marker could trigger an automation event that instructs the RPU to increase the sample rate (the "track's resolution") for all sensors on the "vibration bus."

Treat sensor channels like audio tracks in a Digital Audio Workstation:

#### **6. "Sidechaining" for Correlated Analysis**

#### 1. **Channels as Tracks***   This is a key DAW technique. We can use one sensor track to control a plugin on another.

Each sensor is a "track" with state, health, and data flow.    *   **Example:** The output of a "shock" sensor (accelerometer) could be "sidechained" to the voice conversation track. When a large shock is detected, it could trigger a "ducker" plugin on the voice track, slightly lowering its volume to prevent clipping from a driver's exclamation.



#### 2. **Mixer & Busses**#### **7. "Takes & Comping" for Redundancy and Smart Fusion**

- DMA engine routes data like a mixer*   If you have two redundant temperature sensors ("two takes" of the same performance), the RPU can "comp" them into a single, more reliable track. It can compare the two inputs in real-time and discard anomalous readings from one, or average them to create a smoother, composite track. This is a more advanced version of the malfunction detection.

- Create "busses" (e.g., "vibration bus" aggregating accelerometers)

### Channel State and Data Management

#### 3. **Real-Time Processing "Plugins" (RPU)**

- **Amplifier/Normalizer**: Gain adjustment for low-output sensorsThis extends the DAW analogy to how data streams ("tracks") are managed, recorded, and stored, focusing on efficiency and control.

- **Filter/EQ**: Digital filtering (HPF, LPF) for noise reduction

- **Dynamics/Compressor**: Dynamic range compression for event prominence*   **Hierarchical State Flags (On/Off/Record):** Instead of wastefully logging zero values for inactive sensors, the system uses state-change markers. This is a core principle for data storage efficiency.

    *   **Master Record Flag:** A global `DEVICE_RECORD_ENABLE` flag acts as the main "Record" button for the entire DPU. If this is off, no data is processed or logged.

#### 4. **Preset Racks**    *   **Channel Active Flags:** Each sensor "track" has its own `CHANNEL_ON` / `CHANNEL_OFF` state. When a sensor is turned off, a single `CHANNEL_OFF` marker is written to the log. No further data is recorded for that channel until a `CHANNEL_ON` marker appears. This is far more efficient than storing null data.

Pre-configured RPU plugin chains (e.g., "Wind-Noise-Reduction" for microphones)

*   **Channel Freezing (Mute/Solo/Freeze Analogy):**

#### 5. **Automation**    *   A channel can be temporarily "Frozen" by an external request or internal logic. This is equivalent to the "Freeze" or "Mute" function in a DAW.

Dynamic control triggered by events (e.g., increase sample rate on incident)    *   When frozen, no new data from that sensor is written to the log. A `CHANNEL_FREEZE_START` and `CHANNEL_FREEZE_END` marker is logged instead. This is useful for temporarily ignoring a sensor during specific events without losing its configured state.



#### 6. **Sidechaining***   **Adaptive Precision (Bit-Depth Reduction):**

Use one sensor to control another (e.g., shock sensor ducks voice track)    *   To further optimize storage, the system can dynamically change the "bit-depth" of the sensor data. The APU can configure the RPU to convert sensor readings to a lower precision format if full precision is not required.

    *   **Example:** A temperature sensor might be recorded at 32-bit float (FP32) precision by default, but for long-term archival or during low-variance periods, the system can reduce it to 16-bit float (FP16), halving the storage requirement for that sensor's data. This is analogous to an audio engineer choosing between 24-bit and 16-bit audio for different tracks.

#### 7. **Takes & Comping**

Fuse redundant sensors into single reliable stream (averaging, anomaly rejection)    ## Interactive terminal behavior



### Channel State Management    When running the program headless on a Raspberry Pi (via SSH) the runtime provides a live-updating terminal display for sensor channels. The implementation reserves a small block at the top of the terminal for the baseline channels (the first 4 channels — Unused_0..Unused_3) and updates that region in-place using ANSI cursor movement sequences.



```c    As you dynamically add real sensors at runtime (via the interactive command interface), new channel lines appear below the baseline block and continue to update in-place. The program keeps a prompt area below the channel display so you can type commands while the simulation runs. Currently entered commands are echoed and recorded as event markers; custom command parsing will be added per your spec.

typedef enum {

    CHANNEL_OFF,        // Not logging (saves storage)    Notes about terminal behavior:

    CHANNEL_ON,         // Active logging    - The display uses ANSI escape sequences and works best in Unix-like terminals (ssh, xterm, gnome-terminal). Modern Windows PowerShell/Windows Terminal supports ANSI codes but may produce small visual artifacts when mixed with other stdout logging.

    CHANNEL_FROZEN,     // Temporarily muted    - To reduce output collisions, consider running in the quieter mode (use the `-q` flag in `main` if implemented) or redirecting verbose logs to a file.

    CHANNEL_RECORDING   // High-priority capture

} ChannelState;    ## Running on Raspberry Pi (ARM/Linux)

```

    Windows `.exe` files will not run on Raspberry Pi. Build the project on the Pi or cross-compile for ARM. Example build steps on Raspberry Pi OS:

**State Markers:**

- `CHANNEL_OFF` → Single marker, no further data until `CHANNEL_ON`    ```bash

- `CHANNEL_FREEZE_START` / `END` → Temporary suspension markers    sudo apt update

- Avoids wasting storage on inactive/frozen channels    sudo apt install build-essential

    gcc -Wall -Wextra -std=c11 -O2 event_queue.c memory.c zstd_accelerator.c \

### Adaptive Precision        dma_engine.c nvme_controller.c ethernet_mac.c bus_interconnect.c \

        soc_core.c main.c -o blackbox_dpu

Dynamically adjust sensor bit-depth:    ```

- **FP32 (32-bit float)**: High precision for critical sensors

- **FP16 (16-bit float)**: Storage optimization during low-variance periods    Then run the program over an SSH session. The top-of-terminal sensor display will remain visible and update in-place; type commands below the display to interact.

- **INT16 (16-bit int)**: Fixed-point for embedded efficiency

**Example:** Temperature sensor switches from FP32→FP16 during stable conditions, halving storage.

### Operational Routines

#### 1. Timestamp-Specific Queries
```
Query → Log Index → Seek NVMe → Decompress → Micro-Scan → Return Data
```
- Fast offset lookup via index file
- Avoids full storage scan

#### 2. Sensor Malfunction Detection (RPU)
Multi-factor health scoring (0.0-1.0):
- Stagnation (no value change)
- Out-of-bounds values
- Anomalous jitter
- Cross-sensor discrepancy

Flags: `SensorUnreliable` event injected at threshold (< 0.3)

#### 3. Network Backlog Redemption
```
Connection Lost → Local logging continues → NVMe watermark saved
Connection Restored → Seek watermark → Stream backlog → Inject markers
```
- `Backlog-Start` / `Backlog-End` markers label redeemed data

#### 4. Event Bookmarking
```
External Trigger → APU creates marker → DMA injects timestamp+JSON → Log
```
Example: `{"event": "incident_reported", "severity": "high"}`

---

## 🔒 Security Model

### Local-First Configuration Policy

**Core Principle:** DPU operational parameters cannot be modified remotely by default.

#### Design Goals
1. **Prevent Unauthorized Tampering**: Remote attacker cannot disable sensors or stop logging
2. **Read-Only Cloud Access**: Cloud queries data but cannot change configuration
3. **Explicit Local Authorization**: Config changes require authenticated local input

#### Authorization Mechanisms

| Method | Description |
|--------|-------------|
| **Authenticated Local Input** | Trusted ECU or operator issues signed token |
| **Physical Access** | Direct connection to maintenance port |
| **APU Gatekeeper** | Validates all requests, rejects untrusted sources |

#### Query-Based Cloud Transfer

1. **Marker Key Validation**: Load `marker.key` file, compare with request
2. **Controller Permission**: APU requests permission from controller
3. **Authorized Transfer**: If both pass, Ethernet MAC transmits data
4. **Failure Logging**: Invalid keys or denied permissions are logged

**Implementation:**
```c
// Cloud transfer requires valid key + controller permission
if (validate_marker_key(key) && controller_grant_permission()) {
    ethernet_transmit(data);
} else {
    log_transfer_failure();
}
```

---

## ⚡ Performance

### Typical Benchmarks

| Metric | Value | Notes |
|--------|-------|-------|
| **Compression Ratio** | 30-50% | Depends on data pattern |
| **Zstd Latency** | ~100ns/byte × level | Hardware accelerator model |
| **DMA Latency** | ~10ns/byte | Zero-copy transfer |
| **Dual-Path Overhead** | Minimal | Hardware fan-out (no CPU) |
| **Event Queue** | O(log n) | Priority queue insertion |
| **Memory Bounds Checks** | O(1) | Constant-time region lookup |

### Compression Results (256KB Block)

```
Input:  262144 bytes
Output:  81826 bytes
Ratio:   31.21%
Latency: ~26ms (simulated)
```

---

## 🖥️ Hardware Integration

### Current: Virtual Platform (Simulation)

- Models hardware behavior in software
- Perfect for algorithm development
- No physical hardware required
- Event-driven timing simulation

### Future: Production Driver

**Kernel Driver (Linux):**
```c
// Memory-mapped I/O
volatile uint32_t *zstd_ctrl = ioremap(ZSTD_REGS_BASE, 0x1000);
*zstd_ctrl = ZSTD_CTRL_START;

// Interrupt handling
irqreturn_t dma_complete_handler(int irq, void *dev_id) {
    // Process completion
    return IRQ_HANDLED;
}
```

**Bare-Metal (Embedded):**
```c
// Direct register access
#define ZSTD_CTRL_REG ((volatile uint32_t*)0xFF800000)
*ZSTD_CTRL_REG = 0x01;  // Start compression
```

---

## 🐛 Troubleshooting

### Build Issues

#### Error: `implicit declaration of function 'usleep'`
**Fix:**
```bash
# Ensure _POSIX_C_SOURCE is defined before includes
# Already fixed in main.c (line 6)
git pull origin master
make clean && make
```

#### Error: `file format not recognized` (event_queue.o)
**Cause:** Mixed object file architectures (Windows .exe on Pi)
**Fix:**
```bash
make clean
rm -f *.o *.exe
make
```

### Runtime Issues

#### ANSI Display Garbled on Windows
**Cause:** Windows PowerShell has limited ANSI support
**Fix:** Use interactive mode on Linux/Pi, or redirect output:
```bash
./blackbox_dpu > output.log 2>&1
```

#### SSH Session Drops, Program Stops
**Fix:** Use tmux or screen:
```bash
tmux new -s blackbox
./blackbox_dpu --interactive
# Detach: Ctrl+b d
```

#### No Output Files Generated
**Check:**
```bash
ls -lh nvme_storage.bin cloud_log.bin
# Ensure write permissions in current directory
chmod u+w .
```

---

## 🤝 Contributing

This is an educational/demonstration project. Potential enhancements:

- [ ] Integrate real Zstandard library (libzstd)
- [ ] PCIe link simulation
- [ ] TCP/IP network stack
- [ ] Power management model
- [ ] Error injection testing
- [ ] Multi-threaded processing
- [ ] Web-based GUI dashboard
- [ ] Real NVMe device integration
- [ ] CAN bus sensor interface

---

## 📚 References

- **Zstandard**: https://github.com/facebook/zstd
- **NVMe Spec**: https://nvmexpress.org/
- **DMA Concepts**: Various embedded systems documentation
- **Network-on-Chip**: Academic research on SoC interconnects

---

## 📄 License

Educational and demonstration purposes. See LICENSE file.

---

## 📞 Contact & Support

- **Repository**: https://github.com/afnaayusuf/Kintsugi
- **Issues**: Report bugs via GitHub Issues
- **Discussions**: Use GitHub Discussions for questions

---

## 🙏 Acknowledgments

Built as a demonstration of:
- Event-driven embedded systems
- Hardware/software co-design
- Real-time data processing architectures
- Secure local-first computing

---

**Note:** This is a simulation/virtual platform. No actual hardware is accessed. All operations are modeled in software for educational purposes.

---

<div align="center">
  <strong>Made with 💙 for embedded systems education</strong>
  <br>
  <sub>Kintsugi (金継ぎ) — The art of embracing imperfection and making it stronger</sub>
</div>
