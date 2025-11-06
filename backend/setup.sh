#!/bin/bash

echo "================================================"
echo "Kintsugi Backend Setup Script"
echo "================================================"

# Check Python version
echo "Checking Python version..."
python_version=$(python --version 2>&1 | awk '{print $2}')
echo "Python version: $python_version"

if ! command -v python &> /dev/null; then
    echo "Error: Python is not installed"
    exit 1
fi

# Create virtual environment
echo ""
echo "Creating virtual environment..."
python -m venv venv

# Activate virtual environment
echo "Activating virtual environment..."
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    source venv/Scripts/activate
else
    source venv/bin/activate
fi

# Install dependencies
echo ""
echo "Installing Python dependencies..."
pip install --upgrade pip
pip install -r requirements.txt

# Create .env file if it doesn't exist
echo ""
if [ ! -f .env ]; then
    echo "Creating .env file from .env.example..."
    cp .env.example .env
    echo "⚠️  Please update .env with your configuration"
fi

echo ""
echo "================================================"
echo "Setup complete!"
echo "================================================"
echo ""
echo "To start the backend, run:"
echo "  source venv/bin/activate  # macOS/Linux"
echo "  venv\\Scripts\\activate     # Windows"
echo "  uvicorn main:app --reload"
echo ""
echo "Backend will be available at: http://localhost:8000"
