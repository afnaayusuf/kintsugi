#!/bin/bash

# Install dependencies
echo "Installing Python dependencies..."
pip install -r requirements.txt

# Run the backend
echo "Starting FastAPI backend..."
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
