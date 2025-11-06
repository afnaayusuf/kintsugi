# Kintsugi Vehicle Telemetry Dashboard - Deployment Guide

## Overview
Kintsugi is a full-stack vehicle telemetry dashboard built with:
- **Frontend**: Next.js (deployed on Vercel)
- **Backend**: FastAPI Python 3 (deployed on Render)
- **Storage**: JSON file persistence (no database required)

## Live Deployment

### Backend (Render)
- **URL**: https://kintsugi-backend.onrender.com
- **API Docs**: https://kintsugi-backend.onrender.com/docs
- **Health Check**: https://kintsugi-backend.onrender.com/health

### Frontend (Vercel)
- **URL**: https://express-js-on-vercel-agp6.vercel.app

---

## Local Development Setup

### Prerequisites
- Python 3.11+
- Node.js 18+
- Docker & Docker Compose (optional)

### Option 1: Manual Setup

**Backend:**
\`\`\`bash
cd backend
pip install -r requirements.txt
export SECRET_KEY="your-dev-secret-key"
export ALLOWED_ORIGINS="http://localhost:3000"
uvicorn main:app --reload
# Backend runs at http://localhost:8000
\`\`\`

**Frontend:**
\`\`\`bash
npm install
export NEXT_PUBLIC_API_URL="http://localhost:8000/api/v1"
export NEXT_PUBLIC_WS_URL="ws://localhost:8000/ws"
npm run dev
# Frontend runs at http://localhost:3000
\`\`\`

### Option 2: Docker Compose
\`\`\`bash
docker-compose up
# Both services start automatically
# Frontend: http://localhost:3000
# Backend: http://localhost:8000
\`\`\`

---

## Production Deployment

### Backend on Render

1. **Create Render Web Service**
   - Connect GitHub repository
   - Runtime: Python 3.11
   - Build Command: `pip install -r requirements.txt`
   - Start Command: `uvicorn main:app --host 0.0.0.0 --port 8000`

2. **Environment Variables**
   \`\`\`
   SECRET_KEY=your-production-secret-key-here
   ALGORITHM=HS256
   ACCESS_TOKEN_EXPIRE_MINUTES=1440
   ALLOWED_ORIGINS=https://express-js-on-vercel-agp6.vercel.app,https://kintsugi-backend.onrender.com
   \`\`\`

3. **Deploy**
   - Push to main branch
   - Render automatically deploys

### Frontend on Vercel

1. **Connect Vercel**
   - Import GitHub repository
   - Framework: Next.js
   - Vercel auto-detects configuration

2. **Environment Variables**
   \`\`\`
   NEXT_PUBLIC_API_URL=https://kintsugi-backend.onrender.com/api/v1
   NEXT_PUBLIC_WS_URL=wss://kintsugi-backend.onrender.com/ws
   \`\`\`

3. **Deploy**
   - Push to main branch
   - Vercel automatically deploys

---

## Login Credentials

**Demo Account:**
- Email: `admin@hybrid-drive.io`
- Password: `demo123`

---

## API Endpoints

### Authentication
- `POST /api/v1/auth/login` - User login
- `POST /api/v1/auth/verify` - Verify token

### Vehicles
- `GET /api/v1/vehicles` - List all vehicles
- `GET /api/v1/vehicles/{vehicle_id}` - Get vehicle details

### Telemetry
- `GET /api/v1/telemetry/{vehicle_id}/current` - Get latest telemetry
- `GET /api/v1/telemetry/{vehicle_id}/history` - Get historical data
- `POST /api/v1/telemetry/{vehicle_id}/report` - Report telemetry data

### WebSocket
- `WS /ws/telemetry/{vehicle_id}` - Real-time telemetry streaming

---

## Troubleshooting

### CORS Errors
Ensure `ALLOWED_ORIGINS` environment variable includes your frontend domain.

### WebSocket Connection Fails
Check that WebSocket URL uses `wss://` (secure) for production and `ws://` for localhost.

### Data Not Persisting
Backend stores data in `data.json`. Ensure `/app` directory has write permissions.

### Cold Start on Render
First request may be slow (30s+). This is normal for free tier Render services.

---

## Data Management

The backend uses JSON file persistence:
- All data stored in `backend/data.json`
- Automatically created on first run
- Last 1000 telemetry entries per vehicle
- No database setup required

---

## Contact & Support

For issues or questions, refer to the repository documentation or open an issue.
\`\`\`

```bash file="" isHidden
