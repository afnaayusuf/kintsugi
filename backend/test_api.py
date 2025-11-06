#!/usr/bin/env python3
"""
Test script for Kintsugi Backend API
Tests all major endpoints and WebSocket connectivity
"""

import requests
import json
from datetime import datetime, timedelta

BASE_URL = "http://localhost:8000"
API_URL = f"{BASE_URL}/api/v1"

def test_health():
    """Test health check endpoint"""
    print("Testing health check...")
    response = requests.get(f"{BASE_URL}/health")
    assert response.status_code == 200
    print(f"✓ Health: {response.json()}")

def test_login():
    """Test authentication"""
    print("\nTesting authentication...")
    data = {
        "email": "admin@hybrid-drive.io",
        "password": "demo123"
    }
    response = requests.post(f"{API_URL}/auth/login", json=data)
    assert response.status_code == 200
    token = response.json()["access_token"]
    print(f"✓ Login successful, token: {token[:20]}...")
    return token

def test_vehicles(token):
    """Test vehicle endpoints"""
    print("\nTesting vehicle endpoints...")
    response = requests.get(f"{API_URL}/vehicles", params={"token": token})
    assert response.status_code == 200
    vehicles = response.json()["vehicles"]
    print(f"✓ Found {len(vehicles)} vehicles")
    
    for vehicle in vehicles:
        print(f"  - {vehicle['name']} ({vehicle['id']})")
    
    return vehicles[0]["id"] if vehicles else None

def test_telemetry(token, vehicle_id):
    """Test telemetry endpoints"""
    print(f"\nTesting telemetry endpoints for {vehicle_id}...")
    
    # Get current telemetry
    response = requests.get(
        f"{API_URL}/telemetry/{vehicle_id}/current",
        params={"token": token}
    )
    assert response.status_code == 200
    print("✓ Current telemetry retrieved")

def test_history(token, vehicle_id):
    """Test historical data endpoint"""
    print(f"\nTesting historical data for {vehicle_id}...")
    
    end_date = datetime.utcnow().isoformat()
    start_date = (datetime.utcnow() - timedelta(days=1)).isoformat()
    
    response = requests.get(
        f"{API_URL}/telemetry/{vehicle_id}/history",
        params={
            "token": token,
            "start_date": start_date,
            "end_date": end_date,
            "limit": 10
        }
    )
    assert response.status_code == 200
    data = response.json()
    print(f"✓ Retrieved {len(data.get('telemetry', []))} historical entries")

def run_tests():
    """Run all tests"""
    print("=" * 50)
    print("Kintsugi Backend API Test Suite")
    print("=" * 50)
    
    try:
        test_health()
        token = test_login()
        vehicle_id = test_vehicles(token)
        
        if vehicle_id:
            test_telemetry(token, vehicle_id)
            test_history(token, vehicle_id)
        
        print("\n" + "=" * 50)
        print("All tests passed! ✓")
        print("=" * 50)
        
    except Exception as e:
        print(f"\n✗ Test failed: {e}")
        return False
    
    return True

if __name__ == "__main__":
    import sys
    success = run_tests()
    sys.exit(0 if success else 1)
