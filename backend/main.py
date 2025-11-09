from fastapi import FastAPI, WebSocket, Depends, HTTPException, status, Header
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
import asyncio
import json
from datetime import datetime, timedelta
from typing import Optional, List, Dict, Any
from pydantic import BaseModel
import jwt
from pathlib import Path
import os
import random
import math

# ==================== CONFIG ====================
SECRET_KEY = os.getenv("SECRET_KEY", "your-secret-key-change-in-production")
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 1440  # 24 hours
DATA_FILE = Path("data.json")

ALLOWED_ORIGINS = os.getenv("ALLOWED_ORIGINS", "http://localhost:3000,http://localhost:8000,http://172.20.10.3:3000,http://172.20.10.3:8000").split(",")

# ==================== PYDANTIC MODELS ====================
class LoginRequest(BaseModel):
    email: str
    password: str

class TokenResponse(BaseModel):
    access_token: str
    token_type: str
    user_id: str

class Vehicle(BaseModel):
    id: str
    name: str
    make: str
    model: str
    year: int
    vin: str
    license_plate: str
    status: str = "connected"
    battery_level: float = 100
    last_updated: str = ""

class TelemetryData(BaseModel):
    timestamp: str
    speed: float
    rpm: int
    engine_temp: float
    battery_voltage: float
    fuel_level: float
    traction_control: bool
    cpu_usage: float
    memory_usage: float
    latency_ms: float
    uptime_seconds: int
    ambient_temp: float
    humidity: float
    gps_lat: float
    gps_lon: float
    wheel_speed_fl: float
    wheel_speed_fr: float
    wheel_speed_rl: float
    wheel_speed_rr: float
    diagnostics: List[str] = []

class TelemetryUpdate(BaseModel):
    vehicle_id: str
    data: TelemetryData

# ==================== REALISTIC VEHICLE SIMULATOR ====================
class VehicleSimulator:
    """Simulates realistic vehicle telemetry with gradual changes"""
    
    def __init__(self, vehicle_id: str):
        self.vehicle_id = vehicle_id
        # Current state
        self.speed = 0.0  # km/h
        self.rpm = 800  # idle RPM
        self.throttle = 0.0
        self.brake = 0.0
        self.gear = 1
        self.fuel_level = 75.0
        self.engine_temp = 70.0  # Celsius
        self.battery_voltage = 12.6
        self.odometer = random.uniform(5000, 50000)
        
        # GPS starting position (different for each vehicle)
        base_lat = 10.0053
        base_lon = 76.3601
        offset = hash(vehicle_id) % 100 / 10000.0
        self.gps_lat = base_lat + offset
        self.gps_lon = base_lon + offset
        
        # Environmental
        self.ambient_temp = 28.0
        self.humidity = 65.0
        
        # System
        self.cpu_usage = 15.0
        self.memory_usage = 45.0
        self.uptime = 0
        
        # Driving mode: idle, accelerating, cruising, braking
        self.mode = "idle"
        self.mode_duration = 0
        self.target_speed = 0
        
    def update(self, dt: float = 1.0):
        """Update vehicle state with realistic physics"""
        self.uptime += dt
        
        # Change driving mode randomly
        self.mode_duration += dt
        if self.mode_duration > random.uniform(5, 15):
            self.mode_duration = 0
            self.mode = random.choice(["idle", "accelerating", "cruising", "braking", "cruising"])
            
            if self.mode == "accelerating":
                self.target_speed = min(self.speed + random.uniform(20, 60), 120)
            elif self.mode == "cruising":
                self.target_speed = self.speed
            elif self.mode == "braking":
                self.target_speed = max(0, self.speed - random.uniform(20, 40))
            elif self.mode == "idle":
                self.target_speed = 0
        
        # Update speed based on mode
        if self.mode == "accelerating":
            self.throttle = min(100, self.throttle + random.uniform(5, 15))
            self.brake = 0
            self.speed += random.uniform(1, 3)
            if self.speed >= self.target_speed:
                self.mode = "cruising"
        elif self.mode == "braking":
            self.throttle = 0
            self.brake = min(100, self.brake + random.uniform(10, 30))
            self.speed = max(0, self.speed - random.uniform(2, 5))
            if self.speed <= self.target_speed:
                self.mode = "idle" if self.target_speed == 0 else "cruising"
        elif self.mode == "cruising":
            self.throttle = 20 + random.uniform(-5, 5)
            self.brake = 0
            self.speed += random.uniform(-1, 1)
            self.speed = max(30, min(120, self.speed))
        else:  # idle
            self.throttle = 0
            self.brake = 0
            self.speed = max(0, self.speed - random.uniform(0.5, 1.5))
        
        # Constrain speed
        self.speed = max(0, min(180, self.speed))
        
        # Update RPM based on speed and throttle
        if self.speed < 1:
            self.rpm = 800 + random.uniform(-50, 50)  # Idle
        else:
            base_rpm = (self.speed * 50) + (self.throttle * 20)
            self.rpm = max(800, min(6000, base_rpm + random.uniform(-100, 100)))
        
        # Update gear based on RPM and speed
        if self.speed < 20:
            self.gear = 1
        elif self.speed < 40:
            self.gear = 2
        elif self.speed < 60:
            self.gear = 3
        elif self.speed < 90:
            self.gear = 4
        else:
            self.gear = 5
        
        # Engine temperature rises when driving, cools when idle
        if self.speed > 50:
            self.engine_temp = min(95, self.engine_temp + random.uniform(0.1, 0.3))
        elif self.speed > 20:
            self.engine_temp = min(90, self.engine_temp + random.uniform(0.05, 0.15))
        else:
            self.engine_temp = max(70, self.engine_temp - random.uniform(0.05, 0.1))
        
        # Fuel consumption
        fuel_consumption = (self.throttle / 100) * 0.002 + 0.0001
        self.fuel_level = max(10, self.fuel_level - fuel_consumption)
        
        # Battery voltage varies slightly
        self.battery_voltage = 12.4 + random.uniform(-0.1, 0.2)
        
        # GPS updates (simulate movement)
        if self.speed > 0:
            # Move roughly in a direction
            self.gps_lat += random.uniform(-0.0001, 0.0001) * (self.speed / 100)
            self.gps_lon += random.uniform(-0.0001, 0.0001) * (self.speed / 100)
            self.odometer += self.speed / 3600  # km
        
        # Ambient conditions vary slowly
        self.ambient_temp += random.uniform(-0.1, 0.1)
        self.ambient_temp = max(20, min(40, self.ambient_temp))
        self.humidity += random.uniform(-0.2, 0.2)
        self.humidity = max(40, min(90, self.humidity))
        
        # System usage
        self.cpu_usage = 15 + (self.speed / 10) + random.uniform(-5, 5)
        self.cpu_usage = max(5, min(80, self.cpu_usage))
        self.memory_usage = 45 + random.uniform(-2, 2)
        self.memory_usage = max(30, min(75, self.memory_usage))
    
    def get_telemetry(self) -> Dict[str, Any]:
        """Get current telemetry snapshot"""
        # Calculate wheel speeds (with slight variations)
        base_wheel_speed = self.speed * 1.1
        
        return {
            "timestamp": datetime.utcnow().isoformat(),
            "speed": round(self.speed, 1),
            "rpm": int(self.rpm),
            "engine_temp": round(self.engine_temp, 1),
            "battery_voltage": round(self.battery_voltage, 2),
            "fuel_level": round(self.fuel_level, 1),
            "throttle_pct": round(self.throttle, 1),
            "brake_pct": round(self.brake, 1),
            "gear": self.gear,
            "traction_control": self.speed > 80 or self.brake > 50,
            "cpu_usage": round(self.cpu_usage, 1),
            "memory_usage": round(self.memory_usage, 1),
            "latency_ms": round(random.uniform(20, 50), 1),
            "uptime_seconds": int(self.uptime),
            "ambient_temp": round(self.ambient_temp, 1),
            "humidity": round(self.humidity, 1),
            "gps_lat": round(self.gps_lat, 6),
            "gps_lon": round(self.gps_lon, 6),
            "wheel_speed_fl": round(base_wheel_speed + random.uniform(-1, 1), 1),
            "wheel_speed_fr": round(base_wheel_speed + random.uniform(-1, 1), 1),
            "wheel_speed_rl": round(base_wheel_speed + random.uniform(-1, 1), 1),
            "wheel_speed_rr": round(base_wheel_speed + random.uniform(-1, 1), 1),
            "diagnostics": self._get_diagnostics()
        }
    
    def _get_diagnostics(self) -> List[str]:
        """Generate realistic diagnostic codes"""
        codes = []
        if self.engine_temp > 92:
            codes.append("P0217: Engine Coolant Over Temperature")
        if self.fuel_level < 15:
            codes.append("P0462: Fuel Level Sensor Low")
        if self.battery_voltage < 12.2:
            codes.append("P0562: System Voltage Low")
        return codes

# ==================== STORAGE ====================
class DataStore:
    def __init__(self):
        # Initialize vehicle simulators
        self.simulators = {}
        self.data = {
            "users": {
                "admin@hybrid-drive.io": {
                    "id": "user-001",
                    "email": "admin@hybrid-drive.io",
                    "password": "demo123",
                    "name": "Admin User"
                }
            },
            "vehicles": {
                "tesla-1": {
                    "id": "tesla-1",
                    "name": "Tesla Model S",
                    "make": "Tesla",
                    "model": "Model S",
                    "year": 2023,
                    "vin": "5YJ3E1EA7PF123456",
                    "license_plate": "TESLA01",
                    "status": "connected",
                    "battery_level": 85,
                    "last_updated": datetime.utcnow().isoformat()
                },
                "toyota-1": {
                    "id": "toyota-1",
                    "name": "Toyota Hybrid",
                    "make": "Toyota",
                    "model": "Prius Prime",
                    "year": 2023,
                    "vin": "JTDKP5C29L3098765",
                    "license_plate": "TOYOTA01",
                    "status": "connected",
                    "battery_level": 75,
                    "last_updated": datetime.utcnow().isoformat()
                },
                "hybrid-1": {
                    "id": "hybrid-1",
                    "name": "Hybrid Explorer",
                    "make": "Ford",
                    "model": "Explorer Hybrid",
                    "year": 2023,
                    "vin": "1FMEU7DH5LUD12345",
                    "license_plate": "HYBRID01",
                    "status": "connected",
                    "battery_level": 92,
                    "last_updated": datetime.utcnow().isoformat()
                }
            },
            "telemetry": {}
        }
        self.load_from_file()
        
        # Create simulators for each vehicle
        for vehicle_id in self.data["vehicles"].keys():
            self.simulators[vehicle_id] = VehicleSimulator(vehicle_id)

    def load_from_file(self):
        if DATA_FILE.exists():
            try:
                with open(DATA_FILE, "r") as f:
                    self.data = json.load(f)
            except:
                pass

    def save_to_file(self):
        with open(DATA_FILE, "w") as f:
            json.dump(self.data, f, indent=2, default=str)

    def get_user(self, email: str):
        return self.data["users"].get(email)

    def get_vehicles(self) -> Dict[str, Any]:
        # Update all vehicle statuses based on simulator activity
        for vehicle_id in self.data["vehicles"]:
            # If simulator exists and has been running, vehicle is online
            if vehicle_id in self.simulators and self.simulators[vehicle_id].uptime > 0:
                self.data["vehicles"][vehicle_id]["status"] = "connected"
                self.data["vehicles"][vehicle_id]["last_updated"] = datetime.utcnow().isoformat()
            else:
                # Check telemetry history
                telemetry_list = self.data["telemetry"].get(vehicle_id, [])
                if telemetry_list:
                    last_update = datetime.fromisoformat(telemetry_list[-1]["timestamp"])
                    time_diff = (datetime.utcnow() - last_update).total_seconds()
                    self.data["vehicles"][vehicle_id]["status"] = "connected" if time_diff < 60 else "offline"
                    self.data["vehicles"][vehicle_id]["last_updated"] = telemetry_list[-1]["timestamp"]
                else:
                    # Initialize simulator to mark as online
                    if vehicle_id not in self.simulators:
                        self.simulators[vehicle_id] = VehicleSimulator(vehicle_id)
                    self.data["vehicles"][vehicle_id]["status"] = "connected"
                    self.data["vehicles"][vehicle_id]["last_updated"] = datetime.utcnow().isoformat()
        return self.data["vehicles"]

    def get_vehicle(self, vehicle_id: str):
        vehicle = self.data["vehicles"].get(vehicle_id)
        if vehicle:
            # Update status based on last telemetry
            telemetry_list = self.data["telemetry"].get(vehicle_id, [])
            if telemetry_list:
                last_update = datetime.fromisoformat(telemetry_list[-1]["timestamp"])
                time_diff = (datetime.utcnow() - last_update).total_seconds()
                vehicle["status"] = "connected" if time_diff < 60 else "offline"
                vehicle["last_updated"] = telemetry_list[-1]["timestamp"]
            else:
                vehicle["status"] = "offline"
        return vehicle

    def add_telemetry(self, vehicle_id: str, telemetry: TelemetryData):
        if vehicle_id not in self.data["telemetry"]:
            self.data["telemetry"][vehicle_id] = []
        
        telemetry_dict = telemetry.dict()
        self.data["telemetry"][vehicle_id].append(telemetry_dict)
        
        # Keep only last 1000 entries per vehicle
        if len(self.data["telemetry"][vehicle_id]) > 1000:
            self.data["telemetry"][vehicle_id] = self.data["telemetry"][vehicle_id][-1000:]
        
        self.save_to_file()
        return telemetry_dict

    def get_telemetry(self, vehicle_id: str, limit: int = 100) -> List[Dict]:
        telemetry_list = self.data["telemetry"].get(vehicle_id, [])
        return telemetry_list[-limit:]
    
    def get_current_simulated_telemetry(self, vehicle_id: str) -> Dict:
        """Get current telemetry from simulator"""
        if vehicle_id not in self.simulators:
            self.simulators[vehicle_id] = VehicleSimulator(vehicle_id)
        
        # Update simulator
        self.simulators[vehicle_id].update()
        telemetry = self.simulators[vehicle_id].get_telemetry()
        
        # Store in telemetry history
        if vehicle_id not in self.data["telemetry"]:
            self.data["telemetry"][vehicle_id] = []
        
        self.data["telemetry"][vehicle_id].append(telemetry)
        
        # Keep only last 1000 entries
        if len(self.data["telemetry"][vehicle_id]) > 1000:
            self.data["telemetry"][vehicle_id] = self.data["telemetry"][vehicle_id][-1000:]
        
        return telemetry

    def get_telemetry_history(self, vehicle_id: str, start_date: str, end_date: str) -> List[Dict]:
        telemetry_list = self.data["telemetry"].get(vehicle_id, [])
        start = datetime.fromisoformat(start_date)
        end = datetime.fromisoformat(end_date)
        
        return [
            t for t in telemetry_list
            if start <= datetime.fromisoformat(t["timestamp"]) <= end
        ]

# ==================== APP SETUP ====================
store = DataStore()
app = FastAPI(title="Kintsugi - Vehicle Telemetry Backend", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=ALLOWED_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ==================== JWT UTILITIES ====================
def create_access_token(data: dict, expires_delta: Optional[timedelta] = None):
    to_encode = data.copy()
    if expires_delta:
        expire = datetime.utcnow() + expires_delta
    else:
        expire = datetime.utcnow() + timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
    
    to_encode.update({"exp": expire})
    encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
    return encoded_jwt

def verify_token(authorization: str = None) -> dict:
    """Verify JWT token from Authorization header"""
    if not authorization:
        raise HTTPException(status_code=401, detail="Authorization header missing")
    
    # Extract token from "Bearer <token>" format
    parts = authorization.split()
    if len(parts) != 2 or parts[0].lower() != "bearer":
        raise HTTPException(status_code=401, detail="Invalid authorization header format")
    
    token = parts[1]
    
    try:
        payload = jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])
        email: str = payload.get("sub")
        if email is None:
            raise HTTPException(status_code=401, detail="Invalid token")
        return payload
    except jwt.ExpiredSignatureError:
        raise HTTPException(status_code=401, detail="Token expired")
    except jwt.InvalidTokenError:
        raise HTTPException(status_code=401, detail="Invalid token")

# ==================== AUTH ROUTES ====================
@app.post("/api/v1/auth/login", response_model=TokenResponse)
async def login(credentials: LoginRequest):
    user = store.get_user(credentials.email)
    
    if not user or user["password"] != credentials.password:
        raise HTTPException(status_code=401, detail="Invalid credentials")
    
    access_token = create_access_token(data={"sub": credentials.email, "user_id": user["id"]})
    return {
        "access_token": access_token,
        "token_type": "bearer",
        "user_id": user["id"]
    }

@app.post("/api/v1/auth/verify")
async def verify(token: str):
    payload = verify_token(token)
    return {"valid": True, "email": payload.get("sub")}

# ==================== VEHICLE ROUTES ====================
@app.get("/api/v1/vehicles")
async def get_vehicles(authorization: str = Header(None)):
    if authorization:
        verify_token(authorization)
    vehicles = list(store.get_vehicles().values())
    return {"vehicles": vehicles}

@app.get("/api/v1/vehicles/{vehicle_id}")
async def get_vehicle(vehicle_id: str, authorization: str = Header(None)):
    if authorization:
        verify_token(authorization)
    vehicle = store.get_vehicle(vehicle_id)
    if not vehicle:
        raise HTTPException(status_code=404, detail="Vehicle not found")
    return vehicle

# ==================== TELEMETRY ROUTES ====================
@app.get("/api/v1/telemetry/{vehicle_id}/current")
async def get_current_telemetry(vehicle_id: str, authorization: str = Header(None)):
    if authorization:
        verify_token(authorization)
    
    # Get simulated telemetry (realistic and gradual)
    telemetry = store.get_current_simulated_telemetry(vehicle_id)
    
    return {
        "vehicle_id": vehicle_id,
        "telemetry": telemetry
    }

@app.get("/api/v1/telemetry/{vehicle_id}/history")
async def get_telemetry_history(
    vehicle_id: str,
    start_date: str,
    end_date: str,
    limit: int = 100,
    authorization: str = Header(None)
):
    if authorization:
        verify_token(authorization)
    telemetry = store.get_telemetry_history(vehicle_id, start_date, end_date)
    return {"telemetry": telemetry[-limit:]}

@app.post("/api/v1/telemetry/{vehicle_id}/report")
async def report_telemetry(vehicle_id: str, data: TelemetryData, authorization: str = Header(None)):
    if authorization:
        verify_token(authorization)
    result = store.add_telemetry(vehicle_id, data)
    return {"success": True, "data": result}

# ==================== WEBSOCKET ====================
class ConnectionManager:
    def __init__(self):
        self.active_connections: List[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        self.active_connections.remove(websocket)

    async def broadcast(self, message: dict):
        for connection in self.active_connections:
            try:
                await connection.send_json(message)
            except:
                pass

manager = ConnectionManager()

@app.websocket("/ws/telemetry/{vehicle_id}")
async def websocket_endpoint(websocket: WebSocket, vehicle_id: str):
    await manager.connect(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            message = json.loads(data)
            
            if message.get("type") == "telemetry":
                telemetry = TelemetryData(**message.get("data"))
                store.add_telemetry(vehicle_id, telemetry)
                await manager.broadcast({
                    "type": "telemetry_update",
                    "vehicle_id": vehicle_id,
                    "data": telemetry.dict()
                })
    except Exception as e:
        print(f"WebSocket error: {e}")
    finally:
        manager.disconnect(websocket)

# ==================== HEALTH CHECK ====================
@app.get("/health")
async def health():
    return {"status": "healthy", "timestamp": datetime.utcnow().isoformat()}

@app.get("/")
async def root():
    return {"message": "Vehicle Telemetry Backend API - Running on localhost:8000"}
