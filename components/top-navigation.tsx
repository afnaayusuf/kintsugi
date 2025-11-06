"use client"

import { useVehicleStore } from "@/lib/store"
import { Button } from "@/components/ui/button"
import { ChevronDown, Wifi, WifiOff, Zap } from "lucide-react"
import { useState } from "react"

export function TopNavigation() {
  const vehicles = useVehicleStore((state) => state.vehicles)
  const selectedVehicleId = useVehicleStore((state) => state.selectedVehicleId)
  const selectVehicle = useVehicleStore((state) => state.selectVehicle)
  const [isOpen, setIsOpen] = useState(false)

  const selectedVehicle = vehicles.find((v) => v.id === selectedVehicleId)

  return (
    <div className="border-b border-border bg-gradient-to-r from-card/80 to-card/40 backdrop-blur-md sticky top-0 z-40">
      <div className="px-6 py-4 flex items-center justify-between">
        {/* Left side - Logo and title */}
        <div className="flex items-center gap-3">
          <div className="flex items-center justify-center w-10 h-10 rounded-lg bg-gradient-to-br from-accent to-accent/50 shadow-lg shadow-accent/20">
            <Zap className="w-6 h-6 text-accent-foreground" />
          </div>
          <h1 className="text-xl font-bold bg-gradient-to-r from-accent to-primary bg-clip-text text-transparent">
            Kintsugi
          </h1>
        </div>

        {/* Center - Vehicle Selector */}
        <div className="flex-1 max-w-xs mx-8">
          <div className="relative">
            <Button
              onClick={() => setIsOpen(!isOpen)}
              variant="outline"
              className="w-full justify-between bg-gradient-to-r from-secondary/50 to-secondary/30 hover:from-secondary to-secondary/40 border-border/50 hover:border-primary/50 transition-all"
            >
              <div className="flex items-center gap-2 min-w-0">
                {selectedVehicle && (
                  <>
                    {selectedVehicle.status === "online" ? (
                      <Wifi className="w-4 h-4 text-green-400 flex-shrink-0 animate-pulse" />
                    ) : (
                      <WifiOff className="w-4 h-4 text-red-400 flex-shrink-0" />
                    )}
                    <div className="text-left min-w-0">
                      <div className="text-sm font-semibold truncate">{selectedVehicle.id}</div>
                      <div className="text-xs text-muted-foreground truncate">{selectedVehicle.model}</div>
                    </div>
                  </>
                )}
              </div>
              <ChevronDown
                className={`w-4 h-4 text-muted-foreground flex-shrink-0 transition-transform ${
                  isOpen ? "rotate-180" : ""
                }`}
              />
            </Button>

            {/* Dropdown Menu */}
            {isOpen && (
              <div className="absolute top-full left-0 right-0 mt-2 bg-card border border-border/50 rounded-lg shadow-2xl z-50 backdrop-blur-sm">
                <div className="p-2 space-y-1 max-h-64 overflow-y-auto">
                  {vehicles.map((vehicle) => (
                    <button
                      key={vehicle.id}
                      onClick={() => {
                        selectVehicle(vehicle.id)
                        setIsOpen(false)
                      }}
                      className={`w-full px-3 py-2 rounded-md text-left transition-all flex items-center gap-3 ${
                        selectedVehicleId === vehicle.id
                          ? "bg-gradient-to-r from-primary to-primary/80 text-primary-foreground shadow-lg shadow-primary/30"
                          : "bg-secondary/50 text-foreground hover:bg-secondary/80"
                      }`}
                    >
                      {vehicle.status === "online" ? (
                        <Wifi className="w-4 h-4 text-green-400 flex-shrink-0 animate-pulse" />
                      ) : (
                        <WifiOff className="w-4 h-4 text-red-400 flex-shrink-0" />
                      )}
                      <div className="min-w-0 flex-1">
                        <div className="text-sm font-semibold truncate">{vehicle.id}</div>
                        <div className="text-xs opacity-75 truncate">{vehicle.model}</div>
                      </div>
                    </button>
                  ))}
                </div>
              </div>
            )}
          </div>
        </div>

        {/* Right side - Status indicator */}
        <div className="flex items-center gap-3 px-4 py-2 rounded-lg bg-secondary/30 border border-border/30">
          <div className="text-right">
            <div className="text-sm font-semibold text-foreground">
              {selectedVehicle?.status === "online" ? "Connected" : "Disconnected"}
            </div>
            <div className="text-xs text-muted-foreground">Real-time Data</div>
          </div>
          <div
            className={`w-3 h-3 rounded-full shadow-lg transition-all ${
              selectedVehicle?.status === "online"
                ? "bg-green-400 shadow-green-400/50 animate-pulse"
                : "bg-red-400 shadow-red-400/50"
            }`}
          ></div>
        </div>
      </div>
    </div>
  )
}
