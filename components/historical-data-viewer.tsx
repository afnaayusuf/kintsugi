"use client"

import { useState, useEffect } from "react"
import { useVehicleStore } from "@/lib/store"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { generateMockTelemetry } from "@/lib/mock-data"
import { Calendar, Download, Filter } from "lucide-react"
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
} from "recharts"

interface HistoricalDataPoint {
  timestamp: string
  time: number
  speed: number
  rpm: number
  temp: number
  battery: number
  fuel: number
  distance: number
}

interface DateRange {
  start: Date
  end: Date
}

interface Statistics {
  totalDistance: number
  averageSpeed: number
  maxSpeed: number
  totalDuration: number
  averageTemp: number
  maxTemp: number
  fuelConsumed: number
  avgBattery: number
}

export function HistoricalDataViewer() {
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const [historicalData, setHistoricalData] = useState<HistoricalDataPoint[]>([])
  const [dateRange, setDateRange] = useState<DateRange>({
    start: new Date(Date.now() - 7 * 24 * 60 * 60 * 1000), // 7 days ago
    end: new Date(),
  })
  const [stats, setStats] = useState<Statistics>({
    totalDistance: 0,
    averageSpeed: 0,
    maxSpeed: 0,
    totalDuration: 0,
    averageTemp: 0,
    maxTemp: 0,
    fuelConsumed: 0,
    avgBattery: 0,
  })
  const [filterType, setFilterType] = useState<"all" | "daily" | "weekly">("daily")

  useEffect(() => {
    // Generate historical data simulation
    if (!selectedVehicleId) return

    const data: HistoricalDataPoint[] = []
    const now = new Date()
    const daysBack = Math.ceil((now.getTime() - dateRange.start.getTime()) / (1000 * 60 * 60 * 24))

    for (let day = daysBack; day >= 0; day--) {
      const dayDate = new Date(now)
      dayDate.setDate(dayDate.getDate() - day)
      dayDate.setHours(0, 0, 0, 0)

      // Generate data points throughout the day
      for (let hour = 0; hour < 24; hour += filterType === "all" ? 1 : 6) {
        const pointDate = new Date(dayDate)
        pointDate.setHours(hour)

        const telemetry = generateMockTelemetry(selectedVehicleId)
        data.push({
          timestamp: pointDate.toLocaleString(),
          time: pointDate.getTime(),
          speed: Math.round(telemetry.telemetry.speed_kph * 10) / 10,
          rpm: Math.round(telemetry.telemetry.rpm / 100) / 10,
          temp: Math.round(telemetry.telemetry.engine_temp_c * 10) / 10,
          battery: Math.round(telemetry.telemetry.battery_voltage * 100) / 100,
          fuel: Math.round(telemetry.telemetry.fuel_level_pct),
          distance: Math.random() * 50,
        })
      }
    }

    setHistoricalData(data)

    // Calculate statistics
    if (data.length > 0) {
      const speeds = data.map((d) => d.speed)
      const temps = data.map((d) => d.temp)
      const batteries = data.map((d) => d.battery)
      const distances = data.map((d) => d.distance)

      const newStats: Statistics = {
        totalDistance: distances.reduce((a, b) => a + b, 0),
        averageSpeed: speeds.reduce((a, b) => a + b, 0) / speeds.length,
        maxSpeed: Math.max(...speeds),
        totalDuration: daysBack * 24,
        averageTemp: temps.reduce((a, b) => a + b, 0) / temps.length,
        maxTemp: Math.max(...temps),
        fuelConsumed: Math.abs(100 - data[data.length - 1].fuel + (100 - data[0].fuel)),
        avgBattery: batteries.reduce((a, b) => a + b, 0) / batteries.length,
      }
      setStats(newStats)
    }
  }, [selectedVehicleId, dateRange, filterType])

  const handleDateChange = (type: "start" | "end", date: string) => {
    const newDate = new Date(date)
    setDateRange((prev) => ({
      ...prev,
      [type]: newDate,
    }))
  }

  const handleExport = () => {
    const csv = [
      ["Timestamp", "Speed (km/h)", "RPM", "Temp (°C)", "Battery (V)", "Fuel (%)", "Distance (km)"],
      ...historicalData.map((d) => [d.timestamp, d.speed, d.rpm, d.temp, d.battery, d.fuel, d.distance]),
    ]
      .map((row) => row.join(","))
      .join("\n")

    const blob = new Blob([csv], { type: "text/csv" })
    const url = window.URL.createObjectURL(blob)
    const a = document.createElement("a")
    a.href = url
    a.download = `telemetry-history-${new Date().toISOString().split("T")[0]}.csv`
    a.click()
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div>
        <h1 className="text-3xl font-bold text-foreground">Historical Data</h1>
        <p className="text-sm text-muted-foreground mt-1">View and analyze vehicle telemetry history</p>
      </div>

      {/* Controls */}
      <Card className="border border-border bg-secondary/50">
        <CardContent className="pt-6">
          <div className="flex flex-col gap-4">
            <div className="flex items-center gap-4 flex-wrap">
              <div className="flex items-center gap-2">
                <Calendar className="w-4 h-4 text-muted-foreground" />
                <label className="text-sm text-muted-foreground">Start Date</label>
                <input
                  type="date"
                  value={dateRange.start.toISOString().split("T")[0]}
                  onChange={(e) => handleDateChange("start", e.target.value)}
                  className="px-2 py-1 rounded-md bg-input border border-border text-foreground text-sm"
                />
              </div>

              <div className="flex items-center gap-2">
                <Calendar className="w-4 h-4 text-muted-foreground" />
                <label className="text-sm text-muted-foreground">End Date</label>
                <input
                  type="date"
                  value={dateRange.end.toISOString().split("T")[0]}
                  onChange={(e) => handleDateChange("end", e.target.value)}
                  className="px-2 py-1 rounded-md bg-input border border-border text-foreground text-sm"
                />
              </div>
            </div>

            <div className="flex items-center gap-2 flex-wrap">
              <Filter className="w-4 h-4 text-muted-foreground" />
              <label className="text-sm text-muted-foreground">Granularity:</label>
              <div className="flex gap-2">
                {(["all", "daily", "weekly"] as const).map((type) => (
                  <Button
                    key={type}
                    variant={filterType === type ? "default" : "outline"}
                    size="sm"
                    onClick={() => setFilterType(type)}
                    className={filterType === type ? "bg-primary" : "bg-secondary/50"}
                  >
                    {type.charAt(0).toUpperCase() + type.slice(1)}
                  </Button>
                ))}
              </div>
            </div>

            <Button onClick={handleExport} className="w-fit gap-2 bg-primary hover:bg-primary/90">
              <Download className="w-4 h-4" />
              Export as CSV
            </Button>
          </div>
        </CardContent>
      </Card>

      {/* Statistics Cards */}
      <div className="grid grid-cols-2 lg:grid-cols-4 gap-4">
        <Card className="border border-border bg-secondary/50">
          <CardContent className="pt-4">
            <div className="text-xs text-muted-foreground uppercase tracking-wide">Total Distance</div>
            <div className="flex items-end justify-between mt-2">
              <div className="text-2xl font-bold text-accent">{stats.totalDistance.toFixed(1)}</div>
              <span className="text-xs text-muted-foreground">km</span>
            </div>
          </CardContent>
        </Card>

        <Card className="border border-border bg-secondary/50">
          <CardContent className="pt-4">
            <div className="text-xs text-muted-foreground uppercase tracking-wide">Avg Speed</div>
            <div className="flex items-end justify-between mt-2">
              <div className="text-2xl font-bold text-accent">{stats.averageSpeed.toFixed(1)}</div>
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
              <div className="text-2xl font-bold text-accent">{stats.averageTemp.toFixed(1)}</div>
              <span className="text-xs text-muted-foreground">°C</span>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Charts */}
      <div className="grid grid-cols-2 gap-4">
        <Card className="border border-border col-span-2 lg:col-span-1">
          <CardHeader className="pb-2">
            <CardTitle className="text-base">Speed Over Time</CardTitle>
          </CardHeader>
          <CardContent>
            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={historicalData} margin={{ top: 5, right: 10, left: -20, bottom: 5 }}>
                <CartesianGrid strokeDasharray="3 3" stroke="oklch(0.25 0 0)" />
                <XAxis dataKey="timestamp" stroke="oklch(0.70 0 0)" fontSize={11} angle={-45} height={80} />
                <YAxis stroke="oklch(0.70 0 0)" fontSize={12} />
                <Tooltip />
                <Area type="monotone" dataKey="speed" fill="oklch(0.65 0.30 240)" stroke="oklch(0.65 0.30 240)" />
              </LineChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>

        <Card className="border border-border col-span-2 lg:col-span-1">
          <CardHeader className="pb-2">
            <CardTitle className="text-base">Temperature Trend</CardTitle>
          </CardHeader>
          <CardContent>
            <ResponsiveContainer width="100%" height={300}>
              <AreaChart data={historicalData} margin={{ top: 5, right: 10, left: -20, bottom: 5 }}>
                <CartesianGrid strokeDasharray="3 3" stroke="oklch(0.25 0 0)" />
                <XAxis dataKey="timestamp" stroke="oklch(0.70 0 0)" fontSize={11} angle={-45} height={80} />
                <YAxis stroke="oklch(0.70 0 0)" fontSize={12} />
                <Tooltip />
                <Area type="monotone" dataKey="temp" fill="oklch(0.60 0.24 30)" stroke="oklch(0.60 0.24 30)" />
              </AreaChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>

        <Card className="border border-border col-span-2">
          <CardHeader className="pb-2">
            <CardTitle className="text-base">Fuel Level & Battery Voltage</CardTitle>
          </CardHeader>
          <CardContent>
            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={historicalData} margin={{ top: 5, right: 10, left: -20, bottom: 5 }}>
                <CartesianGrid strokeDasharray="3 3" stroke="oklch(0.25 0 0)" />
                <XAxis dataKey="timestamp" stroke="oklch(0.70 0 0)" fontSize={11} angle={-45} height={80} />
                <YAxis yAxisId="left" stroke="oklch(0.70 0 0)" fontSize={12} />
                <YAxis yAxisId="right" orientation="right" stroke="oklch(0.70 0 0)" fontSize={12} />
                <Tooltip />
                <Legend />
                <Line
                  yAxisId="left"
                  type="monotone"
                  dataKey="fuel"
                  stroke="oklch(0.65 0.30 240)"
                  strokeWidth={2}
                  name="Fuel (%)"
                />
                <Line
                  yAxisId="right"
                  type="monotone"
                  dataKey="battery"
                  stroke="oklch(0.52 0.25 120)"
                  strokeWidth={2}
                  name="Battery (V)"
                />
              </LineChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>
      </div>

      {/* Summary */}
      <Card className="border border-border bg-secondary/50">
        <CardHeader className="pb-3">
          <CardTitle className="text-base">Summary</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-2 lg:grid-cols-4 gap-4 text-sm">
            <div>
              <div className="text-muted-foreground">Total Duration</div>
              <div className="text-lg font-semibold text-foreground mt-1">{stats.totalDuration} hours</div>
            </div>
            <div>
              <div className="text-muted-foreground">Fuel Consumed</div>
              <div className="text-lg font-semibold text-foreground mt-1">{stats.fuelConsumed.toFixed(1)} L</div>
            </div>
            <div>
              <div className="text-muted-foreground">Max Temperature</div>
              <div className="text-lg font-semibold text-foreground mt-1">{stats.maxTemp.toFixed(1)}°C</div>
            </div>
            <div>
              <div className="text-muted-foreground">Avg Battery</div>
              <div className="text-lg font-semibold text-foreground mt-1">{stats.avgBattery.toFixed(2)} V</div>
            </div>
          </div>
        </CardContent>
      </Card>
    </div>
  )
}
