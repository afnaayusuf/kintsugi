"use client"

import type React from "react"

import { useEffect, useState } from "react"
import { useVehicleStore } from "@/lib/store"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { HardDrive, Cpu, Network, Clock, AlertTriangle } from "lucide-react"

interface HealthMetric {
  label: string
  value: string | number
  unit?: string
  icon: React.ReactNode
  percentage?: number
  status: "healthy" | "warning" | "critical"
}

function ProgressBar({ percentage, status }: { percentage: number; status: "healthy" | "warning" | "critical" }) {
  const statusColors = {
    healthy: "bg-green-500",
    warning: "bg-yellow-500",
    critical: "bg-red-500",
  }

  return (
    <div className="w-full bg-muted rounded-full h-2 overflow-hidden">
      <div
        className={`h-full transition-all duration-300 ${statusColors[status]}`}
        style={{ width: `${Math.min(percentage, 100)}%` }}
      />
    </div>
  )
}

function HealthMetricCard({ metric }: { metric: HealthMetric }) {
  const Icon = metric.icon
  const statusIndicatorColor = {
    healthy: "bg-green-500/20 border-green-500/50",
    warning: "bg-yellow-500/20 border-yellow-500/50",
    critical: "bg-red-500/20 border-red-500/50",
  }

  const statusTextColor = {
    healthy: "text-green-400",
    warning: "text-yellow-400",
    critical: "text-red-400",
  }

  return (
    <div className={`p-3 rounded-lg border transition ${statusIndicatorColor[metric.status]}`}>
      <div className="flex items-center gap-2 mb-2">
        <div
          className={`p-1.5 rounded-md ${metric.status === "healthy" ? "bg-green-500/20" : metric.status === "warning" ? "bg-yellow-500/20" : "bg-red-500/20"}`}
        >
          {Icon}
        </div>
        <span className="text-xs text-muted-foreground uppercase tracking-wide flex-1">{metric.label}</span>
        <span className={`text-xs font-semibold ${statusTextColor[metric.status]}`}>
          {metric.status === "healthy" ? "OK" : metric.status === "warning" ? "WARN" : "CRIT"}
        </span>
      </div>
      <div className="flex items-baseline gap-1 mb-2">
        <div className="text-lg font-bold text-foreground">{metric.value}</div>
        {metric.unit && <span className="text-xs text-muted-foreground">{metric.unit}</span>}
      </div>
      {metric.percentage !== undefined && <ProgressBar percentage={metric.percentage} status={metric.status} />}
    </div>
  )
}

export function SystemHealthPanel() {
  const currentTelemetry = useVehicleStore((state) => state.currentTelemetry)
  const [uptime, setUptime] = useState("0h 0m")
  const [metrics, setMetrics] = useState<HealthMetric[]>([])

  useEffect(() => {
    if (!currentTelemetry) return

    const lastSync = new Date(currentTelemetry.system.last_sync)
    const now = new Date(currentTelemetry.timestamp)
    const diff = (now.getTime() - lastSync.getTime()) / 1000
    const hours = Math.floor(diff / 3600)
    const minutes = Math.floor((diff % 3600) / 60)
    setUptime(`${hours}h ${minutes}m`)

    const cpu = currentTelemetry.system.cpu_usage_pct
    const ram = currentTelemetry.system.ram_usage_pct
    const latency = currentTelemetry.system.network_latency_ms

    const newMetrics: HealthMetric[] = [
      {
        label: "CPU Usage",
        value: cpu.toFixed(1),
        unit: "%",
        icon: <Cpu className="w-4 h-4 text-blue-400" />,
        percentage: cpu,
        status: cpu > 80 ? "critical" : cpu > 60 ? "warning" : "healthy",
      },
      {
        label: "Memory Usage",
        value: ram.toFixed(1),
        unit: "%",
        icon: <HardDrive className="w-4 h-4 text-cyan-400" />,
        percentage: ram,
        status: ram > 80 ? "critical" : ram > 70 ? "warning" : "healthy",
      },
      {
        label: "Network Latency",
        value: latency.toFixed(0),
        unit: "ms",
        icon: <Network className="w-4 h-4 text-green-400" />,
        percentage: Math.min((latency / 200) * 100, 100),
        status: latency > 150 ? "critical" : latency > 100 ? "warning" : "healthy",
      },
      {
        label: "System Uptime",
        value: uptime,
        icon: <Clock className="w-4 h-4 text-purple-400" />,
        status: "healthy",
      },
    ]

    setMetrics(newMetrics)
  }, [currentTelemetry])

  if (!currentTelemetry || metrics.length === 0) return null

  return (
    <Card className="border border-border col-span-1">
      <CardHeader className="pb-3">
        <CardTitle className="text-base flex items-center gap-2">
          System Health
          {metrics.some((m) => m.status !== "healthy") && <AlertTriangle className="w-4 h-4 text-yellow-400" />}
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-3">
        {metrics.map((metric, idx) => (
          <HealthMetricCard key={idx} metric={metric} />
        ))}
      </CardContent>
    </Card>
  )
}
