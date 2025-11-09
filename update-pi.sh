#!/bin/bash

echo "======================================"
echo "Updating Kintsugi Backend on Raspberry Pi"
echo "======================================"
echo ""

# Navigate to git repo
cd ~/kintsugi || { echo "Error: ~/kintsugi directory not found"; exit 1; }

echo "1. Pulling latest code from GitHub..."
git pull origin main

echo ""
echo "2. Copying backend files..."
cp ~/kintsugi/backend/main.py ~/kintsugi-backend/
cp ~/kintsugi/backend/requirements.txt ~/kintsugi-backend/

echo ""
echo "3. Checking for throttle_pct in main.py..."
if grep -q "throttle_pct" ~/kintsugi-backend/main.py; then
    echo "✓ throttle_pct found in main.py"
else
    echo "✗ ERROR: throttle_pct NOT found in main.py!"
    echo "File may not have been copied correctly"
    exit 1
fi

if grep -q "brake_pct" ~/kintsugi-backend/main.py; then
    echo "✓ brake_pct found in main.py"
else
    echo "✗ ERROR: brake_pct NOT found in main.py!"
    exit 1
fi

echo ""
echo "4. Activating virtual environment..."
cd ~/kintsugi-backend
source venv/bin/activate

echo ""
echo "5. Stopping old uvicorn process..."
pkill -f uvicorn
sleep 2

echo ""
echo "6. Starting uvicorn server..."
echo "Server will start on http://0.0.0.0:8000"
echo "Press Ctrl+C to stop"
echo ""

python -m uvicorn main:app --host 0.0.0.0 --port 8000 --reload
