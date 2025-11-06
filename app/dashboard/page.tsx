"use client"

import { useEffect } from "react"
import { useRouter } from "next/navigation"
import { getStoredToken, fetchVehicles } from "@/lib/api"
import { useVehicleStore } from "@/lib/store"
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

  useTelemetryConnection()

  useEffect(() => {
    const token = getStoredToken()
    if (!token) {
      router.push("/login")
      return
    }

    // Fetch vehicles from API
    const loadVehicles = async () => {
      try {
        console.log("Fetching vehicles from API...")
        const data = await fetchVehicles(token)
        console.log("Vehicles received:", data)
        setVehicles(data.vehicles.map((v: any) => ({ id: v.id, model: v.name || v.model, status: v.status })))
        if (data.vehicles.length > 0 && !selectedVehicleId) {
          selectVehicle(data.vehicles[0].id)
        }
      } catch (error) {
        console.error("Failed to fetch vehicles:", error)
      }
    }

    loadVehicles()
  }, [router, selectedVehicleId, setVehicles, selectVehicle])

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
