import { create } from "zustand"

export interface Vehicle {
  id: string
  model: string
  status: "online" | "offline"
}

export interface TelemetryData {
  vehicle_id: string
  timestamp: string
  telemetry: {
    speed_kph: number
    rpm: number
    throttle_pct: number
    brake_pct: number
    gear: number
    battery_voltage: number
    engine_temp_c: number
    fuel_level_pct: number
    gps: { lat: number; lon: number }
    ambient_temp_c: number
    humidity_pct: number
    wheel_speed: {
      front_left: number
      front_right: number
      rear_left: number
      rear_right: number
    }
  }
  system: {
    cpu_usage_pct: number
    ram_usage_pct: number
    network_latency_ms: number
    last_sync: string
  }
  status: {
    ABS_active: boolean
    traction_control: boolean
    DTC: string[]
  }
}

interface VehicleStore {
  vehicles: Vehicle[]
  selectedVehicleId: string | null
  currentTelemetry: TelemetryData | null
  isAuthenticated: boolean

  setVehicles: (vehicles: Vehicle[]) => void
  selectVehicle: (id: string) => void
  setTelemetry: (data: TelemetryData) => void
  setAuthenticated: (auth: boolean) => void
}

export const useVehicleStore = create<VehicleStore>((set) => ({
  vehicles: [],
  selectedVehicleId: null,
  currentTelemetry: null,
  isAuthenticated: false,

  setVehicles: (vehicles) => set({ vehicles }),
  selectVehicle: (id) => set({ selectedVehicleId: id }),
  setTelemetry: (data) => set({ currentTelemetry: data }),
  setAuthenticated: (auth) => set({ isAuthenticated: auth }),
}))
