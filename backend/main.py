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
