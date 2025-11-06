# Deploying Kintsugi Frontend to Vercel

## Prerequisites
- GitHub account with repository
- Vercel account (free tier available)
- Backend deployed to Render (or known backend URL)

## Step-by-Step Deployment

### 1. Prepare Code
Ensure your `.env` variables are configured:
\`\`\`env
NEXT_PUBLIC_API_URL=https://kintsugi-backend.onrender.com/api/v1
NEXT_PUBLIC_WS_URL=wss://kintsugi-backend.onrender.com/ws
\`\`\`

### 2. Push to GitHub
\`\`\`bash
git add .
git commit -m "Prepare for Vercel deployment"
git push origin main
\`\`\`

### 3. Create Vercel Account
Visit: https://vercel.com and sign up with GitHub

### 4. Import Project
- Click "Add New..." → "Project"
- Select your GitHub repository
- Vercel auto-detects Next.js

### 5. Configure Environment Variables
In project settings, add:

| Name | Value |
|------|-------|
| NEXT_PUBLIC_API_URL | https://kintsugi-backend.onrender.com/api/v1 |
| NEXT_PUBLIC_WS_URL | wss://kintsugi-backend.onrender.com/ws |

### 6. Deploy
- Click "Deploy"
- Vercel automatically builds and deploys
- Your frontend URL: `https://express-js-on-vercel-agp6.vercel.app`

## Verify Deployment

1. Visit your Vercel URL
2. Login with test credentials:
   - Email: `admin@hybrid-drive.io`
   - Password: `demo123`
3. Verify telemetry dashboard loads

## Custom Domain (Optional)

In Vercel project settings:
1. Go to "Domains"
2. Add your custom domain
3. Follow DNS configuration instructions

## Continuous Deployment

- Every push to `main` branch auto-deploys
- View deployment history in Vercel dashboard
- Rollback to previous versions if needed

## Performance Optimization

Vercel automatically optimizes:
- Image optimization
- Code splitting
- Edge functions
- Caching

Your site is served from 300+ global edge locations.

## Environment-Specific Overrides

Create `vercel.json` for advanced config:
\`\`\`json
{
  "env": {
    "NEXT_PUBLIC_API_URL": "@api_url",
    "NEXT_PUBLIC_WS_URL": "@ws_url"
  }
}
\`\`\`

## Analytics & Monitoring

Vercel provides:
- Page performance metrics
- Core Web Vitals
- Real User Monitoring (RUM)
- Edge request logs

## Cost Estimation (Vercel Free Tier)
- Hosting: Free
- Bandwidth: 100GB/month
- Build minutes: 6000/month
- Perfect for production apps

## Troubleshooting

**Build fails:**
- Check `npm run build` works locally
- Verify `next.config.js` syntax
- Check for missing dependencies

**API calls fail:**
- Verify backend URL is accessible
- Check CORS configuration on backend
- Ensure environment variables are set

**WebSocket not connecting:**
- Use `wss://` for production (WSS = WebSocket Secure)
- Verify backend WebSocket endpoint is working

## Updating Frontend

After code changes:
1. Commit and push to GitHub
2. Vercel automatically redeploys
3. Deployment status visible in dashboard

---

For backend deployment on Render, see `RENDER_DEPLOYMENT.md`
