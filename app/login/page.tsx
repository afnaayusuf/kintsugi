"use client"

import type React from "react"

import { useState } from "react"
import { useRouter } from "next/navigation"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { setStoredToken, apiCall } from "@/lib/api"
import { useVehicleStore } from "@/lib/store"

export default function LoginPage() {
  const router = useRouter()
  const setAuthenticated = useVehicleStore((state) => state.setAuthenticated)
  const [email, setEmail] = useState("admin@hybrid-drive.io")
  const [password, setPassword] = useState("demo123")
  const [error, setError] = useState("")
  const [isLoading, setIsLoading] = useState(false)

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault()
    setError("")
    setIsLoading(true)

    try {
      const response = await apiCall<{ access_token: string; token_type: string; user_id: string }>("/auth/login", {
        method: "POST",
        body: JSON.stringify({ email, password }),
      })

      console.log("[Login] ✅ Login successful, token:", response.access_token.substring(0, 20) + "...")
      setStoredToken(response.access_token)
      setAuthenticated(true)
      router.push("/dashboard")
    } catch (err) {
      console.error("[Login] ❌ Login failed:", err)
      setError(err instanceof Error ? err.message : "Login failed")
    } finally {
      setIsLoading(false)
    }
  }

  const handleDemoLogin = () => {
    setStoredToken("demo_token_12345")
    setAuthenticated(true)
    router.push("/dashboard")
  }

  return (
    <div className="min-h-screen flex items-center justify-center bg-gradient-to-br from-background to-muted p-4">
      <Card className="w-full max-w-md border border-border">
        <CardHeader className="space-y-2">
          <CardTitle className="text-2xl text-balance">Vehicle Telemetry</CardTitle>
          <CardDescription>Sign in to access your fleet dashboard</CardDescription>
        </CardHeader>
        <CardContent>
          <form onSubmit={handleLogin} className="space-y-4">
            <div className="space-y-2">
              <label htmlFor="email" className="text-sm font-medium">
                Email
              </label>
              <Input
                id="email"
                type="email"
                placeholder="you@example.com"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                required
              />
            </div>
            <div className="space-y-2">
              <label htmlFor="password" className="text-sm font-medium">
                Password
              </label>
              <Input
                id="password"
                type="password"
                placeholder="••••••••"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                required
              />
            </div>
            {error && <div className="text-sm text-destructive">{error}</div>}
            <Button type="submit" className="w-full" disabled={isLoading}>
              {isLoading ? "Signing in..." : "Sign in"}
            </Button>
          </form>

          <div className="mt-6 pt-6 border-t border-border">
            <p className="text-xs text-muted-foreground mb-3">Demo Mode</p>
            <Button type="button" variant="outline" className="w-full bg-transparent" onClick={handleDemoLogin}>
              Try Demo (Mock Data)
            </Button>
            <p className="text-xs text-muted-foreground mt-2">Test credentials: admin@hybrid-drive.io / demo123</p>
          </div>
        </CardContent>
      </Card>
    </div>
  )
}
