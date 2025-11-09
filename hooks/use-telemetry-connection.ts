"use client"

import { useEffect } from "react"
import { useVehicleStore } from "@/lib/store"
import { generateMockTelemetry } from "@/lib/mock-data"
import { getStoredToken, fetchTelemetry } from "@/lib/api"

export function useTelemetryConnection() {
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const setTelemetry = useVehicleStore((state) => state.setTelemetry)
  const isAuthenticated = useVehicleStore((state) => state.isAuthenticated)

  useEffect(() => {
    console.log("[Telemetry] Hook fired - isAuthenticated:", isAuthenticated, "selectedVehicleId:", selectedVehicleId)
    
    if (!isAuthenticated || !selectedVehicleId) {
      console.warn("[Telemetry] ⚠️ Not running: isAuthenticated =", isAuthenticated, ", selectedVehicleId =", selectedVehicleId)
      return
    }

    const token = getStoredToken()
    if (!token) {
      console.warn("[Telemetry] ⚠️ No token found in localStorage")
      return
    }

    console.log("[Telemetry] ==========================================")
    console.log("[Telemetry] Token:", token.substring(0, 30) + "...")
    console.log("[Telemetry] Selected Vehicle:", selectedVehicleId)
    console.log("[Telemetry] Is Authenticated:", isAuthenticated)

    // Check if using demo mode
    const isDemo = token.startsWith("demo_")
    
    console.log("[Telemetry] Mode:", isDemo ? "🎭 DEMO/MOCK" : "🌐 REAL API")

    if (isDemo) {
      console.log("[Telemetry] Using MOCK DATA mode")
      // Use mock data for demo mode
      const interval = setInterval(() => {
        const mockData = generateMockTelemetry(selectedVehicleId)
        setTelemetry(mockData)
      }, 1000)

      return () => clearInterval(interval)
    }

    console.log("[Telemetry] ✅ Using REAL DATA mode - polling API")

    // Get update interval from settings (default 2 seconds)
    const updateInterval = parseInt(localStorage.getItem("telemetry_update_interval") || "2000")
    console.log("[Telemetry] Update interval:", updateInterval, "ms")

    // Use API polling for real data
    const pollTelemetry = async () => {
      try {
        console.log("[Telemetry] 🔄 Polling telemetry for vehicle:", selectedVehicleId)
        const response = await fetchTelemetry(selectedVehicleId, token)
        console.log("[Telemetry] ✅ Raw response from API:", response)
        
        if (response && response.telemetry && response.telemetry !== null) {
          // Map backend format to frontend format
          const backendData = response.telemetry
          const mappedData = {
            vehicle_id: response.vehicle_id || selectedVehicleId,
            timestamp: backendData.timestamp,
            telemetry: {
              speed_kph: backendData.speed || 0,
              rpm: backendData.rpm || 0,
              throttle_pct: backendData.throttle_pct || 0,
              brake_pct: backendData.brake_pct || 0,
              gear: backendData.gear || 1,
              battery_voltage: backendData.battery_voltage || 12.6,
              engine_temp_c: backendData.engine_temp || 70,
              fuel_level_pct: backendData.fuel_level || 100,
              gps: { 
                lat: backendData.gps_lat || 0, 
                lon: backendData.gps_lon || 0 
              },
              ambient_temp_c: backendData.ambient_temp || 25,
              humidity_pct: backendData.humidity || 50,
              wheel_speed: {
                front_left: backendData.wheel_speed_fl || 0,
                front_right: backendData.wheel_speed_fr || 0,
                rear_left: backendData.wheel_speed_rl || 0,
                rear_right: backendData.wheel_speed_rr || 0,
              },
            },
            system: {
              cpu_usage_pct: backendData.cpu_usage || 0,
              ram_usage_pct: backendData.memory_usage || 0,
              network_latency_ms: backendData.latency_ms || 0,
              last_sync: backendData.timestamp,
            },
            status: {
              ABS_active: backendData.brake_pct > 50,
              traction_control: backendData.traction_control || false,
              DTC: backendData.diagnostics || [],
            },
          }
          
          console.log("[Telemetry] 📊 Mapped data being set to store")
          setTelemetry(mappedData)
        } else {
          console.error("[Telemetry] ❌ Backend returned null data!")
          console.error("[Telemetry] 🔧 Please run the Python FastAPI backend on your Raspberry Pi:")
          console.error("[Telemetry] 📝 cd ~/kintsugi-backend && python -m uvicorn main:app --host 0.0.0.0 --port 8000")
        }
      } catch (error) {
        console.error("[Telemetry] ❌ Failed to fetch:", error)
        console.error("[Telemetry] 💡 Make sure your Raspberry Pi backend is running at:", "http://172.20.10.3:8000")
      }
    }

    // Poll at configured interval
    const interval = setInterval(pollTelemetry, updateInterval)
    
    // Fetch immediately
    pollTelemetry()

    return () => clearInterval(interval)
  }, [selectedVehicleId, isAuthenticated, setTelemetry])
}
