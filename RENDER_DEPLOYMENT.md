# Deploying Kintsugi Backend to Render

## Prerequisites
- GitHub account with repository containing backend code
- Render account (free tier available)

## Step-by-Step Deployment

### 1. Push Code to GitHub
\`\`\`bash
git add .
git commit -m "Prepare for Render deployment"
git push origin main
\`\`\`

### 2. Create Render Account
Visit: https://render.com and sign up

### 3. Connect GitHub Repository
- Go to Render Dashboard
- Click "New +" → "Web Service"
- Select "Connect a Repository"
- Choose your GitHub repository

### 4. Configure Web Service
**Name:** kintsugi-backend

**Environment:** Python 3

**Build Command:**
\`\`\`
pip install -r requirements.txt
\`\`\`

**Start Command:**
\`\`\`
uvicorn main:app --host 0.0.0.0 --port 8000
\`\`\`

### 5. Add Environment Variables
In Render Dashboard, click "Environment" and add:

| Key | Value |
|-----|-------|
| SECRET_KEY | your-production-secret-key-change-this |
| ALGORITHM | HS256 |
| ACCESS_TOKEN_EXPIRE_MINUTES | 1440 |
| ALLOWED_ORIGINS | https://express-js-on-vercel-agp6.vercel.app,https://kintsugi-backend.onrender.com |

### 6. Deploy
- Click "Create Web Service"
- Render automatically deploys
- Your backend URL: `https://kintsugi-backend.onrender.com`

## Verify Deployment

### Check Health
\`\`\`bash
curl https://kintsugi-backend.onrender.com/health
\`\`\`

Response should be:
\`\`\`json
{"status": "healthy", "timestamp": "2024-..."}
\`\`\`

### Test Login
\`\`\`bash
curl -X POST https://kintsugi-backend.onrender.com/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "email": "admin@hybrid-drive.io",
    "password": "demo123"
  }'
\`\`\`

### Test Vehicles
\`\`\`bash
curl https://kintsugi-backend.onrender.com/api/v1/vehicles
\`\`\`

## Cold Start Optimization

Free tier Render services may sleep after 15 minutes of inactivity:
- First request may take 30-60 seconds
- Consider upgrading to paid tier for production

## Monitoring

In Render Dashboard:
- View logs in real-time
- Monitor CPU/Memory usage
- Check deployment history
- Set up alerts (paid feature)

## Updating Backend

After making code changes:
1. Commit and push to GitHub
2. Render automatically redeploys
3. Or manually trigger redeploy in dashboard

## Troubleshooting

**Build fails:**
- Check Python version is 3.11+
- Verify requirements.txt is valid
- Check for syntax errors in main.py

**Service crashes:**
- Check Render logs for errors
- Verify environment variables are set
- Test locally first with `uvicorn main:app`

**CORS errors from frontend:**
- Verify ALLOWED_ORIGINS includes Vercel domain
- Restart the service after updating env vars

## Performance Notes

- Response time: 100-500ms (includes Render cold start)
- Concurrent connections: 100+
- Data persistence: JSON file (persists across restarts)
- Upgrade to PostgreSQL for production scale

## Cost Estimation (Render Free Tier)
- Backend service: Free
- Cold starts after 15 min inactivity
- 750 compute hours/month
- Perfect for development and testing

---

For Vercel frontend deployment, see `VERCEL_DEPLOYMENT.md`
