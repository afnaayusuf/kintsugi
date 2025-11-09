from fastapi import FastAPI, WebSocket, Depends, HTTPException, status
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
import time

# ==================== CONFIG ====================
SECRET_KEY = os.getenv("SECRET_KEY", "your-secret-key-change-in-production")
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 1440  # 24 hours
DATA_FILE = Path("data.json")

ALLOWED_ORIGINS = os.getenv("ALLOWED_ORIGINS", "http://localhost:3000,http://localhost:8000").split(",")

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

# ==================== VEHICLE SIMULATOR ====================
class VehicleSimulator:
    """Realistic vehicle simulator with gradual changes"""
    def __init__(self, vehicle_id: str):
        self.vehicle_id = vehicle_id
        self.speed = 0.0
        self.rpm = 800
        self.throttle_pct = 0.0
        self.brake_pct = 0.0
        self.gear = 1
        self.engine_temp = 70.0
        self.fuel_level = 100.0
        self.battery_voltage = 12.6
        self.gps_lat = 37.7749
        self.gps_lon = -122.4194
        self.ambient_temp = 22.0
        self.humidity = 45.0
        self.mode = "idle"  # idle, accelerating, cruising, braking
        self.last_update = time.time()
        self.uptime_start = time.time()
        
        print(f"[SIMULATOR] Created new simulator for {vehicle_id}")

    def update(self):
        """Update vehicle state with realistic physics"""
        current_time = time.time()
        dt = current_time - self.last_update
        self.last_update = current_time
        
        # Randomly change driving mode
        if random.random() < 0.05:  # 5% chance per update
            self.mode = random.choice(["idle", "accelerating", "cruising", "braking"])
            print(f"[SIMULATOR] {self.vehicle_id} mode changed to: {self.mode}")
        
        # Update based on mode
        if self.mode == "idle":
            self.throttle_pct = max(0, self.throttle_pct - 5)
            self.brake_pct = 0
            self.speed = max(0, self.speed - 2)
            self.rpm = 800 + int(self.speed * 10)
            
        elif self.mode == "accelerating":
            self.throttle_pct = min(100, self.throttle_pct + 10)
            self.brake_pct = 0
            self.speed = min(120, self.speed + 3)
            self.rpm = 800 + int(self.speed * 50)
            
        elif self.mode == "cruising":
            self.throttle_pct = 40 + random.uniform(-5, 5)
            self.brake_pct = 0
            target_speed = 80
            if self.speed < target_speed:
                self.speed += 1
            elif self.speed > target_speed:
                self.speed -= 1
            self.rpm = 2000 + int(self.speed * 20)
            
        elif self.mode == "braking":
            self.throttle_pct = 0
            self.brake_pct = min(100, self.brake_pct + 15)
            self.speed = max(0, self.speed - 5)
            self.rpm = 800 + int(self.speed * 10)
        
        # Engine temperature increases with speed and load
        target_temp = 70 + (self.speed / 120) * 30 + (self.throttle_pct / 100) * 20
        if self.engine_temp < target_temp:
            self.engine_temp += 0.5
        else:
            self.engine_temp -= 0.3
        
        # Fuel consumption based on throttle
        self.fuel_level = max(0, self.fuel_level - (self.throttle_pct / 100) * 0.01)
        
        # GPS drifts slightly
        self.gps_lat += random.uniform(-0.0001, 0.0001)
        self.gps_lon += random.uniform(-0.0001, 0.0001)
        
        # Ambient conditions vary slowly
        self.ambient_temp += random.uniform(-0.1, 0.1)
        self.humidity += random.uniform(-0.5, 0.5)
        self.humidity = max(20, min(80, self.humidity))

    def get_telemetry(self) -> dict:
        """Get current telemetry snapshot"""
        uptime = int(time.time() - self.uptime_start)
        
        return {
            "timestamp": datetime.utcnow().isoformat() + "Z",
            "speed": round(self.speed, 1),
            "rpm": int(self.rpm),
            "engine_temp": round(self.engine_temp, 1),
            "battery_voltage": round(self.battery_voltage + random.uniform(-0.1, 0.1), 2),
            "fuel_level": round(self.fuel_level, 1),
            "throttle_pct": round(self.throttle_pct, 1),
            "brake_pct": round(self.brake_pct, 1),
            "gear": self.gear,
            "traction_control": self.brake_pct > 50,
            "cpu_usage": round(random.uniform(10, 30), 1),
            "memory_usage": round(random.uniform(40, 60), 1),
            "latency_ms": round(random.uniform(5, 25), 1),
            "uptime_seconds": uptime,
            "ambient_temp": round(self.ambient_temp, 1),
            "humidity": round(self.humidity, 1),
            "gps_lat": round(self.gps_lat, 6),
            "gps_lon": round(self.gps_lon, 6),
            "wheel_speed_fl": round(self.speed + random.uniform(-0.5, 0.5), 1),
            "wheel_speed_fr": round(self.speed + random.uniform(-0.5, 0.5), 1),
            "wheel_speed_rl": round(self.speed + random.uniform(-0.5, 0.5), 1),
            "wheel_speed_rr": round(self.speed + random.uniform(-0.5, 0.5), 1),
            "diagnostics": []
        }

# Global simulator instances
simulators: Dict[str, VehicleSimulator] = {}

def get_or_create_simulator(vehicle_id: str) -> VehicleSimulator:
    """Get existing simulator or create new one"""
    if vehicle_id not in simulators:
        simulators[vehicle_id] = VehicleSimulator(vehicle_id)
        print(f"[SIMULATOR] Created simulator for {vehicle_id}. Total simulators: {len(simulators)}")
    return simulators[vehicle_id]

# ==================== STORAGE ====================
class DataStore:
    def __init__(self):
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
        return self.data["vehicles"]

    def get_vehicle(self, vehicle_id: str):
        return self.data["vehicles"].get(vehicle_id)

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

def verify_token(token: str) -> dict:
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
async def get_vehicles(token: str = ""):
    if token:
        verify_token(token)
    vehicles = list(store.get_vehicles().values())
    return {"vehicles": vehicles}

@app.get("/api/v1/vehicles/{vehicle_id}")
async def get_vehicle(vehicle_id: str, token: str = ""):
    if token:
        verify_token(token)
    vehicle = store.get_vehicle(vehicle_id)
    if not vehicle:
        raise HTTPException(status_code=404, detail="Vehicle not found")
    return vehicle

# ==================== TELEMETRY ROUTES ====================
@app.get("/api/v1/telemetry/{vehicle_id}/current")
async def get_current_telemetry(vehicle_id: str, token: str = ""):
    """Get current simulated telemetry for a vehicle"""
    if token:
        verify_token(token)
    
    print(f"[API] GET /telemetry/{vehicle_id}/current")
    
    # Get or create simulator for this vehicle
    simulator = get_or_create_simulator(vehicle_id)
    
    # Update simulator state
    simulator.update()
    
    # Get telemetry snapshot
    telemetry = simulator.get_telemetry()
    
    print(f"[API] Returning telemetry: speed={telemetry['speed']}, rpm={telemetry['rpm']}, temp={telemetry['engine_temp']}")
    
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
    token: str = ""
):
    if token:
        verify_token(token)
    telemetry = store.get_telemetry_history(vehicle_id, start_date, end_date)
    return {"telemetry": telemetry[-limit:]}

@app.post("/api/v1/telemetry/{vehicle_id}/report")
async def report_telemetry(vehicle_id: str, data: TelemetryData, token: str = ""):
    if token:
        verify_token(token)
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
