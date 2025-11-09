"use client"

import { useEffect, useState } from "react"
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

  useEffect(() => {
    if (!selectedVehicleId) return

    const updateChart = () => {
      if (isPaused) return

      // Use real telemetry data from the store (fetched by use-telemetry-connection hook)
      const telemetry = currentTelemetry
      if (!telemetry) return
      
      const now = new Date()
      const time = now.getTime()

      const newPoint: DataPoint = {
        timestamp: now.toLocaleTimeString(),
        time,
        speed: Math.round(telemetry.telemetry.speed_kph * 10) / 10,
        rpm: Math.round(telemetry.telemetry.rpm / 100) / 10,
        temp: Math.round(telemetry.telemetry.engine_temp_c * 10) / 10,
        battery: Math.round(telemetry.telemetry.battery_voltage * 100) / 100,
        fuel: Math.round(telemetry.telemetry.fuel_level_pct),
        throttle: Math.round(telemetry.telemetry.throttle_pct),
        brake: Math.round(telemetry.telemetry.brake_pct),
      }

      setChartData((prev) => {
        const updated = [...prev, newPoint]
        const kept = updated.slice(-60) // Keep last 60 data points (1 minute)

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
    }

    updateChart()
    const interval = setInterval(updateChart, 1000)
    return () => clearInterval(interval)
  }, [selectedVehicleId, isPaused])

  return (
    <div className="space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-xl font-bold text-foreground">Real-Time Telemetry</h2>
          <p className="text-sm text-muted-foreground">Live vehicle performance monitoring</p>
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
            <CardTitle className="text-base">Throttle & Brake</CardTitle>
          </CardHeader>
          <CardContent>
            <ResponsiveContainer width="100%" height={240}>
              <BarChart data={chartData} margin={{ top: 5, right: 10, left: -20, bottom: 5 }}>
                <CartesianGrid strokeDasharray="3 3" stroke="oklch(0.25 0 0)" />
                <XAxis dataKey="timestamp" stroke="oklch(0.70 0 0)" fontSize={12} />
                <YAxis stroke="oklch(0.70 0 0)" fontSize={12} domain={[0, 100]} />
                <Tooltip content={<CustomTooltip />} />
                <Legend wrapperStyle={{ paddingTop: "10px" }} />
                <Bar dataKey="throttle" fill="oklch(0.65 0.30 240)" name="Throttle (%)" />
                <Bar dataKey="brake" fill="oklch(0.60 0.24 30)" name="Brake (%)" />
              </BarChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>
      </div>
    </div>
  )
}
