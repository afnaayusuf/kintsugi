#!/bin/bash
# Quick start script for Raspberry Pi
# Run backend and MMIT manually (without systemd)

set -e

# Get Raspberry Pi IP
PI_IP=$(hostname -I | awk '{print $1}')

echo "=============================================="
echo "  Starting Kintsugi on Raspberry Pi"
echo "  Backend: http://$PI_IP:8000"
echo "=============================================="

# Check if we're in the right directory
if [ ! -d "backend" ] || [ ! -d "MMIT" ]; then
    echo "Error: Please run this from the kintsugi project root"
    exit 1
fi

# Start Backend in background
echo ""
echo "Starting FastAPI Backend..."
cd backend

if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi

source venv/bin/activate
pip install -q -r requirements.txt

echo "Backend starting on http://$PI_IP:8000"
uvicorn main:app --host 0.0.0.0 --port 8000 > backend.log 2>&1 &
BACKEND_PID=$!
echo "Backend PID: $BACKEND_PID"

# Wait for backend to start
sleep 3

# Build and run MMIT
echo ""
echo "Building MMIT Driver..."
cd ../MMIT
make clean && make

echo ""
echo "Starting MMIT Telemetry Driver..."
echo "Sending telemetry to http://localhost:8000"
echo ""
echo "Press Ctrl+C to stop both services"
echo "=============================================="
echo ""

# Run MMIT in foreground (with interactive mode)
./blackbox_dpu --interactive --backend http://localhost:8000

# Cleanup on exit
echo ""
echo "Stopping services..."
kill $BACKEND_PID 2>/dev/null || true
echo "Services stopped"
