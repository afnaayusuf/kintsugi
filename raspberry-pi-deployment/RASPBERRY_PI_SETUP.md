# Kintsugi Raspberry Pi Deployment Guide

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Your Laptop                               │
│                                                              │
│  Frontend (Next.js)                                         │
│  http://localhost:3000  ──────────────────┐                │
│                                            │                │
└────────────────────────────────────────────┼────────────────┘
                                             │
                                             │ HTTP/WebSocket
                                             │
┌────────────────────────────────────────────▼────────────────┐
│               Raspberry Pi (192.168.x.x)                     │
│                                                              │
│  ┌──────────────────────┐    ┌─────────────────────┐       │
│  │  MMIT Driver (C)     │───▶│  Backend (Python)   │       │
│  │  - Vehicle Sim       │    │  - FastAPI          │       │
│  │  - Telemetry Gen     │    │  - WebSocket        │       │
│  │  - Real-time Data    │    │  - REST API         │       │
│  └──────────────────────┘    │  Port: 8000         │       │
│                               └─────────────────────┘       │
└─────────────────────────────────────────────────────────────┘
```

## Prerequisites

- Raspberry Pi 4 (2GB RAM or more recommended)
- Raspbian/Raspberry Pi OS (Bullseye or later)
- Internet connection
- SSH access to your Pi

## Quick Setup (Automated)

### On Raspberry Pi:

```bash
# Clone the repository
cd ~
git clone https://github.com/afnaayusuf/kintsugi.git
cd kintsugi

# Run automated setup
chmod +x raspberry-pi-deployment/setup_raspberry_pi.sh
./raspberry-pi-deployment/setup_raspberry_pi.sh

# Start services
sudo systemctl start kintsugi-backend
sudo systemctl start kintsugi-mmit

# Check status
sudo systemctl status kintsugi-backend
sudo systemctl status kintsugi-mmit
```

### On Your Laptop:

1. **Get Raspberry Pi IP address:**
   ```bash
   # On Pi, run:
   hostname -I
   # Example output: 192.168.1.100
   ```

2. **Update frontend configuration:**
   
   Edit `lib/api.ts` in your project:
   ```typescript
   const API_BASE_URL = process.env.NEXT_PUBLIC_API_URL || 'http://192.168.1.100:8000';
   ```

3. **Start frontend:**
   ```bash
   npm install --legacy-peer-deps
   npm run dev
   ```

4. **Access dashboard:**
   Open http://localhost:3000

## Manual Setup (Step by Step)

### Step 1: Install Dependencies on Raspberry Pi

```bash
sudo apt-get update
sudo apt-get install -y python3 python3-pip python3-venv gcc make libcurl4-openssl-dev git
```

### Step 2: Clone Repository

```bash
cd ~
git clone https://github.com/afnaayusuf/kintsugi.git
cd kintsugi
```

### Step 3: Setup Backend

```bash
cd backend

# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt

# Test backend
uvicorn main:app --host 0.0.0.0 --port 8000
```

Verify at: http://YOUR_PI_IP:8000/health

### Step 4: Build MMIT Driver

```bash
cd ~/kintsugi/MMIT

# Build the driver
make clean
make

# Test MMIT
./blackbox_dpu --help
```

### Step 5: Run System

#### Option A: Quick Start (Foreground)

```bash
cd ~/kintsugi
chmod +x raspberry-pi-deployment/start_system.sh
./raspberry-pi-deployment/start_system.sh
```

#### Option B: Using systemd (Background Services)

```bash
# Enable services
sudo systemctl enable kintsugi-backend
sudo systemctl enable kintsugi-mmit

# Start services
sudo systemctl start kintsugi-backend
sudo systemctl start kintsugi-mmit

# Check logs
sudo journalctl -u kintsugi-backend -f
sudo journalctl -u kintsugi-mmit -f
```

## MMIT Driver Commands

### Basic Usage

```bash
cd ~/kintsugi/MMIT

# Run test suite (default mode)
./blackbox_dpu

# Run interactive dashboard mode
./blackbox_dpu --interactive

# Send telemetry to backend
./blackbox_dpu --interactive --backend http://localhost:8000

# Continuous simulation mode
./blackbox_dpu --stream --backend http://localhost:8000
```

### Command Line Options

- `--interactive` - Run interactive dashboard with live sensor display
- `--backend <url>` - Send telemetry to backend (default: http://localhost:8000)
- `--stream` - Continuous streaming mode
- `--help` - Show help message

## Frontend Configuration

### Update API Endpoint

1. Edit `lib/api.ts`:
   ```typescript
   const API_BASE_URL = process.env.NEXT_PUBLIC_API_URL || 'http://YOUR_PI_IP:8000';
   ```

2. Or use environment variable:
   ```bash
   # Create .env.local
   echo "NEXT_PUBLIC_API_URL=http://192.168.1.100:8000" > .env.local
   ```

3. Update WebSocket connection in `lib/websocket.ts`:
   ```typescript
   const WS_URL = process.env.NEXT_PUBLIC_WS_URL || 'ws://YOUR_PI_IP:8000';
   ```

## Testing the Setup

### 1. Test Backend

```bash
curl http://YOUR_PI_IP:8000/health
# Expected: {"status":"healthy","timestamp":"..."}

curl http://YOUR_PI_IP:8000/api/v1/vehicles
# Expected: {"vehicles":[...]}
```

### 2. Test MMIT Driver

```bash
cd ~/kintsugi/MMIT
./blackbox_dpu --interactive
```

You should see:
- Live telemetry data
- Vehicle metrics
- Sensor readings

### 3. Test Full Integration

1. Start backend on Pi
2. Start MMIT driver on Pi
3. Start frontend on laptop
4. Login with `admin@hybrid-drive.io` / `demo123`
5. View real-time telemetry from MMIT

## Troubleshooting

### Backend Won't Start

```bash
# Check if port 8000 is in use
sudo lsof -i :8000

# Kill existing process
sudo kill -9 $(sudo lsof -t -i:8000)

# Check logs
sudo journalctl -u kintsugi-backend -n 50
```

### MMIT Build Errors

```bash
# Install missing dependencies
sudo apt-get install -y build-essential libcurl4-openssl-dev

# Clean and rebuild
cd ~/kintsugi/MMIT
make clean
make
```

### Frontend Can't Connect

1. **Check Pi IP address:**
   ```bash
   hostname -I
   ```

2. **Test connectivity:**
   ```bash
   # From laptop
   ping YOUR_PI_IP
   curl http://YOUR_PI_IP:8000/health
   ```

3. **Check firewall:**
   ```bash
   # On Pi
   sudo ufw status
   sudo ufw allow 8000/tcp
   ```

### MMIT Not Sending Data

```bash
# Check if backend is running
curl http://localhost:8000/health

# Run MMIT with verbose output
./blackbox_dpu --interactive --backend http://localhost:8000

# Check network logs
sudo journalctl -u kintsugi-mmit -f
```

## Performance Tips

### Optimize for Raspberry Pi

1. **Reduce telemetry rate** (edit MMIT code):
   ```c
   // In realistic_drive_sim.c
   #define TELEMETRY_INTERVAL_MS 1000  // Send every 1 second instead of 100ms
   ```

2. **Limit concurrent connections:**
   ```bash
   # In backend startup
   uvicorn main:app --host 0.0.0.0 --port 8000 --workers 2
   ```

3. **Monitor system resources:**
   ```bash
   # Install monitoring tools
   sudo apt-get install htop
   htop
   ```

## Monitoring

### Check Service Status

```bash
# Backend status
sudo systemctl status kintsugi-backend

# MMIT status
sudo systemctl status kintsugi-mmit

# View all logs
sudo journalctl -u kintsugi-backend -u kintsugi-mmit -f
```

### System Resources

```bash
# CPU and Memory
htop

# Disk usage
df -h

# Network activity
ifstat
```

## Auto-Start on Boot

Services are automatically configured to start on boot when using systemd setup.

To disable auto-start:
```bash
sudo systemctl disable kintsugi-backend
sudo systemctl disable kintsugi-mmit
```

## Updating the System

```bash
cd ~/kintsugi
git pull

# Rebuild MMIT
cd MMIT
make clean && make

# Restart services
sudo systemctl restart kintsugi-backend
sudo systemctl restart kintsugi-mmit
```

## Network Configuration

### Static IP (Optional)

Edit `/etc/dhcpcd.conf`:
```bash
sudo nano /etc/dhcpcd.conf

# Add at the end:
interface eth0
static ip_address=192.168.1.100/24
static routers=192.168.1.1
static domain_name_servers=192.168.1.1 8.8.8.8
```

Reboot:
```bash
sudo reboot
```

## Security Notes

1. **Change default credentials** in `backend/main.py`
2. **Use HTTPS** in production (setup reverse proxy with Nginx)
3. **Enable firewall:**
   ```bash
   sudo ufw enable
   sudo ufw allow 22/tcp  # SSH
   sudo ufw allow 8000/tcp  # Backend
   ```

## Accessing from Other Devices

The backend will be accessible from any device on your network:
- Dashboard: http://YOUR_PI_IP:8000/docs
- API: http://YOUR_PI_IP:8000/api/v1/
- WebSocket: ws://YOUR_PI_IP:8000/ws/telemetry/

## Support

For issues:
1. Check service logs: `sudo journalctl -u kintsugi-backend -f`
2. Verify network connectivity: `curl http://localhost:8000/health`
3. Check MMIT build: `cd ~/kintsugi/MMIT && make clean && make`

---

**Quick Reference Commands:**

```bash
# Start everything
sudo systemctl start kintsugi-backend kintsugi-mmit

# Stop everything
sudo systemctl stop kintsugi-backend kintsugi-mmit

# View logs
sudo journalctl -u kintsugi-backend -u kintsugi-mmit -f

# Check status
sudo systemctl status kintsugi-backend kintsugi-mmit
```
