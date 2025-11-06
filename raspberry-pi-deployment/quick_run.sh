#!/bin/bash
# Quick run script - Start MMIT and Backend without systemd
# Use this for testing or manual control

set -e

PI_IP=$(hostname -I | awk '{print $1}')

echo "=============================================="
echo "  Kintsugi Quick Start"
echo "  Running on: $PI_IP"
echo "=============================================="
echo ""

# Check if in correct directory
if [ ! -d "backend" ] || [ ! -d "MMIT" ]; then
    echo "Error: Run this from ~/kintsugi directory"
    echo "cd ~/kintsugi && ./raspberry-pi-deployment/quick_run.sh"
    exit 1
fi

# Start Backend
echo "Starting Backend..."
cd backend

if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
    source venv/bin/activate
    pip install -r requirements.txt
else
    source venv/bin/activate
fi

echo "Backend running on http://$PI_IP:8000"
uvicorn main:app --host 0.0.0.0 --port 8000 &
BACKEND_PID=$!
echo "Backend PID: $BACKEND_PID"

# Wait for backend to start
echo "Waiting for backend to initialize..."
sleep 5

# Test backend
if curl -s http://localhost:8000/health > /dev/null; then
    echo "✓ Backend is running"
else
    echo "✗ Backend failed to start"
    kill $BACKEND_PID 2>/dev/null || true
    exit 1
fi

# Build and run MMIT
echo ""
echo "Building MMIT Driver..."
cd ../MMIT

if [ ! -f "blackbox_dpu" ]; then
    echo "Compiling MMIT..."
    make clean && make
fi

echo ""
echo "=============================================="
echo "  System Ready!"
echo "=============================================="
echo ""
echo "Backend: http://$PI_IP:8000"
echo "API Docs: http://$PI_IP:8000/docs"
echo ""
echo "Starting MMIT Driver..."
echo "Press Ctrl+C to stop both services"
echo "=============================================="
echo ""

# Run MMIT in foreground
./blackbox_dpu --interactive

# Cleanup on exit
echo ""
echo "Stopping services..."
kill $BACKEND_PID 2>/dev/null || true
echo "Services stopped"
