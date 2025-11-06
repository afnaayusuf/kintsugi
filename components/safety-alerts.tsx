"use client"

import type React from "react"

import { useVehicleStore } from "@/lib/store"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { AlertCircle, CheckCircle2, AlertTriangle, Shield } from "lucide-react"

interface SafetyStatus {
  label: string
  active: boolean
  icon: React.ReactNode
  severity: "info" | "warning" | "critical"
}

export function SafetyAlerts() {
  const currentTelemetry = useVehicleStore((state) => state.currentTelemetry)

  if (!currentTelemetry) return null

  const safetyStatuses: SafetyStatus[] = [
    {
      label: "ABS Active",
      active: currentTelemetry.status.ABS_active,
      icon: (
        <AlertCircle
          className={`w-4 h-4 ${currentTelemetry.status.ABS_active ? "text-yellow-400" : "text-gray-500"}`}
        />
      ),
      severity: currentTelemetry.status.ABS_active ? "warning" : "info",
    },
    {
      label: "Traction Control",
      active: currentTelemetry.status.traction_control,
      icon: (
        <Shield
          className={`w-4 h-4 ${currentTelemetry.status.traction_control ? "text-green-400" : "text-gray-500"}`}
        />
      ),
      severity: currentTelemetry.status.traction_control ? "info" : "warning",
    },
  ]

  const hasDTC = currentTelemetry.status.DTC.length > 0
  const hasWarnings = safetyStatuses.some((s) => s.severity === "warning")

  return (
    <Card className="border border-border col-span-1">
      <CardHeader className="pb-3">
        <CardTitle className="text-base flex items-center gap-2">
          Safety & Diagnostics
          {hasDTC && <AlertTriangle className="w-4 h-4 text-red-400" />}
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-3">
        {/* Safety statuses */}
        {safetyStatuses.map((status, idx) => (
          <div
            key={idx}
            className={`p-3 rounded-lg border transition ${
              status.severity === "critical"
                ? "bg-red-500/10 border-red-500/50"
                : status.severity === "warning"
                  ? "bg-yellow-500/10 border-yellow-500/50"
                  : "bg-green-500/10 border-green-500/50"
            }`}
          >
            <div className="flex items-center gap-2 mb-1">
              {status.icon}
              <span className="text-xs text-muted-foreground flex-1">{status.label}</span>
              <span
                className={`text-xs font-semibold ${
                  status.severity === "critical"
                    ? "text-red-400"
                    : status.severity === "warning"
                      ? "text-yellow-400"
                      : "text-green-400"
                }`}
              >
                {status.active ? (status.severity === "warning" ? "ACTIVE" : "ON") : "OFF"}
              </span>
            </div>
          </div>
        ))}

        {/* Overall system status */}
        <div
          className={`p-3 rounded-lg border transition ${
            hasDTC
              ? "bg-red-900/20 border-red-500/50"
              : hasWarnings
                ? "bg-yellow-900/20 border-yellow-500/50"
                : "bg-green-900/20 border-green-500/50"
          }`}
        >
          <div className="flex items-center gap-2">
            {hasDTC ? (
              <>
                <AlertCircle className="w-4 h-4 text-red-400 flex-shrink-0" />
                <div>
                  <span className="text-xs font-semibold text-red-400">System Alert</span>
                  <p className="text-xs text-red-300 mt-0.5">{currentTelemetry.status.DTC.length} diagnostic codes</p>
                </div>
              </>
            ) : hasWarnings ? (
              <>
                <AlertTriangle className="w-4 h-4 text-yellow-400 flex-shrink-0" />
                <span className="text-xs font-semibold text-yellow-400">Safety Systems Active</span>
              </>
            ) : (
              <>
                <CheckCircle2 className="w-4 h-4 text-green-400 flex-shrink-0" />
                <span className="text-xs font-semibold text-green-400">All Systems Optimal</span>
              </>
            )}
          </div>
        </div>

        {/* Diagnostic codes if present */}
        {hasDTC && (
          <div className="bg-red-500/5 p-2 rounded-md border border-red-500/20">
            <p className="text-xs text-red-300">Codes:</p>
            <div className="text-xs text-red-400 font-mono mt-1">{currentTelemetry.status.DTC.join(", ")}</div>
          </div>
        )}
      </CardContent>
    </Card>
  )
}
