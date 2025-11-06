"use client"

import { useVehicleStore } from "@/lib/store"
import { Wifi, WifiOff } from "lucide-react"

interface VehicleSelectorProps {
  compact?: boolean
}

export function VehicleSelector({ compact = false }: VehicleSelectorProps) {
  const vehicles = useVehicleStore((state) => state.vehicles)
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const selectVehicle = useVehicleStore((state) => state.selectVehicle)

  if (compact) {
    return (
      <div className="space-y-2">
        <div className="text-xs uppercase font-semibold text-muted-foreground">Active Vehicle</div>
        <div className="space-y-1">
          {vehicles.map((vehicle) => (
            <button
              key={vehicle.id}
              onClick={() => selectVehicle(vehicle.id)}
              className={`w-full px-3 py-2 rounded-md text-left text-sm transition flex items-center gap-2 ${
                selectedVehicleId === vehicle.id
                  ? "bg-primary text-primary-foreground font-medium"
                  : "bg-secondary/50 text-foreground hover:bg-secondary"
              }`}
            >
              {vehicle.status === "online" ? (
                <Wifi className="w-3 h-3 flex-shrink-0 text-green-400" />
              ) : (
                <WifiOff className="w-3 h-3 flex-shrink-0 text-red-400" />
              )}
              <span className="truncate">{vehicle.id}</span>
            </button>
          ))}
        </div>
      </div>
    )
  }

  return null
}
