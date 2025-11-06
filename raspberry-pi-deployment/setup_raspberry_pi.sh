#!/bin/bash
# Complete Raspberry Pi Setup Script for Kintsugi
# This script sets up both the MMIT driver and Backend on Raspberry Pi

set -e

echo "=============================================="
echo "  Kintsugi Raspberry Pi Setup"
echo "=============================================="

# Get Raspberry Pi IP address
PI_IP=$(hostname -I | awk '{print $1}')
echo "Raspberry Pi IP: $PI_IP"

# Update system
echo ""
echo "Step 1: Updating system packages..."
sudo apt-get update
sudo apt-get upgrade -y

# Install required packages
echo ""
echo "Step 2: Installing required packages..."
sudo apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    gcc \
    make \
    libcurl4-openssl-dev \
    git \
    curl

# Create project directory
echo ""
echo "Step 3: Setting up project directories..."
cd ~
if [ ! -d "kintsugi" ]; then
    git clone https://github.com/afnaayusuf/kintsugi.git
else
    cd kintsugi
    git pull
    cd ~
fi

cd ~/kintsugi

# Setup Backend
echo ""
echo "Step 4: Setting up FastAPI Backend..."
cd backend

# Create virtual environment
if [ ! -d "venv" ]; then
    python3 -m venv venv
fi

# Activate virtual environment and install dependencies
source venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt

# Create systemd service for backend
echo ""
echo "Step 5: Creating backend systemd service..."
sudo tee /etc/systemd/system/kintsugi-backend.service > /dev/null <<EOF
[Unit]
Description=Kintsugi FastAPI Backend
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$HOME/kintsugi/backend
Environment="PATH=$HOME/kintsugi/backend/venv/bin"
ExecStart=$HOME/kintsugi/backend/venv/bin/uvicorn main:app --host 0.0.0.0 --port 8000
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# Setup MMIT Driver
echo ""
echo "Step 6: Building MMIT Driver..."
cd ~/kintsugi/MMIT

# Clean and build
make clean
make

# Create systemd service for MMIT
echo ""
echo "Step 7: Creating MMIT systemd service..."
sudo tee /etc/systemd/system/kintsugi-mmit.service > /dev/null <<EOF
[Unit]
Description=Kintsugi MMIT Telemetry Driver
After=network.target kintsugi-backend.service

[Service]
Type=simple
User=$USER
WorkingDirectory=$HOME/kintsugi/MMIT
ExecStart=$HOME/kintsugi/MMIT/blackbox_dpu --interactive --backend http://localhost:8000
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# Enable and start services
echo ""
echo "Step 8: Enabling and starting services..."
sudo systemctl daemon-reload
sudo systemctl enable kintsugi-backend
sudo systemctl enable kintsugi-mmit

echo ""
echo "=============================================="
echo "  Setup Complete!"
echo "=============================================="
echo ""
echo "Backend will be available at: http://$PI_IP:8000"
echo "MMIT Driver will send telemetry to backend"
echo ""
echo "To start services:"
echo "  sudo systemctl start kintsugi-backend"
echo "  sudo systemctl start kintsugi-mmit"
echo ""
echo "To check status:"
echo "  sudo systemctl status kintsugi-backend"
echo "  sudo systemctl status kintsugi-mmit"
echo ""
echo "To view logs:"
echo "  sudo journalctl -u kintsugi-backend -f"
echo "  sudo journalctl -u kintsugi-mmit -f"
echo ""
echo "Configure your laptop frontend to connect to:"
echo "  http://$PI_IP:8000"
echo ""
