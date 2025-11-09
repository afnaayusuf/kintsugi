const API_BASE = process.env.NEXT_PUBLIC_API_URL || "https://kintsugi-backend.onrender.com/api/v1"

interface ApiOptions extends RequestInit {
  token?: string
}

export async function apiCall<T>(endpoint: string, options: ApiOptions = {}): Promise<T> {
  const { token, ...fetchOptions } = options
  const url = `${API_BASE}${endpoint}`

  console.log(`[API] 🔑 Token present: ${!!token}, Token length: ${token?.length || 0}`)

  const headers: Record<string, string> = {
    "Content-Type": "application/json",
    ...(fetchOptions.headers as Record<string, string>),
  }

  if (token) {
    headers["Authorization"] = `Bearer ${token}`
    console.log(`[API] 📤 Sending Authorization header: Bearer ${token.substring(0, 20)}...`)
  } else {
    console.log(`[API] ⚠️ No token provided - request will be unauthorized`)
  }

  console.log(`[API] 🔄 Calling: ${url}`)

  const response = await fetch(url, {
    ...fetchOptions,
    headers,
  })

  console.log(`[API] 📡 Response: ${response.status} ${response.statusText}`)

  if (!response.ok) {
    console.error(`[API] ❌ Error: ${response.status} ${response.statusText}`)
    throw new Error(`API Error: ${response.status} ${response.statusText}`)
  }

  return response.json()
}

export function getStoredToken(): string | null {
  if (typeof window === "undefined") return null
  return localStorage.getItem("telemetry_token")
}

export function setStoredToken(token: string): void {
  if (typeof window === "undefined") return
  localStorage.setItem("telemetry_token", token)
}

export function clearStoredToken(): void {
  if (typeof window === "undefined") return
  localStorage.removeItem("telemetry_token")
}

export async function fetchVehicles(token: string): Promise<{ vehicles: any[] }> {
  return apiCall<{ vehicles: any[] }>("/vehicles", {
    token,
  })
}

export async function fetchTelemetry(vehicleId: string, token: string): Promise<any> {
  return apiCall<any>(`/telemetry/${vehicleId}/current`, {
    token,
  })
}

export async function fetchHistoricalData(
  vehicleId: string,
  startDate: string,
  endDate: string,
  token: string,
): Promise<any> {
  const params = new URLSearchParams({
    start_date: startDate,
    end_date: endDate,
  })
  return apiCall<any>(`/telemetry/${vehicleId}/history?${params}`, {
    token,
  })
}

export async function sendVehicleCommand(
  vehicleId: string,
  command: string,
  params: any,
  token: string,
): Promise<{ success: boolean; message: string }> {
  return apiCall<{ success: boolean; message: string }>(`/vehicles/${vehicleId}/command`, {
    method: "POST",
    body: JSON.stringify({ command, params }),
    token,
  })
}

export async function getSystemHealth(vehicleId: string, token: string): Promise<any> {
  return apiCall<any>(`/vehicles/${vehicleId}/health`, {
    token,
  })
}
