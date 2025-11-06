#!/bin/bash
# Complete setup and run script for Raspberry Pi
# This script will set up MMIT and Backend, then run them

echo "=============================================="
echo "  Kintsugi - Raspberry Pi Complete Setup"
echo "=============================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get Raspberry Pi IP
PI_IP=$(hostname -I | awk '{print $1}')
echo -e "${GREEN}Raspberry Pi IP: $PI_IP${NC}"
echo ""

# Step 1: Update system
echo -e "${YELLOW}Step 1: Updating system...${NC}"
sudo apt-get update -y
sudo apt-get upgrade -y

# Step 2: Install dependencies
echo ""
echo -e "${YELLOW}Step 2: Installing dependencies...${NC}"
sudo apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    gcc \
    make \
    libcurl4-openssl-dev \
    git \
    curl \
    build-essential

# Step 3: Clone repository (if not already done)
echo ""
echo -e "${YELLOW}Step 3: Setting up project...${NC}"
cd ~

if [ ! -d "kintsugi" ]; then
    echo "Cloning repository..."
    git clone https://github.com/afnaayusuf/kintsugi.git
else
    echo "Repository already exists, pulling latest changes..."
    cd kintsugi
    git pull
    cd ~
fi

# Step 4: Setup Backend
echo ""
echo -e "${YELLOW}Step 4: Setting up Backend...${NC}"
cd ~/kintsugi/backend

# Create virtual environment
if [ ! -d "venv" ]; then
    echo "Creating Python virtual environment..."
    python3 -m venv venv
fi

# Activate and install dependencies
echo "Installing Python dependencies..."
source venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
deactivate

echo -e "${GREEN}✓ Backend setup complete${NC}"

# Step 5: Build MMIT
echo ""
echo -e "${YELLOW}Step 5: Building MMIT Driver...${NC}"
cd ~/kintsugi/MMIT

# Clean previous builds
make clean

# Build MMIT
echo "Compiling MMIT driver..."
make

if [ -f "blackbox_dpu" ]; then
    echo -e "${GREEN}✓ MMIT driver built successfully${NC}"
else
    echo -e "${RED}✗ MMIT build failed${NC}"
    exit 1
fi

# Step 6: Create systemd services
echo ""
echo -e "${YELLOW}Step 6: Creating systemd services...${NC}"

# Backend service
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

# MMIT service
sudo tee /etc/systemd/system/kintsugi-mmit.service > /dev/null <<EOF
[Unit]
Description=Kintsugi MMIT Telemetry Driver
After=network.target kintsugi-backend.service

[Service]
Type=simple
User=$USER
WorkingDirectory=$HOME/kintsugi/MMIT
ExecStart=$HOME/kintsugi/MMIT/blackbox_dpu --interactive
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# Reload systemd
sudo systemctl daemon-reload

# Enable services
sudo systemctl enable kintsugi-backend
sudo systemctl enable kintsugi-mmit

echo -e "${GREEN}✓ Systemd services created and enabled${NC}"

# Step 7: Start services
echo ""
echo -e "${YELLOW}Step 7: Starting services...${NC}"

sudo systemctl start kintsugi-backend
sleep 3
sudo systemctl start kintsugi-mmit

echo ""
echo "=============================================="
echo -e "${GREEN}  Setup Complete!${NC}"
echo "=============================================="
echo ""
echo "Service Status:"
echo "---------------"
sudo systemctl status kintsugi-backend --no-pager -l
echo ""
sudo systemctl status kintsugi-mmit --no-pager -l
echo ""
echo "=============================================="
echo "Access Information:"
echo "=============================================="
echo ""
echo "Backend API:     http://$PI_IP:8000"
echo "API Docs:        http://$PI_IP:8000/docs"
echo "Health Check:    http://$PI_IP:8000/health"
echo ""
echo "=============================================="
echo "Useful Commands:"
echo "=============================================="
echo ""
echo "View backend logs:   sudo journalctl -u kintsugi-backend -f"
echo "View MMIT logs:      sudo journalctl -u kintsugi-mmit -f"
echo "Stop backend:        sudo systemctl stop kintsugi-backend"
echo "Stop MMIT:           sudo systemctl stop kintsugi-mmit"
echo "Restart backend:     sudo systemctl restart kintsugi-backend"
echo "Restart MMIT:        sudo systemctl restart kintsugi-mmit"
echo ""
echo "=============================================="
echo "Frontend Configuration (on your laptop):"
echo "=============================================="
echo ""
echo "Create .env.local file with:"
echo "NEXT_PUBLIC_API_URL=http://$PI_IP:8000"
echo "NEXT_PUBLIC_WS_URL=ws://$PI_IP:8000"
echo ""
echo "Then run: npm run dev"
echo ""
