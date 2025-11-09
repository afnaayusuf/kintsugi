from fastapi import FastAPI, WebSocket, Depends, HTTPException, status, Header
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from contextlib import asynccontextmanager
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

# ==================== REALISTIC VEHICLE SIMULATOR ====================
class VehicleSimulator:
    """Simulates realistic vehicle telemetry with physically accurate correlations"""
    
    def __init__(self, vehicle_id: str):
        self.vehicle_id = vehicle_id
        
        # Motion state
        self.speed = 0.0  # km/h
        self.rpm = 0  # RPM - starts at 0 when off
        self.throttle = 0.0  # 0-100%
        self.brake = 0.0  # 0-100%
        self.gear = 0  # 0=Park, 1-6=Drive gears
        
        # Engine state
        self.engine_running = False
        self.engine_temp = 20.0  # Celsius - starts at ambient
        self.oil_pressure = 0.0  # PSI
        self.coolant_temp = 20.0  # Celsius
        
        # Fuel & Battery
        self.fuel_level = random.uniform(40, 90)  # % - start with random fuel
        self.battery_voltage = 12.6  # Volts
        
        # Location (Kochi, India area)
        base_lat = 10.0053
        base_lon = 76.3601
        offset = hash(vehicle_id) % 100 / 10000.0
        self.gps_lat = base_lat + offset
        self.gps_lon = base_lon + offset
        self.heading = random.uniform(0, 360)  # degrees
        
        # Environmental
        self.ambient_temp = random.uniform(25, 35)  # Realistic India temp
        self.humidity = random.uniform(60, 85)  # Tropical humidity
        
        # System metrics
        self.cpu_usage = 8.0  # % - low when idle
        self.memory_usage = 35.0  # % - baseline usage
        self.uptime = 0
        self.network_latency = 5.0  # ms - base latency
        
        # Driving behavior
        self.mode = "parked"  # parked, idle, accelerating, cruising, braking
        self.mode_duration = 0
        self.target_speed = 0
        
        # Tire pressure (PSI)
        self.tire_pressure_fl = random.uniform(30, 34)
        self.tire_pressure_fr = random.uniform(30, 34)
        self.tire_pressure_rl = random.uniform(30, 34)
        self.tire_pressure_rr = random.uniform(30, 34)
        
    def update(self, dt: float = 1.0):
        """Update vehicle state with realistic physics"""
        self.uptime += dt
        
        # Mode transitions with realistic delays
        self.mode_duration += dt
        
        if self.mode == "parked" and self.mode_duration > 5:
            # Start engine after 5 seconds
            self.engine_running = True
            self.mode = "idle"
            self.mode_duration = 0
            self.gear = 0
            
        elif self.mode == "idle" and self.mode_duration > random.uniform(3, 8):
            # Shift to drive and start moving
            self.mode = "accelerating"
            self.mode_duration = 0
            self.gear = 1
            self.target_speed = random.uniform(30, 80)
            
        elif self.mode == "accelerating" and self.speed >= self.target_speed:
            self.mode = "cruising"
            self.mode_duration = 0
            
        elif self.mode == "cruising" and self.mode_duration > random.uniform(10, 20):
            # Randomly decide to brake or accelerate more
            choice = random.choice(["brake", "accelerate"])
            if choice == "brake":
                self.mode = "braking"
                self.target_speed = max(0, self.speed - random.uniform(20, 40))
            else:
                self.mode = "accelerating"
                self.target_speed = min(120, self.speed + random.uniform(10, 30))
            self.mode_duration = 0
            
        elif self.mode == "braking" and self.speed <= self.target_speed + 2:
            if self.target_speed < 5:
                self.mode = "idle"
                self.gear = 0
            else:
                self.mode = "cruising"
            self.mode_duration = 0
        
        # Update controls based on mode
        if self.mode == "parked":
            self.throttle = 0
            self.brake = 100
            self.speed = 0
            self.rpm = 0
            self.gear = 0
            
        elif self.mode == "idle":
            self.throttle = 0
            self.brake = 0
            self.speed = 0
            self.rpm = 800 + random.uniform(-30, 30) if self.engine_running else 0
            self.gear = 0
            
        elif self.mode == "accelerating":
            # Realistic acceleration
            speed_diff = self.target_speed - self.speed
            self.throttle = min(100, max(20, speed_diff * 2))
            self.brake = 0
            
            # Acceleration decreases with speed
            accel = (self.throttle / 100) * (3.5 - (self.speed / 60))
            self.speed = min(self.target_speed, self.speed + accel * dt)
            
        elif self.mode == "cruising":
            # Maintain speed with small throttle
            self.throttle = 15 + random.uniform(-3, 3)
            self.brake = 0
            self.speed += random.uniform(-0.5, 0.5)
            
        elif self.mode == "braking":
            self.throttle = 0
            speed_diff = self.speed - self.target_speed
            self.brake = min(100, max(20, speed_diff * 3))
            
            # Deceleration proportional to brake
            decel = (self.brake / 100) * 5
            self.speed = max(self.target_speed, self.speed - decel * dt)
        
        # Constrain speed
        self.speed = max(0, min(180, self.speed))
        
        # REALISTIC RPM calculation based on gear and speed
        if not self.engine_running:
            self.rpm = 0
        elif self.speed < 1:
            self.rpm = 800 + random.uniform(-30, 30)  # Idle RPM
        else:
            # Gear ratios (speed range per gear at 3000 RPM)
            gear_ratios = {
                1: (0, 25),    # 1st gear: 0-25 km/h
                2: (20, 50),   # 2nd gear: 20-50 km/h
                3: (45, 80),   # 3rd gear: 45-80 km/h
                4: (70, 110),  # 4th gear: 70-110 km/h
                5: (100, 140), # 5th gear: 100-140 km/h
                6: (120, 180)  # 6th gear: 120+ km/h
            }
            
            # Auto gear selection based on speed and throttle
            if self.speed < 25 and self.gear != 1:
                self.gear = 1
            elif 20 < self.speed < 50 and self.gear != 2:
                self.gear = 2
            elif 45 < self.speed < 80 and self.gear != 3:
                self.gear = 3
            elif 70 < self.speed < 110 and self.gear != 4:
                self.gear = 4
            elif 100 < self.speed < 140 and self.gear != 5:
                self.gear = 5
            elif self.speed >= 120:
                self.gear = 6
            
            # Calculate RPM based on current gear and speed
            if self.gear > 0:
                gear_min, gear_max = gear_ratios[self.gear]
                gear_range = gear_max - gear_min
                speed_in_gear = (self.speed - gear_min) / gear_range
                
                # RPM ranges from 1000 (low) to 4500 (high) in each gear
                base_rpm = 1000 + (speed_in_gear * 3500)
                # Add throttle influence
                rpm_boost = (self.throttle / 100) * 500
                self.rpm = int(max(800, min(6000, base_rpm + rpm_boost + random.uniform(-50, 50))))
        
        # Engine temperature - correlates with RPM and speed
        target_temp = 70 + (self.rpm / 100) + (self.speed / 10)
        target_temp = min(95, max(60, target_temp))
        
        if self.engine_temp < target_temp:
            self.engine_temp += random.uniform(0.05, 0.2)
        else:
            self.engine_temp -= random.uniform(0.02, 0.1)
        
        self.coolant_temp = self.engine_temp - random.uniform(2, 5)
        
        # Oil pressure - correlates with RPM
        if self.engine_running:
            self.oil_pressure = 20 + (self.rpm / 100) + random.uniform(-2, 2)
            self.oil_pressure = max(15, min(80, self.oil_pressure))
        else:
            self.oil_pressure = 0
        
        # Fuel consumption - realistic MPG equivalent
        if self.speed > 0:
            # L/100km consumption based on speed and throttle
            base_consumption = 0.05  # 5L/100km base
            throttle_factor = (self.throttle / 100) * 0.08
            speed_factor = abs(self.speed - 60) / 1000  # Most efficient at 60 km/h
            
            consumption_per_second = (base_consumption + throttle_factor + speed_factor) / 3600
            distance_km = (self.speed / 3600) * dt
            fuel_used = (consumption_per_second * self.speed) / 100
            self.fuel_level = max(5, self.fuel_level - fuel_used)
        
        # Battery voltage - drops slightly when engine off
        if self.engine_running:
            # Alternator charging
            self.battery_voltage = 13.8 + random.uniform(-0.1, 0.2)
        else:
            self.battery_voltage = 12.3 + random.uniform(-0.1, 0.1)
        
        # GPS movement - realistic heading and movement
        if self.speed > 1:
            # Update heading gradually
            self.heading += random.uniform(-5, 5)
            self.heading = self.heading % 360
            
            # Move in heading direction
            distance_deg = (self.speed / 111000) * dt  # 111km per degree
            self.gps_lat += distance_deg * math.cos(math.radians(self.heading))
            self.gps_lon += distance_deg * math.sin(math.radians(self.heading))
        
        # Environmental changes
        self.ambient_temp += random.uniform(-0.05, 0.05)
        self.ambient_temp = max(20, min(40, self.ambient_temp))
        self.humidity += random.uniform(-0.1, 0.1)
        self.humidity = max(40, min(95, self.humidity))
        
        # System metrics correlate with load
        self.cpu_usage = 8 + (self.speed / 5) + (self.rpm / 200) + random.uniform(-2, 2)
        self.cpu_usage = max(5, min(85, self.cpu_usage))
        self.memory_usage = 35 + random.uniform(-1, 1)
        self.memory_usage = max(30, min(60, self.memory_usage))
        
        # Network latency varies slightly
        self.network_latency = 5 + random.uniform(-2, 5)
        self.network_latency = max(1, min(50, self.network_latency))
        
        # Tire pressure - slow changes
        self.tire_pressure_fl += random.uniform(-0.01, 0.01)
        self.tire_pressure_fr += random.uniform(-0.01, 0.01)
        self.tire_pressure_rl += random.uniform(-0.01, 0.01)
        self.tire_pressure_rr += random.uniform(-0.01, 0.01)
    
    def get_telemetry(self) -> Dict[str, Any]:
        """Get current telemetry snapshot"""
        # Wheel speeds correlate with actual speed (with small variance)
        # Wheels spin slightly faster than vehicle speed
        wheel_speed_base = self.speed * 1.02
        
        diagnostics = []
        if self.engine_temp > 92:
            diagnostics.append("P0217: Engine Coolant Over Temperature")
        if self.fuel_level < 15:
            diagnostics.append("P0462: Fuel Level Sensor Low")
        if self.battery_voltage < 12.0:
            diagnostics.append("P0562: System Voltage Low")
        if self.oil_pressure < 20 and self.engine_running:
            diagnostics.append("P0524: Oil Pressure Too Low")
        if any(p < 28 for p in [self.tire_pressure_fl, self.tire_pressure_fr, self.tire_pressure_rl, self.tire_pressure_rr]):
            diagnostics.append("P0234: Tire Pressure Warning")
        
        return {
            "timestamp": datetime.utcnow().isoformat() + "Z",
            "speed": round(self.speed, 1),
            "rpm": int(self.rpm),
            "engine_temp": round(self.engine_temp, 1),
            "battery_voltage": round(self.battery_voltage, 2),
            "fuel_level": round(self.fuel_level, 1),
            "throttle_pct": round(self.throttle, 1),
            "brake_pct": round(self.brake, 1),
            "gear": self.gear,
            "traction_control": self.speed > 80 and (self.throttle > 70 or self.brake > 50),
            "cpu_usage": round(self.cpu_usage, 1),
            "memory_usage": round(self.memory_usage, 1),
            "latency_ms": round(self.network_latency, 1),
            "uptime_seconds": int(self.uptime),
            "ambient_temp": round(self.ambient_temp, 1),
            "humidity": round(self.humidity, 1),
            "gps_lat": round(self.gps_lat, 6),
            "gps_lon": round(self.gps_lon, 6),
            "wheel_speed_fl": round(wheel_speed_base + random.uniform(-0.5, 0.5), 1),
            "wheel_speed_fr": round(wheel_speed_base + random.uniform(-0.5, 0.5), 1),
            "wheel_speed_rl": round(wheel_speed_base + random.uniform(-0.5, 0.5), 1),
            "wheel_speed_rr": round(wheel_speed_base + random.uniform(-0.5, 0.5), 1),
            "diagnostics": diagnostics
        }

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
                },
                "BENYON_001": {
                    "id": "BENYON_001",
                    "name": "Benyon Test Vehicle",
                    "make": "Tesla",
                    "model": "Model 3",
                    "year": 2024,
                    "vin": "5YJ3E1EB8MF123456",
                    "license_plate": "BENYON01",
                    "status": "connected",
                    "battery_level": 88,
                    "last_updated": datetime.utcnow().isoformat()
                }
            },
            "telemetry": {}
        }
        self.load_from_file()
        
        # Ensure BENYON_001 exists (in case old data.json doesn't have it)
        if "BENYON_001" not in self.data["vehicles"]:
            self.data["vehicles"]["BENYON_001"] = {
                "id": "BENYON_001",
                "name": "Benyon Test Vehicle",
                "make": "Tesla",
                "model": "Model 3",
                "year": 2024,
                "vin": "5YJ3E1EB8MF123456",
                "license_plate": "BENYON01",
                "status": "connected",
                "battery_level": 88,
                "last_updated": datetime.utcnow().isoformat()
            }
            self.save_to_file()
            print("[STORAGE] ✅ Added BENYON_001 vehicle to database")

    def load_from_file(self):
        if DATA_FILE.exists():
            try:
                with open(DATA_FILE, "r") as f:
                    loaded_data = json.load(f)
                    # Merge loaded data with defaults
                    self.data.update(loaded_data)
                    print(f"[STORAGE] 📂 Loaded data from {DATA_FILE}")
            except Exception as e:
                print(f"[STORAGE] ⚠️ Error loading file: {e}")
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
simulators: Dict[str, VehicleSimulator] = {}

# ==================== BACKGROUND TASK ====================
async def generate_telemetry():
    """Background task to continuously generate telemetry data"""
    print("[SIMULATOR] 🚀 Starting telemetry generation...")
    
    # Initialize simulators for all vehicles
    for vehicle_id in store.get_vehicles().keys():
        simulators[vehicle_id] = VehicleSimulator(vehicle_id)
        print(f"[SIMULATOR] ✅ Created simulator for {vehicle_id}")
    
    # Counter for periodic logging
    log_counter = 0
    
    while True:
        try:
            for vehicle_id, simulator in simulators.items():
                simulator.update(1.0)  # Update with 1 second delta
                telemetry_data = simulator.get_telemetry()
                
                # Store telemetry
                telemetry = TelemetryData(**telemetry_data)
                store.add_telemetry(vehicle_id, telemetry)
                
                # Log telemetry every 5 seconds for BENYON_001
                if vehicle_id == "BENYON_001" and log_counter % 5 == 0:
                    print(f"\n[TELEMETRY] {vehicle_id} - Mode: {simulator.mode}")
                    print(f"  📊 Speed: {telemetry_data['speed']} km/h | RPM: {telemetry_data['rpm']} | Gear: {telemetry_data['gear']}")
                    print(f"  🎮 Throttle: {telemetry_data['throttle_pct']}% | Brake: {telemetry_data['brake_pct']}%")
                    print(f"  🌡️  Engine: {telemetry_data['engine_temp']}°C | Battery: {telemetry_data['battery_voltage']}V")
                    print(f"  ⛽ Fuel: {telemetry_data['fuel_level']}% | Traction: {telemetry_data['traction_control']}")
                    if telemetry_data['diagnostics']:
                        print(f"  ⚠️  Diagnostics: {', '.join(telemetry_data['diagnostics'])}")
            
            log_counter += 1
            await asyncio.sleep(1)  # Update every second
        except Exception as e:
            print(f"[SIMULATOR] ❌ Error: {e}")
            import traceback
            traceback.print_exc()
            await asyncio.sleep(1)

@asynccontextmanager
async def lifespan(app: FastAPI):
    """Lifespan context manager for startup and shutdown"""
    # Startup
    print("[SERVER] 🚀 Starting up...")
    task = asyncio.create_task(generate_telemetry())
    print("[SERVER] ✅ Backend started successfully")
    yield
    # Shutdown
    task.cancel()
    print("[SERVER] 👋 Shutting down...")

app = FastAPI(title="Kintsugi - Vehicle Telemetry Backend", version="1.0.0", lifespan=lifespan)

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
        print(f"[AUTH] Verifying token: {token[:20]}...")
        payload = jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])
        email: str = payload.get("sub")
        if email is None:
            print(f"[AUTH] ❌ Token missing 'sub' field")
            raise HTTPException(status_code=401, detail="Invalid token")
        print(f"[AUTH] ✅ Token valid for: {email}")
        return payload
    except jwt.ExpiredSignatureError:
        print(f"[AUTH] ❌ Token expired")
        raise HTTPException(status_code=401, detail="Token expired")
    except jwt.InvalidTokenError as e:
        print(f"[AUTH] ❌ Invalid token: {e}")
        raise HTTPException(status_code=401, detail="Invalid token")

def get_token_from_header(authorization: Optional[str] = Header(None)) -> Optional[str]:
    """Extract token from Authorization header"""
    print(f"[AUTH] Authorization header: {authorization[:30] if authorization else 'None'}...")
    if not authorization:
        return None
    
    # Handle "Bearer <token>" format
    if authorization.startswith("Bearer "):
        token = authorization[7:]
        print(f"[AUTH] Extracted token: {token[:20]}...")
        return token
    
    return authorization

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
async def get_vehicles(authorization: Optional[str] = Header(None)):
    token = get_token_from_header(authorization)
    if token:
        verify_token(token)
    vehicles = list(store.get_vehicles().values())
    return {"vehicles": vehicles}

@app.get("/api/v1/vehicles/{vehicle_id}")
async def get_vehicle(vehicle_id: str, authorization: Optional[str] = Header(None)):
    token = get_token_from_header(authorization)
    if token:
        verify_token(token)
    vehicle = store.get_vehicle(vehicle_id)
    if not vehicle:
        raise HTTPException(status_code=404, detail="Vehicle not found")
    return vehicle

# ==================== TELEMETRY ROUTES ====================
@app.get("/api/v1/telemetry/{vehicle_id}/current")
async def get_current_telemetry(vehicle_id: str, authorization: Optional[str] = Header(None)):
    token = get_token_from_header(authorization)
    if token:
        verify_token(token)
    telemetry = store.get_telemetry(vehicle_id, limit=1)
    return {"telemetry": telemetry[-1] if telemetry else None}

@app.get("/api/v1/telemetry/{vehicle_id}/history")
async def get_telemetry_history(
    vehicle_id: str,
    start_date: str,
    end_date: str,
    limit: int = 100,
    authorization: Optional[str] = Header(None)
):
    token = get_token_from_header(authorization)
    if token:
        verify_token(token)
    telemetry = store.get_telemetry_history(vehicle_id, start_date, end_date)
    return {"telemetry": telemetry[-limit:]}

@app.post("/api/v1/telemetry/{vehicle_id}/report")
async def report_telemetry(vehicle_id: str, data: TelemetryData, authorization: Optional[str] = Header(None)):
    token = get_token_from_header(authorization)
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

@app.get("/debug/simulators")
async def debug_simulators():
    """Debug endpoint to check simulator status"""
    simulator_status = {}
    for vehicle_id, simulator in simulators.items():
        simulator_status[vehicle_id] = {
            "speed": simulator.speed,
            "rpm": simulator.rpm,
            "mode": simulator.mode,
            "uptime": simulator.uptime
        }
    
    telemetry_counts = {}
    for vehicle_id in store.get_vehicles().keys():
        telemetry_counts[vehicle_id] = len(store.get_telemetry(vehicle_id, limit=1000))
    
    return {
        "simulators_running": len(simulators),
        "simulator_status": simulator_status,
        "telemetry_record_counts": telemetry_counts,
        "vehicles_in_db": list(store.get_vehicles().keys())
    }

@app.get("/")
async def root():
    return {"message": "Vehicle Telemetry Backend API - Running on localhost:8000"}
