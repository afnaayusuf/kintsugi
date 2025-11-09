"use client"

import type React from "react"

import { useState } from "react"
import { useVehicleStore } from "@/lib/store"
import { Activity, Gauge, Zap, Wifi, AlertCircle, Thermometer } from "lucide-react"

interface MetricDisplayProps {
  label: string
  value: string
  unit?: string
  icon: React.ReactNode
  status?: "normal" | "warning" | "alert"
  trend?: "up" | "down" | "stable"
}

function MetricCard({ label, value, unit, icon, status = "normal", trend }: MetricDisplayProps) {
  const statusColors = {
    normal: "border-border bg-secondary/50",
    warning: "border-yellow-500/30 bg-yellow-500/10",
    alert: "border-red-500/30 bg-red-500/10",
  }

  const trendIcons = {
    up: "↗",
    down: "↙",
    stable: "→",
  }

  return (
    <div className={`flex items-center gap-3 px-4 py-3 rounded-lg border transition ${statusColors[status]}`}>
      <div
        className={`p-2 rounded-md ${status === "alert" ? "bg-red-500/20" : status === "warning" ? "bg-yellow-500/20" : "bg-accent/20"}`}
      >
        {icon}
      </div>
      <div className="min-w-0 flex-1">
        <div className="text-xs text-muted-foreground uppercase tracking-wide">{label}</div>
        <div className="flex items-baseline gap-1">
          <div className="text-lg font-bold text-foreground">{value}</div>
          {unit && <span className="text-xs text-muted-foreground">{unit}</span>}
          {trend && <span className="text-xs text-muted-foreground">{trendIcons[trend]}</span>}
        </div>
      </div>
    </div>
  )
}

export function VehicleBanner() {
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const currentTelemetry = useVehicleStore((state) => state.currentTelemetry)
  const [isUpdating, setIsUpdating] = useState(false)

  // NOTE: Telemetry data is now fetched by use-telemetry-connection.ts hook
  // This component just displays the data from the store

  if (!currentTelemetry) {
    return (
      <div className="bg-secondary/50 border-b border-border p-6 text-center">
        <div className="text-muted-foreground">Loading vehicle telemetry...</div>
      </div>
    )
  }

  const speed = currentTelemetry.telemetry.speed_kph
  const rpm = currentTelemetry.telemetry.rpm
  const engineTemp = currentTelemetry.telemetry.engine_temp_c
  const battery = currentTelemetry.telemetry.battery_voltage
  const fuel = currentTelemetry.telemetry.fuel_level_pct
  const networkLatency = currentTelemetry.system.network_latency_ms

  // Determine status indicators
  const speedStatus = speed > 100 ? "warning" : "normal"
  const engineStatus = engineTemp > 95 ? "alert" : engineTemp > 85 ? "warning" : "normal"
  const batteryStatus = battery < 12.0 ? "alert" : "normal"

  return (
    <div className="border-b border-border bg-card/50 backdrop-blur-sm">
      {/* Top row: Vehicle and connection info */}
      <div className="px-6 py-4 border-b border-border/50 flex items-center justify-between">
        <div className="flex items-center gap-3">
          <div className="flex items-center justify-center w-10 h-10 rounded-lg bg-primary/20">
            <Activity className="w-5 h-5 text-accent" />
          </div>
          <div>
            <h2 className="text-sm font-semibold text-foreground">{currentTelemetry.vehicle_id}</h2>
            <p className="text-xs text-muted-foreground">Real-time Vehicle Telemetry</p>
          </div>
        </div>

        <div className="flex items-center gap-4">
          <div className="text-right">
            <div className="text-xs text-muted-foreground">Connection Status</div>
            <div className="flex items-center gap-2 mt-1">
              <div
                className={`w-2 h-2 rounded-full ${isUpdating ? "bg-yellow-400 animate-pulse" : "bg-green-400"}`}
              ></div>
              <span className="text-sm font-medium">{isUpdating ? "Updating..." : "Connected"}</span>
            </div>
          </div>

          <div className="text-right">
            <div className="text-xs text-muted-foreground">Latency</div>
            <div className="text-sm font-medium text-foreground">{networkLatency.toFixed(0)}ms</div>
          </div>
        </div>
      </div>

      {/* Metrics grid */}
      <div className="px-6 py-4 grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-3">
        <MetricCard
          label="Speed"
          value={speed.toFixed(1)}
          unit="km/h"
          icon={<Gauge className="w-4 h-4 text-accent" />}
          status={speedStatus}
          trend={speed > 50 ? "up" : "stable"}
        />

        <MetricCard
          label="RPM"
          value={(rpm / 1000).toFixed(1)}
          unit="k"
          icon={<Activity className="w-4 h-4 text-accent" />}
          status={rpm > 4500 ? "warning" : "normal"}
        />

        <MetricCard
          label="Engine Temp"
          value={engineTemp.toFixed(0)}
          unit="°C"
          icon={<Thermometer className="w-4 h-4 text-accent" />}
          status={engineStatus}
        />

        <MetricCard
          label="Fuel Level"
          value={fuel.toFixed(0)}
          unit="%"
          icon={<Zap className="w-4 h-4 text-accent" />}
          status={fuel < 20 ? "alert" : fuel < 50 ? "warning" : "normal"}
        />

        <MetricCard
          label="Battery"
          value={battery.toFixed(2)}
          unit="V"
          icon={<Zap className="w-4 h-4 text-accent" />}
          status={batteryStatus}
        />

        <MetricCard
          label="Traction"
          value={currentTelemetry.status.traction_control ? "Active" : "Inactive"}
          icon={<Wifi className="w-4 h-4 text-accent" />}
          status={currentTelemetry.status.traction_control ? "normal" : "warning"}
        />
      </div>

      {/* Alert section if any issues */}
      {(speedStatus === "alert" || engineStatus === "alert" || batteryStatus === "alert") && (
        <div className="px-6 py-3 bg-red-500/10 border-t border-red-500/20 flex items-center gap-2">
          <AlertCircle className="w-4 h-4 text-red-400 flex-shrink-0" />
          <span className="text-sm text-red-400">System Alert: Check engine status immediately</span>
        </div>
      )}
    </div>
  )
}
