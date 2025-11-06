# MMIT Live Streaming Mode Guide

## Overview

The MMIT BlackBox driver now supports **live telemetry streaming mode** that displays real-time vehicle data in your terminal while simultaneously sending it to the FastAPI backend.

## Quick Start

```bash
# Build MMIT
cd MMIT
make

# Run live streaming (60 updates = 1 minute)
./blackbox_dpu --stream

# Run for longer (300 updates = 5 minutes)
./blackbox_dpu --stream 300
```

## What You'll See

The terminal will display a live-updating dashboard like this:

```
╔══════════════════════════════════════════════════════════════════════╗
║           MMIT BLACKBOX - LIVE TELEMETRY STREAMING                  ║
║                   Vehicle: BENYON_001                                ║
╚══════════════════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────────────────┐
│ VEHICLE DYNAMICS                                                    │
├─────────────────────────────────────────────────────────────────────┤
│  Speed:            75.3 km/h    │  RPM:             2543            │
│  Throttle:         45.2 %       │  Brake:            5.1 %          │
│  Gear:                 4        │  Battery:         12.58 V         │
│  Engine Temp:      87.3 °C      │  Fuel Level:      74.2 %          │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ GPS LOCATION                                                        │
├─────────────────────────────────────────────────────────────────────┤
│  Latitude:       10.005623°     │  Longitude:      76.360542°       │
└─────────────────────────────────────────────────────────────────────┘

... (more panels for wheel speeds, environment, system status)

Press Ctrl+C to stop streaming...
```

## Features

### Real-Time Display
- ✅ **Updates every second** with fresh telemetry data
- ✅ **ANSI escape codes** clear screen and reposition cursor (no scrolling)
- ✅ **Clean tabular layout** with box-drawing characters
- ✅ **Status indicators** for ABS, traction control, etc.

### Simultaneous Backend Updates
- ✅ **HTTP POST** to backend API every second
- ✅ **JSON payload** matching `TelemetryData` model
- ✅ **Non-blocking** - display continues even if POST fails
- ✅ **Success counter** shown in System Status panel

### Realistic Driving Scenarios

The data generator creates varied, realistic scenarios that change every 10 seconds:

1. **City Driving** (0-10s)
   - Speed: 40-60 km/h
   - Moderate throttle, occasional braking
   - Gear: 3

2. **Highway Cruising** (10-20s)
   - Speed: 100+ km/h
   - Steady throttle, no braking
   - Gear: 5

3. **Acceleration** (20-30s)
   - Speed: increasing from 20 to 100 km/h
   - High throttle (80%+)
   - Gear shifts: 2 → 3 → 4 → 5

4. **Braking/Deceleration** (30-40s)
   - Speed: decreasing from 80 to 0 km/h
   - Brake: 60%
   - ABS activates when appropriate

5. **Idle/Stopped** (40-50s)
   - Speed: 0 km/h
   - RPM: 800 (idle)
   - Gear: Neutral

Then the cycle repeats!

## Usage Examples

### Basic Streaming
```bash
./blackbox_dpu --stream
# Runs for 60 seconds (60 updates at 1 Hz)
```

### Custom Duration
```bash
./blackbox_dpu --stream 300
# Runs for 5 minutes (300 updates)
```

### Continuous Streaming
```bash
./blackbox_dpu --stream 99999
# Runs until you press Ctrl+C
```

### With Background Backend
```bash
# Terminal 1: Start backend
cd code/backend
source venv/bin/activate
uvicorn main:app --host 0.0.0.0 --port 8000 &

# Terminal 2: Stream telemetry
cd MMIT
./blackbox_dpu --stream
```

## Backend Integration

### Configuration

The streaming mode uses settings from `network_config.h`:

```c
#define BACKEND_API_HOST    "localhost"
#define BACKEND_API_PORT    8000
```

### API Endpoint

Each update POSTs to:
```
POST http://localhost:8000/api/v1/telemetry/BENYON_001/update
Content-Type: application/json
```

### Verify Data Reaches Backend

```bash
# In another terminal
curl http://localhost:8000/api/v1/telemetry/BENYON_001/current
```

You should see the latest data from MMIT!

## Frontend Display

Once the backend receives data from MMIT:

1. Open frontend: `http://localhost:3000`
2. Login (admin@hybrid-drive.io / demo123)
3. Select vehicle `BENYON_001`
4. Watch the dashboard display **real data from MMIT**!

The frontend will show:
- ✅ Real-time speed, RPM, throttle, brake
- ✅ GPS location on map
- ✅ Wheel speeds
- ✅ Environmental conditions
- ✅ System health stats
- ✅ Safety alerts (ABS, traction control)

## Command Line Options

```bash
./blackbox_dpu [options]

Options:
  -s, --stream [count]    Live telemetry streaming mode
                          Default: 60 updates (1 minute)
  -i, --interactive       Interactive sensor dashboard
  -q, --quiet             Quiet mode (less output)
  -h, --help              Show help message

Examples:
  ./blackbox_dpu                  # Run full test suite
  ./blackbox_dpu --stream         # Stream 60 updates
  ./blackbox_dpu --stream 120     # Stream 2 minutes
  ./blackbox_dpu --interactive    # Interactive mode
```

## System Requirements

- **Terminal**: Must support ANSI escape codes
  - ✅ Linux/Mac terminals
  - ✅ Windows Terminal
  - ✅ PuTTY, SecureCRT (SSH)
  - ❌ Old Windows CMD (use PowerShell or Windows Terminal)

- **Backend**: FastAPI server running on port 8000

- **Network**: If backend is on different machine, update `BACKEND_API_HOST` in `network_config.h`

## Troubleshooting

### Display is garbled or scrolling
- Your terminal may not support ANSI escape codes
- Try using: Windows Terminal, modern SSH client, or run on Pi directly

### "Failed to initialize telemetry sender"
- Check backend is running: `curl http://localhost:8000/`
- Verify network settings in `network_config.h`

### No data in frontend
- Verify backend received data: `curl http://localhost:8000/api/v1/telemetry/BENYON_001/current`
- Check frontend `.env.local` points to correct backend URL
- Check browser console for errors

### High CPU usage
- This is normal - updating display + sending HTTP requests every second
- On Raspberry Pi 4, expect 20-30% CPU usage

## Performance

- **Update Rate**: 1 Hz (1 update per second)
- **Display Latency**: < 10ms (ANSI terminal updates)
- **Network Latency**: 30-100ms (HTTP POST to backend)
- **Total Latency**: < 150ms (MMIT → Backend → Frontend)

## Advanced Usage

### Run as Background Service

For continuous operation, use systemd (see INTEGRATION_GUIDE.md):

```bash
sudo systemctl start mmit-streaming
```

### Custom Update Rate

To change update frequency, modify `main.c`:

```c
// Change sleep(1) to sleep(2) for 0.5 Hz
sleep(2);  // 2 seconds between updates
```

### Multiple Vehicles

Run multiple instances with different vehicle IDs:

```bash
# Modify telemetry_sender.c to accept vehicle ID as parameter
./blackbox_dpu --stream --vehicle BENYON_002
```

## Next Steps

1. ✅ **See live data** in terminal
2. ✅ **Verify backend** receives updates
3. ✅ **View in frontend** dashboard
4. 🔄 **Connect real sensors** (replace simulation in `mmit_sensors_to_telemetry()`)
5. 🔄 **Add more vehicles** for fleet monitoring
6. 🔄 **Store to database** for historical analysis

---

**Enjoy streaming your telemetry data!** 🚗💨📡
