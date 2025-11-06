import type { Vehicle, TelemetryData } from "./store"

export const mockVehicles: Vehicle[] = [
  { id: "BENYON_001", model: "RaspberryCar", status: "online" },
  { id: "BENYON_002", model: "RaspberryCar Pro", status: "online" },
  { id: "BENYON_003", model: "RaspberryCar", status: "offline" },
]

export function generateMockTelemetry(vehicleId: string): TelemetryData {
  const now = new Date()
  return {
    vehicle_id: vehicleId,
    timestamp: now.toISOString(),
    telemetry: {
      speed_kph: Math.random() * 120,
      rpm: Math.random() * 6000,
      throttle_pct: Math.random() * 100,
      brake_pct: Math.random() * 100,
      gear: Math.floor(Math.random() * 5) + 1,
      battery_voltage: 12.5 + Math.random() * 0.5,
      engine_temp_c: 80 + Math.random() * 20,
      fuel_level_pct: 30 + Math.random() * 70,
      gps: { lat: 10.0053 + Math.random() * 0.01, lon: 76.3601 + Math.random() * 0.01 },
      ambient_temp_c: 25 + Math.random() * 15,
      humidity_pct: 50 + Math.random() * 50,
      wheel_speed: {
        front_left: Math.random() * 100,
        front_right: Math.random() * 100,
        rear_left: Math.random() * 100,
        rear_right: Math.random() * 100,
      },
    },
    system: {
      cpu_usage_pct: Math.random() * 50,
      ram_usage_pct: 30 + Math.random() * 40,
      network_latency_ms: 30 + Math.random() * 40,
      last_sync: new Date(now.getTime() - Math.random() * 60000).toISOString(),
    },
    status: {
      ABS_active: false,
      traction_control: true,
      DTC: [],
    },
  }
}

export function generateHistoricalData(vehicleId: string, daysBack = 7) {
  const data = []
  const now = new Date()

  for (let day = daysBack; day >= 0; day--) {
    const dayDate = new Date(now)
    dayDate.setDate(dayDate.getDate() - day)
    dayDate.setHours(0, 0, 0, 0)

    for (let hour = 0; hour < 24; hour++) {
      const pointDate = new Date(dayDate)
      pointDate.setHours(hour)
      pointDate.setMinutes(Math.random() * 60)

      data.push({
        timestamp: pointDate.toISOString(),
        vehicleId,
        telemetry: {
          speed_kph: Math.random() * 120,
          rpm: Math.random() * 6000,
          throttle_pct: Math.random() * 100,
          brake_pct: Math.random() * 100,
          gear: Math.floor(Math.random() * 5) + 1,
          battery_voltage: 12.5 + Math.random() * 0.5,
          engine_temp_c: 80 + Math.random() * 20,
          fuel_level_pct: 30 + Math.random() * 70,
        },
      })
    }
  }

  return data
}
