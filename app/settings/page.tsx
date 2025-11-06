"use client"

import { useState, useEffect } from "react"
import { useRouter } from "next/navigation"
import { getStoredToken } from "@/lib/api"
import { Sidebar } from "@/components/sidebar"
import { TopNavigation } from "@/components/top-navigation"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { Label } from "@/components/ui/label"
import { Button } from "@/components/ui/button"
import { RadioGroup, RadioGroupItem } from "@/components/ui/radio-group"
import { useToast } from "@/hooks/use-toast"

const UPDATE_INTERVALS = [
  { value: "1000", label: "1 second", description: "Real-time updates (high bandwidth)" },
  { value: "2000", label: "2 seconds", description: "Default - balanced performance" },
  { value: "5000", label: "5 seconds", description: "Reduced updates" },
  { value: "10000", label: "10 seconds", description: "Low bandwidth mode" },
  { value: "30000", label: "30 seconds", description: "Minimal updates" },
  { value: "60000", label: "1 minute", description: "Very low bandwidth" },
]

export default function SettingsPage() {
  const router = useRouter()
  const { toast } = useToast()
  const [updateInterval, setUpdateInterval] = useState("2000")
  const [apiUrl, setApiUrl] = useState("")

  useEffect(() => {
    const token = getStoredToken()
    if (!token) {
      router.push("/login")
      return
    }

    // Load saved settings
    const savedInterval = localStorage.getItem("telemetry_update_interval") || "2000"
    setUpdateInterval(savedInterval)
    setApiUrl(process.env.NEXT_PUBLIC_API_URL || "")
  }, [router])

  const handleSave = () => {
    localStorage.setItem("telemetry_update_interval", updateInterval)
    
    toast({
      title: "Settings saved",
      description: `Update interval set to ${UPDATE_INTERVALS.find(i => i.value === updateInterval)?.label}. Please refresh the dashboard.`,
    })

    // Trigger a page refresh to apply settings
    setTimeout(() => {
      window.location.href = "/dashboard"
    }, 1500)
  }

  const handleReset = () => {
    setUpdateInterval("2000")
    localStorage.setItem("telemetry_update_interval", "2000")
    
    toast({
      title: "Settings reset",
      description: "Default settings have been restored.",
    })
  }

  return (
    <div className="flex h-screen overflow-hidden">
      <Sidebar />
      <div className="flex-1 flex flex-col overflow-hidden">
        <TopNavigation />
        <div className="flex-1 overflow-y-auto p-6">
          <div className="max-w-3xl mx-auto space-y-6">
            <div>
              <h1 className="text-3xl font-bold">Settings</h1>
              <p className="text-muted-foreground mt-2">Configure your telemetry dashboard preferences</p>
            </div>

            {/* Update Interval Settings */}
            <Card>
              <CardHeader>
                <CardTitle>Telemetry Update Interval</CardTitle>
                <CardDescription>
                  Choose how frequently the dashboard fetches new data from the backend
                </CardDescription>
              </CardHeader>
              <CardContent className="space-y-4">
                <RadioGroup value={updateInterval} onValueChange={setUpdateInterval}>
                  {UPDATE_INTERVALS.map((interval) => (
                    <div key={interval.value} className="flex items-center space-x-3 space-y-0">
                      <RadioGroupItem value={interval.value} id={interval.value} />
                      <Label htmlFor={interval.value} className="flex flex-col cursor-pointer">
                        <span className="font-medium">{interval.label}</span>
                        <span className="text-sm text-muted-foreground">{interval.description}</span>
                      </Label>
                    </div>
                  ))}
                </RadioGroup>

                <div className="pt-4 border-t">
                  <div className="text-sm text-muted-foreground">
                    <strong>Note:</strong> Faster update intervals provide more real-time data but consume more
                    bandwidth and may increase CPU usage.
                  </div>
                </div>
              </CardContent>
            </Card>

            {/* Connection Settings */}
            <Card>
              <CardHeader>
                <CardTitle>Connection Information</CardTitle>
                <CardDescription>Current backend API configuration</CardDescription>
              </CardHeader>
              <CardContent className="space-y-3">
                <div>
                  <Label className="text-sm font-medium">Backend API URL</Label>
                  <div className="mt-1 p-3 bg-muted rounded-md font-mono text-sm">{apiUrl || "Not configured"}</div>
                </div>
                <div className="text-sm text-muted-foreground">
                  To change the API URL, update the <code className="bg-muted px-1 py-0.5 rounded">.env.local</code>{" "}
                  file
                </div>
              </CardContent>
            </Card>

            {/* Action Buttons */}
            <div className="flex gap-4">
              <Button onClick={handleSave} size="lg">
                Save Settings
              </Button>
              <Button onClick={handleReset} variant="outline" size="lg">
                Reset to Defaults
              </Button>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
