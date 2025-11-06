"use client"

import { useVehicleStore } from "@/lib/store"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Cloud, Droplets, MapPin, Gauge } from "lucide-react"

interface WheelSpeeds {
  label: string
  value: number
  unit: string
}

export function EnvironmentPanel() {
  const currentTelemetry = useVehicleStore((state) => state.currentTelemetry)

  if (!currentTelemetry) return null

  const ambientTemp = currentTelemetry.telemetry.ambient_temp_c
  const humidity = currentTelemetry.telemetry.humidity_pct
  const gps = currentTelemetry.telemetry.gps

  const wheelSpeeds: WheelSpeeds[] = [
    {
      label: "Front Left",
      value: currentTelemetry.telemetry.wheel_speed.front_left,
      unit: "km/h",
    },
    {
      label: "Front Right",
      value: currentTelemetry.telemetry.wheel_speed.front_right,
      unit: "km/h",
    },
    {
      label: "Rear Left",
      value: currentTelemetry.telemetry.wheel_speed.rear_left,
      unit: "km/h",
    },
    {
      label: "Rear Right",
      value: currentTelemetry.telemetry.wheel_speed.rear_right,
      unit: "km/h",
    },
  ]

  const tempStatus = ambientTemp > 35 ? "text-red-400" : ambientTemp > 25 ? "text-orange-400" : "text-blue-400"

  return (
    <Card className="border border-border col-span-1">
      <CardHeader className="pb-3">
        <CardTitle className="text-base">Environment & Location</CardTitle>
      </CardHeader>
      <CardContent className="space-y-4">
        {/* Environmental metrics */}
        <div className="space-y-2">
          <div className="bg-secondary/50 p-3 rounded-lg border border-border">
            <div className="flex items-center gap-2 mb-1">
              <Cloud className="w-4 h-4 text-orange-400" />
              <span className="text-xs text-muted-foreground uppercase tracking-wide">Ambient Temperature</span>
            </div>
            <div className={`text-lg font-bold ${tempStatus}`}>{ambientTemp.toFixed(1)}°C</div>
          </div>

          <div className="bg-secondary/50 p-3 rounded-lg border border-border">
            <div className="flex items-center gap-2 mb-1">
              <Droplets className="w-4 h-4 text-blue-400" />
              <span className="text-xs text-muted-foreground uppercase tracking-wide">Humidity</span>
            </div>
            <div className="text-lg font-bold text-foreground">{humidity.toFixed(1)}%</div>
          </div>

          <div className="bg-secondary/50 p-3 rounded-lg border border-border">
            <div className="flex items-center gap-2 mb-1">
              <MapPin className="w-4 h-4 text-green-400" />
              <span className="text-xs text-muted-foreground uppercase tracking-wide">GPS Position</span>
            </div>
            <div className="text-xs font-mono text-accent mt-1">
              {gps.lat.toFixed(4)}, {gps.lon.toFixed(4)}
            </div>
          </div>
        </div>

        {/* Wheel speeds grid */}
        <div className="border-t border-border pt-3">
          <div className="text-xs uppercase font-semibold text-muted-foreground mb-2 flex items-center gap-2">
            <Gauge className="w-3 h-3" />
            Wheel Speeds
          </div>
          <div className="grid grid-cols-2 gap-2">
            {wheelSpeeds.map((wheel, idx) => (
              <div key={idx} className="bg-secondary/30 p-2 rounded-md border border-border/50">
                <div className="text-xs text-muted-foreground">{wheel.label}</div>
                <div className="text-sm font-semibold text-accent">{wheel.value.toFixed(1)}</div>
              </div>
            ))}
          </div>
        </div>
      </CardContent>
    </Card>
  )
}
