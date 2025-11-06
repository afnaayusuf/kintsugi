"use client"

import { useEffect } from "react"
import { useVehicleStore } from "@/lib/store"
import { telemetryWS } from "@/lib/websocket"
import { generateMockTelemetry } from "@/lib/mock-data"
import { getStoredToken, fetchTelemetry } from "@/lib/api"

export function useTelemetryConnection() {
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const setTelemetry = useVehicleStore((state) => state.setTelemetry)
  const isAuthenticated = useVehicleStore((state) => state.isAuthenticated)

  useEffect(() => {
    if (!isAuthenticated || !selectedVehicleId) return

    const token = getStoredToken()
    if (!token) return

    console.log("[Telemetry] Token:", token.substring(0, 20) + "...")
    console.log("[Telemetry] Selected Vehicle:", selectedVehicleId)

    // Check if using demo mode
    const isDemo = token.startsWith("demo_")

    if (isDemo) {
      console.log("[Telemetry] Using MOCK DATA mode")
      // Use mock data for demo mode
      const interval = setInterval(() => {
        const mockData = generateMockTelemetry(selectedVehicleId)
        setTelemetry(mockData)
      }, 1000)

      return () => clearInterval(interval)
    }

    console.log("[Telemetry] Using REAL DATA mode - polling API")

    // Get update interval from settings (default 2 seconds)
    const updateInterval = parseInt(localStorage.getItem("telemetry_update_interval") || "2000")
    console.log("[Telemetry] Update interval:", updateInterval, "ms")

    // Use API polling for real data (since WebSocket may not be working)
    const pollTelemetry = async () => {
      try {
        const data = await fetchTelemetry(selectedVehicleId, token)
        console.log("[Telemetry] Received from API:", data)
        if (data && data.telemetry) {
          setTelemetry(data.telemetry)
        }
      } catch (error) {
        console.error("[Telemetry] Failed to fetch:", error)
      }
    }

    // Poll at configured interval
    const interval = setInterval(pollTelemetry, updateInterval)
    
    // Fetch immediately
    pollTelemetry()

    return () => clearInterval(interval)
  }, [selectedVehicleId, isAuthenticated, setTelemetry])
}
