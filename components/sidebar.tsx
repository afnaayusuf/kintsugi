"use client"

import { useEffect, useState } from "react"
import Link from "next/link"
import { useVehicleStore } from "@/lib/store"
import { mockVehicles } from "@/lib/mock-data"
import { Button } from "@/components/ui/button"
import { Card } from "@/components/ui/card"
import { Loader2, LogOut, Gauge } from "lucide-react"

export function Sidebar() {
  const vehicles = useVehicleStore((state) => state.vehicles)
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const setVehicles = useVehicleStore((state) => state.setVehicles)
  const selectVehicle = useVehicleStore((state) => state.selectVehicle)
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    // Use mock data for development
    setVehicles(mockVehicles)
    if (!selectedVehicleId && mockVehicles.length > 0) {
      selectVehicle(mockVehicles[0].id)
    }
    setLoading(false)
  }, [])

  const handleLogout = () => {
    localStorage.removeItem("telemetry_token")
    window.location.href = "/login"
  }

  return (
    <div className="w-64 border-r border-border bg-card h-screen overflow-y-auto p-4 flex flex-col">
      <Link href="/dashboard" className="flex items-center gap-2 mb-8">
        <div className="flex items-center justify-center w-10 h-10 rounded-lg bg-gradient-to-br from-accent to-accent/50">
          <Gauge className="w-6 h-6 text-accent-foreground" />
        </div>
        <span className="text-xl font-bold bg-gradient-to-r from-accent to-primary bg-clip-text text-transparent">
          Kintsugi
        </span>
      </Link>

      <nav className="space-y-2 flex-1">
        <div className="text-xs uppercase font-semibold text-muted-foreground mb-3 tracking-widest">Navigation</div>
        <Link href="/dashboard">
          <Button variant="ghost" className="w-full justify-start hover:bg-primary/20">
            Dashboard
          </Button>
        </Link>
        <Link href="/history">
          <Button variant="ghost" className="w-full justify-start hover:bg-primary/20">
            History
          </Button>
        </Link>
        <Link href="/settings">
          <Button variant="ghost" className="w-full justify-start hover:bg-primary/20">
            Settings
          </Button>
        </Link>
      </nav>

      <div className="space-y-3 mt-8 pt-4 border-t border-border">
        <div className="text-xs uppercase font-semibold text-muted-foreground tracking-widest">Vehicles</div>
        {loading ? (
          <div className="flex items-center justify-center p-4">
            <Loader2 className="w-4 h-4 animate-spin" />
          </div>
        ) : (
          <div className="space-y-1">
            {vehicles.map((vehicle) => (
              <Card
                key={vehicle.id}
                onClick={() => selectVehicle(vehicle.id)}
                className={`p-3 cursor-pointer transition-all ${
                  selectedVehicleId === vehicle.id
                    ? "bg-gradient-to-r from-primary to-primary/80 text-primary-foreground border-primary shadow-lg shadow-primary/50"
                    : "bg-secondary/50 text-secondary-foreground hover:bg-secondary border-transparent hover:border-primary/50"
                }`}
              >
                <div className="text-sm font-medium text-balance">{vehicle.id}</div>
                <div className="text-xs opacity-75">{vehicle.model}</div>
                <div
                  className={`text-xs mt-1 font-semibold ${vehicle.status === "online" ? "text-green-400" : "text-red-400"}`}
                >
                  {vehicle.status === "online" ? "● Online" : "● Offline"}
                </div>
              </Card>
            ))}
          </div>
        )}
      </div>

      <Button variant="outline" onClick={handleLogout} className="w-full mt-4 bg-transparent hover:bg-destructive/20">
        <LogOut className="w-4 h-4 mr-2" />
        Logout
      </Button>
    </div>
  )
}
