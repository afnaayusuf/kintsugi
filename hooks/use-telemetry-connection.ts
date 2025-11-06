"use client"

import { useEffect } from "react"
import { useVehicleStore } from "@/lib/store"
import { telemetryWS } from "@/lib/websocket"
import { generateMockTelemetry } from "@/lib/mock-data"
import { getStoredToken } from "@/lib/api"

export function useTelemetryConnection() {
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const setTelemetry = useVehicleStore((state) => state.setTelemetry)
  const isAuthenticated = useVehicleStore((state) => state.isAuthenticated)

  useEffect(() => {
    if (!isAuthenticated || !selectedVehicleId) return

    const token = getStoredToken()
    if (!token) return

    // Check if using demo mode
    const isDemo = token.startsWith("demo_")

    if (isDemo) {
      // Use mock data for demo mode
      const interval = setInterval(() => {
        const mockData = generateMockTelemetry(selectedVehicleId)
        setTelemetry(mockData)
      }, 1000)

      return () => clearInterval(interval)
    }

    // Setup real WebSocket connection for production
    const wsBaseUrl = process.env.NEXT_PUBLIC_WS_URL || "ws://localhost:8000"
    const wsUrl = `${wsBaseUrl}/ws/telemetry/${selectedVehicleId}`

    const setupConnection = async () => {
      try {
        console.log("Connecting WebSocket to:", wsUrl)
        await telemetryWS.connect({
          url: wsUrl,
          token,
          vehicleId: selectedVehicleId,
        })

        telemetryWS.on("telemetry", (data) => {
          console.log("Telemetry received:", data)
          setTelemetry(data)
        })
      } catch (error) {
        console.error("[v0] Failed to connect WebSocket:", error)
      }
    }

    setupConnection()

    return () => {
      telemetryWS.disconnect()
    }
  }, [selectedVehicleId, isAuthenticated, setTelemetry])
}
