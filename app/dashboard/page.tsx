"use client"

import { useEffect } from "react"
import { useRouter } from "next/navigation"
import { getStoredToken } from "@/lib/api"
import { useVehicleStore } from "@/lib/store"
import { mockVehicles } from "@/lib/mock-data"
import { useTelemetryConnection } from "@/hooks/use-telemetry-connection"
import { Sidebar } from "@/components/sidebar"
import { TopNavigation } from "@/components/top-navigation"
import { VehicleBanner } from "@/components/vehicle-banner"
import { TelemetryCharts } from "@/components/telemetry-charts"
import { SystemHealthPanel } from "@/components/system-health-panel"
import { EnvironmentPanel } from "@/components/environment-panel"
import { SafetyAlerts } from "@/components/safety-alerts"

export default function DashboardPage() {
  const router = useRouter()
  const setVehicles = useVehicleStore((state) => state.setVehicles)
  const selectVehicle = useVehicleStore((state) => state.selectVehicle)
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const setAuthenticated = useVehicleStore((state) => state.setAuthenticated)
  const isAuthenticated = useVehicleStore((state) => state.isAuthenticated)

  useTelemetryConnection()

  useEffect(() => {
    const token = getStoredToken()
    console.log("[Dashboard] Token check:", token ? "EXISTS" : "MISSING")
    
    if (!token) {
      console.log("[Dashboard] ❌ No token, redirecting to login")
      router.push("/login")
      return
    }

    console.log("[Dashboard] ✅ Token exists, setting authenticated = true")
    setAuthenticated(true)

    // Check if demo mode
    const isDemo = token.startsWith("demo_")
    console.log("[Dashboard] Mode:", isDemo ? "🎭 DEMO" : "🌐 REAL API")

    if (!selectedVehicleId) {
      if (isDemo) {
        console.log("[Dashboard] Using mock vehicles for demo mode")
        setVehicles(mockVehicles)
        if (mockVehicles.length > 0) {
          selectVehicle(mockVehicles[0].id)
        }
      } else {
        console.log("[Dashboard] 🔧 TODO: Fetch real vehicles from API")
        // For now, use BENYON_001 as the real vehicle
        const realVehicles = [
          { id: "BENYON_001", model: "RaspberryCar", status: "online" as const }
        ]
        setVehicles(realVehicles)
        selectVehicle("BENYON_001")
      }
    }
  }, [router, selectedVehicleId, setVehicles, selectVehicle, setAuthenticated])

  return (
    <div className="flex h-screen overflow-hidden">
      <Sidebar />
      <div className="flex-1 flex flex-col overflow-hidden">
        <TopNavigation />
        <div className="flex-1 overflow-y-auto">
          <VehicleBanner />
          <div className="p-6 space-y-6">
            <TelemetryCharts />
            <div className="grid grid-cols-3 gap-6">
              <SystemHealthPanel />
              <EnvironmentPanel />
              <SafetyAlerts />
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
