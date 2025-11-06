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

  useTelemetryConnection()

  useEffect(() => {
    const token = getStoredToken()
    if (!token) {
      router.push("/login")
      return
    }

    if (!selectedVehicleId) {
      setVehicles(mockVehicles)
      if (mockVehicles.length > 0) {
        selectVehicle(mockVehicles[0].id)
      }
    }
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
