"use client"

import { useEffect, useState, useRef } from "react"
import { useVehicleStore } from "@/lib/store"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Pause, Play, TrendingDown, TrendingUp } from "lucide-react"
import {
  LineChart,
  Line,
  AreaChart,
  Area,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
  BarChart,
  Bar,
  ComposedChart,
} from "recharts"

interface DataPoint {
  timestamp: string
  time: number
  speed: number
  rpm: number
  temp: number
  battery: number
  fuel: number
  throttle: number
  brake: number
}

function TrendIndicator({ current, previous }: { current: number; previous?: number }) {
  if (!previous) return null
  const diff = current - previous
  const isPositive = diff > 0
  return (
    <div className={`flex items-center gap-1 text-xs ${isPositive ? "text-green-400" : "text-red-400"}`}>
      {isPositive ? <TrendingUp className="w-3 h-3" /> : <TrendingDown className="w-3 h-3" />}
      {Math.abs(diff).toFixed(1)}
    </div>
  )
}

function CustomTooltip(props: any) {
  const { active, payload, label } = props
  if (active && payload && payload.length) {
    return (
      <div className="bg-card border border-border rounded-lg p-3 shadow-lg">
        <p className="text-xs text-muted-foreground mb-2">{label}</p>
        {payload.map((entry: any, index: number) => (
          <p key={index} style={{ color: entry.color }} className="text-sm font-medium">
            {entry.name}: {entry.value.toFixed(1)}
          </p>
        ))}
      </div>
    )
  }
  return null
}

export function TelemetryCharts() {
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const currentTelemetry = useVehicleStore((state) => state.currentTelemetry)
  const [chartData, setChartData] = useState<DataPoint[]>([])
  const [isPaused, setIsPaused] = useState(false)
  const [stats, setStats] = useState({
    avgSpeed: 0,
    maxSpeed: 0,
    avgTemp: 0,
  })
  const lastTimestampRef = useRef<string | null>(null)

  // Update chart when telemetry changes
  useEffect(() => {
    if (!currentTelemetry || isPaused) return

    // Only add new point if timestamp changed (avoid duplicates)
    if (currentTelemetry.timestamp === lastTimestampRef.current) return
    lastTimestampRef.current = currentTelemetry.timestamp

    console.log("[TelemetryCharts] Adding new data point:", {
      speed: currentTelemetry.telemetry.speed_kph,
      rpm: currentTelemetry.telemetry.rpm,
      temp: currentTelemetry.telemetry.engine_temp_c,
      battery: currentTelemetry.telemetry.battery_voltage,
      throttle: currentTelemetry.telemetry.throttle_pct,
      brake: currentTelemetry.telemetry.brake_pct,
    })

    const now = new Date()
    const newPoint: DataPoint = {
      timestamp: now.toLocaleTimeString(),
      time: now.getTime(),
      speed: Math.round(currentTelemetry.telemetry.speed_kph * 10) / 10,
      rpm: Math.round(currentTelemetry.telemetry.rpm / 100) / 10,
      temp: Math.round(currentTelemetry.telemetry.engine_temp_c * 10) / 10,
      battery: Math.round(currentTelemetry.telemetry.battery_voltage * 100) / 100,
      fuel: Math.round(currentTelemetry.telemetry.fuel_level_pct),
      throttle: Math.round(currentTelemetry.telemetry.throttle_pct),
      brake: Math.round(currentTelemetry.telemetry.brake_pct),
    }

    console.log("[TelemetryCharts] New point created:", {
      throttle: newPoint.throttle,
      brake: newPoint.brake,
      timestamp: newPoint.timestamp
    })

    setChartData((prev) => {
      const updated = [...prev, newPoint]
      const kept = updated.slice(-60) // Keep last 60 data points (1 minute)

      console.log("[TelemetryCharts] Chart data updated:", {
        totalPoints: kept.length,
        latestPoint: kept[kept.length - 1],
        throttleValues: kept.slice(-5).map(p => p.throttle),
        brakeValues: kept.slice(-5).map(p => p.brake)
      })

      if (kept.length > 0) {
        const speeds = kept.map((p) => p.speed)
        const temps = kept.map((p) => p.temp)
        setStats({
          avgSpeed: speeds.reduce((a, b) => a + b, 0) / speeds.length,
          maxSpeed: Math.max(...speeds),
          avgTemp: temps.reduce((a, b) => a + b, 0) / temps.length,
        })
      }

      return kept
    })
  }, [currentTelemetry, isPaused])

  return (
    <div className="space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-xl font-bold text-foreground">Real-Time Telemetry</h2>
          <p className="text-sm text-muted-foreground">
            Live vehicle performance monitoring ({chartData.length} data points)
          </p>
        </div>
        <Button
          variant="outline"
          onClick={() => setIsPaused(!isPaused)}
          className="gap-2 bg-secondary/50 border-border hover:bg-secondary"
        >
          {isPaused ? <Play className="w-4 h-4" /> : <Pause className="w-4 h-4" />}
          {isPaused ? "Resume" : "Pause"}
        </Button>
      </div>

      {/* Show loading state if no data */}
      {chartData.length === 0 && (
        <Card className="border border-border">
          <CardContent className="pt-6 pb-6 text-center">
            <div className="text-muted-foreground">
              Waiting for telemetry data...
              {!currentTelemetry && <div className="text-xs mt-2">No telemetry received yet</div>}
            </div>
          </CardContent>
        </Card>
      )}

      {/* Stats cards */}
      <div className="grid grid-cols-3 gap-3">
        <Card className="border border-border bg-secondary/50">
          <CardContent className="pt-4">
            <div className="text-xs text-muted-foreground uppercase tracking-wide">Avg Speed</div>
            <div className="flex items-end justify-between mt-2">
              <div className="text-2xl font-bold text-accent">{stats.avgSpeed.toFixed(1)}</div>
              <span className="text-xs text-muted-foreground">km/h</span>
            </div>
          </CardContent>
        </Card>
        <Card className="border border-border bg-secondary/50">
          <CardContent className="pt-4">
            <div className="text-xs text-muted-foreground uppercase tracking-wide">Max Speed</div>
            <div className="flex items-end justify-between mt-2">
              <div className="text-2xl font-bold text-accent">{stats.maxSpeed.toFixed(1)}</div>
              <span className="text-xs text-muted-foreground">km/h</span>
            </div>
          </CardContent>
        </Card>
        <Card className="border border-border bg-secondary/50">
          <CardContent className="pt-4">
            <div className="text-xs text-muted-foreground uppercase tracking-wide">Avg Temp</div>
            <div className="flex items-end justify-between mt-2">
              <div className="text-2xl font-bold text-accent">{stats.avgTemp.toFixed(1)}</div>
              <span className="text-xs text-muted-foreground">°C</span>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Charts grid */}
      <div className="grid grid-cols-2 gap-4">
        {/* Speed & RPM */}
        <Card className="border border-border">
          <CardHeader className="pb-2">
            <CardTitle className="text-base flex items-center justify-between">
              Speed & RPM
              {chartData.length > 1 && (
                <TrendIndicator
                  current={chartData[chartData.length - 1].speed}
                  previous={chartData[chartData.length - 2].speed}
                />
              )}
            </CardTitle>
          </CardHeader>
          <CardContent>
            <ResponsiveContainer width="100%" height={280}>
              <LineChart data={chartData} margin={{ top: 5, right: 10, left: -20, bottom: 5 }}>
                <CartesianGrid strokeDasharray="3 3" stroke="oklch(0.25 0 0)" />
                <XAxis dataKey="timestamp" stroke="oklch(0.70 0 0)" fontSize={12} />
                <YAxis yAxisId="left" stroke="oklch(0.70 0 0)" fontSize={12} />
                <YAxis yAxisId="right" orientation="right" stroke="oklch(0.70 0 0)" fontSize={12} />
                <Tooltip content={<CustomTooltip />} />
                <Legend wrapperStyle={{ paddingTop: "10px" }} />
                <Line
                  yAxisId="left"
                  type="monotone"
                  dataKey="speed"
                  stroke="oklch(0.65 0.30 240)"
                  strokeWidth={2}
                  dot={false}
                  name="Speed (km/h)"
                />
                <Line
                  yAxisId="right"
                  type="monotone"
                  dataKey="rpm"
                  stroke="oklch(0.70 0.25 200)"
                  strokeWidth={2}
                  dot={false}
                  name="RPM (x100)"
                />
              </LineChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>

        {/* Temperature & Battery */}
        <Card className="border border-border">
          <CardHeader className="pb-2">
            <CardTitle className="text-base flex items-center justify-between">
              Temperature & Battery
              {chartData.length > 1 && (
                <TrendIndicator
                  current={chartData[chartData.length - 1].temp}
                  previous={chartData[chartData.length - 2].temp}
                />
              )}
            </CardTitle>
          </CardHeader>
          <CardContent>
            <ResponsiveContainer width="100%" height={280}>
              <LineChart data={chartData} margin={{ top: 5, right: 10, left: -20, bottom: 5 }}>
                <CartesianGrid strokeDasharray="3 3" stroke="oklch(0.25 0 0)" />
                <XAxis dataKey="timestamp" stroke="oklch(0.70 0 0)" fontSize={12} />
                <YAxis yAxisId="left" stroke="oklch(0.70 0 0)" fontSize={12} />
                <YAxis yAxisId="right" orientation="right" stroke="oklch(0.70 0 0)" fontSize={12} />
                <Tooltip content={<CustomTooltip />} />
                <Legend wrapperStyle={{ paddingTop: "10px" }} />
                <Line
                  yAxisId="left"
                  type="monotone"
                  dataKey="temp"
                  stroke="oklch(0.60 0.24 30)"
                  strokeWidth={2}
                  dot={false}
                  name="Temp (°C)"
                />
                <Line
                  yAxisId="right"
                  type="monotone"
                  dataKey="battery"
                  stroke="oklch(0.52 0.25 120)"
                  strokeWidth={2}
                  dot={false}
                  name="Battery (V)"
                />
              </LineChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>
      </div>

      {/* Fuel & Pedal inputs */}
      <div className="grid grid-cols-2 gap-4">
        <Card className="border border-border">
          <CardHeader className="pb-2">
            <CardTitle className="text-base">Fuel Level</CardTitle>
          </CardHeader>
          <CardContent>
            <ResponsiveContainer width="100%" height={240}>
              <AreaChart data={chartData} margin={{ top: 5, right: 10, left: -20, bottom: 5 }}>
                <CartesianGrid strokeDasharray="3 3" stroke="oklch(0.25 0 0)" />
                <XAxis dataKey="timestamp" stroke="oklch(0.70 0 0)" fontSize={12} />
                <YAxis stroke="oklch(0.70 0 0)" fontSize={12} domain={[0, 100]} />
                <Tooltip content={<CustomTooltip />} />
                <Area type="monotone" dataKey="fuel" fill="oklch(0.65 0.30 240)" stroke="oklch(0.65 0.30 240)" />
              </AreaChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>

        <Card className="border border-border">
          <CardHeader className="pb-2">
            <CardTitle className="text-base flex items-center justify-between">
              Throttle & Brake
              {chartData.length > 0 && (
                <div className="text-xs text-muted-foreground font-normal">
                  Last: T={chartData[chartData.length - 1]?.throttle}% B={chartData[chartData.length - 1]?.brake}%
                  | Points: {chartData.length}
                </div>
              )}
            </CardTitle>
          </CardHeader>
          <CardContent>
            {chartData.length === 0 ? (
              <div className="h-[240px] flex items-center justify-center text-muted-foreground text-sm">
                Waiting for data...
              </div>
            ) : (
              <div>
                {/* Debug info */}
                <div className="text-xs text-muted-foreground mb-2 p-2 bg-secondary/30 rounded">
                  Chart has {chartData.length} points. Sample data: 
                  {chartData.slice(-3).map((d, i) => (
                    <div key={i}>
                      {d.timestamp}: throttle={d.throttle}, brake={d.brake}
                    </div>
                  ))}
                </div>
                <ResponsiveContainer width="100%" height={200}>
                  <LineChart data={chartData} margin={{ top: 5, right: 10, left: -20, bottom: 5 }}>
                    <CartesianGrid strokeDasharray="3 3" stroke="oklch(0.25 0 0)" />
                    <XAxis dataKey="timestamp" stroke="oklch(0.70 0 0)" fontSize={12} />
                    <YAxis stroke="oklch(0.70 0 0)" fontSize={12} domain={[0, 100]} />
                    <Tooltip content={<CustomTooltip />} />
                    <Legend wrapperStyle={{ paddingTop: "10px" }} />
                    <Line 
                      type="monotone" 
                      dataKey="throttle" 
                      stroke="oklch(0.65 0.30 240)" 
                      strokeWidth={2} 
                      dot={false} 
                      name="Throttle (%)"
                      fill="oklch(0.65 0.30 240)"
                      fillOpacity={0.3}
                    />
                    <Line 
                      type="monotone" 
                      dataKey="brake" 
                      stroke="oklch(0.60 0.24 30)" 
                      strokeWidth={2} 
                      dot={false} 
                      name="Brake (%)"
                      fill="oklch(0.60 0.24 30)"
                      fillOpacity={0.3}
                    />
                  </LineChart>
                </ResponsiveContainer>
              </div>
            )}
          </CardContent>
        </Card>
      </div>
    </div>
  )
}
