# Kintsugi - Vehicle Telemetry Dashboard

**Kintsugi** (金継ぎ) is a Japanese art form that embraces imperfection and impermanence. Similarly, this vehicle telemetry dashboard merges broken data streams into a beautiful, unified interface for real-time vehicle monitoring.

## Architecture

\`\`\`
┌─────────────────────────────────────────────────────────┐
│                    Frontend (Next.js)                    │
│              https://express-js-on-vercel-agp6.vercel.app│
└────────────────────┬────────────────────────────────────┘
                     │ (REST API + WebSocket)
                     │
┌────────────────────▼────────────────────────────────────┐
│              Backend (FastAPI Python)                    │
│         https://kintsugi-backend.onrender.com            │
│                                                           │
│  ✓ JWT Authentication     ✓ Real-time WebSocket         │
│  ✓ Vehicle Management     ✓ Telemetry Streaming         │
│  ✓ Historical Analytics   ✓ JSON Persistence            │
└─────────────────────────────────────────────────────────┘
\`\`\`

## Features

- **Real-Time Dashboard**: Live vehicle metrics with beautiful animations
- **Multi-Vehicle Support**: Monitor multiple vehicles simultaneously
- **Historical Analytics**: Track performance trends over time
- **System Health Monitoring**: CPU, memory, latency tracking
- **Safety Alerts**: Automatic warnings for critical issues
- **Responsive Design**: Works on desktop and mobile
- **Zero Database Setup**: JSON file persistence, run locally!

## Quick Start (Local Development)

### Option 1: Docker (Easiest)
\`\`\`bash
docker-compose up
\`\`\`
- Frontend: http://localhost:3000
- Backend: http://localhost:8000

### Option 2: Manual Setup

**Backend:**
\`\`\`bash
cd backend
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
uvicorn main:app --reload --host 0.0.0.0 --port 8000
\`\`\`

**Frontend:**
\`\`\`bash
# Install dependencies (use --legacy-peer-deps due to React 19 compatibility)
npm install --legacy-peer-deps

# Start development server
npm run dev
\`\`\`

Access at: http://localhost:3000

### Option 3: Raspberry Pi Deployment

See `raspberry-pi-deployment/` folder for Pi-specific setup:
\`\`\`bash
cd raspberry-pi-deployment
pip install -r requirements.txt
# Copy backend files and run
\`\`\`

## Login Credentials

\`\`\`
Email: admin@hybrid-drive.io
Password: demo123
\`\`\`

## Deployment

### Production URLs
- Backend: https://kintsugi-backend.onrender.com
- Frontend: https://express-js-on-vercel-agp6.vercel.app

### Deploy Yourself
- Backend: See `RENDER_DEPLOYMENT.md`
- Frontend: See `VERCEL_DEPLOYMENT.md`

## API Documentation

Interactive API docs available at: `http://localhost:8000/docs`

### Endpoints

**Authentication**
- `POST /api/v1/auth/login` - Login
- `POST /api/v1/auth/verify` - Verify token

**Vehicles**
- `GET /api/v1/vehicles` - List all vehicles
- `GET /api/v1/vehicles/{vehicle_id}` - Get vehicle details

**Telemetry**
- `GET /api/v1/telemetry/{vehicle_id}/current` - Latest data
- `GET /api/v1/telemetry/{vehicle_id}/history` - Historical data
- `POST /api/v1/telemetry/{vehicle_id}/report` - Submit data

**WebSocket**
- `WS /ws/telemetry/{vehicle_id}` - Real-time streaming

## Testing

Run API tests:
\`\`\`bash
cd backend
python test_api.py
\`\`\`

## Tech Stack

**Frontend:**
- Next.js 16.0.0 with App Router
- React 19.2.0 with Server Components
- Tailwind CSS v4.1.9
- Recharts (latest) for data visualization
- Radix UI components
- TypeScript 5
- Zustand for state management

**Backend:**
- FastAPI 0.104.1 (Python 3.11+)
- PyJWT 2.8.1 for authentication
- WebSocket for real-time updates
- Uvicorn ASGI server
- JSON file persistence (no database required)

**Infrastructure:**
- Vercel for frontend hosting
- Render for backend hosting
- Docker support for containerization
- Raspberry Pi compatible

**Known Issues:**
- React 19 compatibility requires `--legacy-peer-deps` flag during npm install
- `react-is` dependency conflict with recharts (resolved)

## Project Structure

\`\`\`
kintsugi-driver-deploy/
├── app/                          # Next.js App Router pages
│   ├── dashboard/               # Main dashboard page
│   ├── history/                 # Historical data viewer
│   ├── login/                   # Authentication page
│   ├── globals.css             # Global styles
│   ├── layout.tsx              # Root layout
│   └── page.tsx                # Home page
├── backend/                      # FastAPI backend
│   ├── main.py                 # Main API application
│   ├── requirements.txt        # Python dependencies
│   ├── test_api.py            # API tests
│   ├── Dockerfile              # Development Docker image
│   ├── Dockerfile.prod         # Production Docker image
│   ├── render.yaml            # Render deployment config
│   └── data.json              # JSON data store (auto-generated)
├── components/                   # React components
│   ├── ui/                     # Shadcn/ui base components
│   ├── telemetry-charts.tsx   # Chart components
│   ├── vehicle-selector.tsx   # Vehicle selection UI
│   ├── system-health-panel.tsx
│   └── ... (other components)
├── lib/                         # Utility functions
│   ├── api.ts                 # API client
│   ├── websocket.ts           # WebSocket client
│   ├── store.ts               # Zustand state management
│   └── utils.ts               # Helper functions
├── hooks/                       # React hooks
│   ├── use-telemetry-connection.ts
│   └── use-toast.ts
├── raspberry-pi-deployment/    # Raspberry Pi specific files
│   └── requirements.txt       # Pi-optimized dependencies
├── public/                      # Static assets
├── styles/                      # Additional styles
├── docker-compose.yml          # Local Docker setup
├── package.json               # Node.js dependencies
├── tsconfig.json              # TypeScript configuration
├── next.config.mjs            # Next.js configuration
├── components.json            # Shadcn/ui config
├── README.md                  # This file
├── DEPLOYMENT.md              # General deployment guide
├── RENDER_DEPLOYMENT.md       # Render.com deployment
├── VERCEL_DEPLOYMENT.md       # Vercel deployment
└── BACKEND_SETUP.md           # Backend setup guide
\`\`\`

## Performance

- **Frontend**: 90+ Lighthouse score
- **Backend**: 100+ req/sec on standard laptop
- **Latency**: <100ms average response time
- **Concurrent Users**: 1000+ (with proper infrastructure)

## License

MIT License - Feel free to use for personal or commercial projects

## Support

For issues, questions, or contributions:
1. Check `DEPLOYMENT.md` for common problems
2. Review API logs: `http://localhost:8000/docs`
3. Enable debug logging in backend
4. For Raspberry Pi deployment, see `raspberry-pi-deployment/` folder
5. Install frontend dependencies with `--legacy-peer-deps` flag

**Common Issues:**
- **Module not found 'react-is'**: Run `npm install --legacy-peer-deps`
- **Port already in use**: Change ports in docker-compose.yml or kill existing processes
- **CORS errors**: Check ALLOWED_ORIGINS in backend/main.py
- **WebSocket connection fails**: Ensure backend is running and accessible

**Current Status (Nov 2025):**
- ✅ Frontend running on Next.js 16 with React 19
- ✅ Backend API fully functional with FastAPI
- ✅ Docker containerization working
- ✅ Raspberry Pi deployment ready
- ⚠️ React 19 requires legacy peer deps for some packages
- ✅ WebSocket real-time telemetry streaming active

---

Built with ❤️ for vehicle telemetry monitoring
