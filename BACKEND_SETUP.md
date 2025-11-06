# Vehicle Telemetry Backend Setup

## Quick Start (Laptop/Local Development)

### Option 1: Direct Python (Recommended for Development)

1. **Install Python 3.11+**
   \`\`\`bash
   python --version  # Should be 3.11 or higher
   \`\`\`

2. **Navigate to backend directory**
   \`\`\`bash
   cd backend
   \`\`\`

3. **Create virtual environment**
   \`\`\`bash
   python -m venv venv
   
   # On Windows
   venv\Scripts\activate
   
   # On macOS/Linux
   source venv/bin/activate
   \`\`\`

4. **Install dependencies**
   \`\`\`bash
   pip install -r requirements.txt
   \`\`\`

5. **Run the backend**
   \`\`\`bash
   uvicorn main:app --host 0.0.0.0 --port 8000 --reload
   \`\`\`

   Backend will be available at: **http://localhost:8000**

### Option 2: Docker Compose (All-in-One)

1. **Make sure Docker is running**

2. **From project root, run:**
   \`\`\`bash
   docker-compose up
   \`\`\`

   - Frontend: http://localhost:3000
   - Backend: http://localhost:8000

## API Endpoints

### Authentication
- `POST /api/v1/auth/login` - Login with email and password
- `POST /api/v1/auth/verify` - Verify JWT token

### Vehicles
- `GET /api/v1/vehicles` - Get all vehicles
- `GET /api/v1/vehicles/{vehicle_id}` - Get specific vehicle

### Telemetry
- `GET /api/v1/telemetry/{vehicle_id}/current` - Get latest telemetry
- `GET /api/v1/telemetry/{vehicle_id}/history` - Get historical data
- `POST /api/v1/telemetry/{vehicle_id}/report` - Submit telemetry data

### WebSocket
- `WS /ws/telemetry/{vehicle_id}` - Real-time telemetry streaming

## Test Credentials

\`\`\`
Email: admin@hybrid-drive.io
Password: demo123
\`\`\`

## Data Storage

- In-memory storage with JSON file persistence (`data.json`)
- Data persists across restarts
- Telemetry keeps last 1000 entries per vehicle
- No external database required

## Environment Variables

Copy `.env.example` to `.env` and configure as needed:
\`\`\`bash
cp backend/.env.example backend/.env
\`\`\`

Key variables:
- `SECRET_KEY` - JWT secret (change in production!)
- `ALGORITHM` - JWT algorithm (default: HS256)
- `ACCESS_TOKEN_EXPIRE_MINUTES` - Token expiry time (default: 1440)

## Frontend Configuration

Update your frontend environment variables:
\`\`\`
NEXT_PUBLIC_API_URL=http://localhost:8000/api/v1
NEXT_PUBLIC_WS_URL=ws://localhost:8000/ws
\`\`\`

## Testing the API

### Using curl
\`\`\`bash
# Login
curl -X POST http://localhost:8000/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"admin@hybrid-drive.io","password":"demo123"}'

# Get vehicles
curl http://localhost:8000/api/v1/vehicles

# Get current telemetry
curl http://localhost:8000/api/v1/telemetry/tesla-1/current
\`\`\`

### Using Python requests
\`\`\`python
import requests

# Login
response = requests.post('http://localhost:8000/api/v1/auth/login', json={
    'email': 'admin@hybrid-drive.io',
    'password': 'demo123'
})
token = response.json()['access_token']

# Get vehicles
response = requests.get('http://localhost:8000/api/v1/vehicles', 
                       params={'token': token})
print(response.json())
\`\`\`

## Troubleshooting

**Port 8000 already in use:**
\`\`\`bash
# Find process using port 8000
lsof -i :8000  # macOS/Linux
netstat -ano | findstr :8000  # Windows

# Kill process or use different port
uvicorn main:app --port 8001
\`\`\`

**Import errors:**
\`\`\`bash
pip install --upgrade pip
pip install -r requirements.txt --force-reinstall
\`\`\`

**WebSocket connection fails:**
- Check that backend is running on port 8000
- Verify CORS is properly configured
- Check browser console for connection errors

## Production Deployment

Before deploying to production:

1. Change `SECRET_KEY` to a strong random string
2. Set `ALGORITHM` appropriately
3. Use a proper database (PostgreSQL recommended)
4. Set `DEBUG=False`
5. Configure proper logging
6. Use environment-specific settings

## Performance Notes

- Current setup: **100+ requests/sec** on standard laptop
- Memory usage: **~50-100MB** (Python process + data)
- JSON persistence: **~1-2KB per telemetry entry**
- Suitable for: Development, testing, small-scale deployments

## Next Steps

1. Test the backend with the provided test credentials
2. Connect the frontend to the backend via API_URL env var
3. Verify real-time WebSocket updates in the dashboard
4. Add your own vehicle data as needed
5. Scale to production database when ready
