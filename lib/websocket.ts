type WebSocketMessageHandler = (data: any) => void

interface WebSocketConfig {
  url: string
  token: string
  vehicleId: string
}

class TelemetryWebSocket {
  private ws: WebSocket | null = null
  private url = ""
  private token = ""
  private vehicleId = ""
  private reconnectAttempts = 0
  private maxReconnectAttempts = 5
  private reconnectDelay = 3000
  private messageHandlers: Map<string, WebSocketMessageHandler[]> = new Map()
  private heartbeatInterval: NodeJS.Timeout | null = null

  connect(config: WebSocketConfig): Promise<void> {
    return new Promise((resolve, reject) => {
      try {
        this.url = config.url
        this.token = config.token
        this.vehicleId = config.vehicleId

        const wsUrl = `${config.url}?token=${config.token}&vehicleId=${config.vehicleId}`
        this.ws = new WebSocket(wsUrl)

        this.ws.onopen = () => {
          console.log("[v0] WebSocket connected")
          this.reconnectAttempts = 0
          this.startHeartbeat()
          resolve()
        }

        this.ws.onmessage = (event) => {
          this.handleMessage(event.data)
        }

        this.ws.onerror = (error) => {
          console.error("[v0] WebSocket error:", error)
          reject(error)
        }

        this.ws.onclose = () => {
          console.log("[v0] WebSocket disconnected")
          this.stopHeartbeat()
          this.attemptReconnect()
        }
      } catch (error) {
        reject(error)
      }
    })
  }

  private handleMessage(data: string) {
    try {
      const message = JSON.parse(data)
      const { type, payload } = message

      const handlers = this.messageHandlers.get(type) || []
      handlers.forEach((handler) => handler(payload))
    } catch (error) {
      console.error("[v0] Failed to parse WebSocket message:", error)
    }
  }

  private startHeartbeat() {
    this.heartbeatInterval = setInterval(() => {
      if (this.ws?.readyState === WebSocket.OPEN) {
        this.send({ type: "ping" })
      }
    }, 30000)
  }

  private stopHeartbeat() {
    if (this.heartbeatInterval) {
      clearInterval(this.heartbeatInterval)
      this.heartbeatInterval = null
    }
  }

  private attemptReconnect() {
    if (this.reconnectAttempts < this.maxReconnectAttempts) {
      this.reconnectAttempts++
      console.log(`[v0] Attempting to reconnect... (${this.reconnectAttempts}/${this.maxReconnectAttempts})`)
      setTimeout(() => {
        this.connect({
          url: this.url,
          token: this.token,
          vehicleId: this.vehicleId,
        })
      }, this.reconnectDelay * this.reconnectAttempts)
    } else {
      console.error("[v0] Max reconnection attempts reached")
    }
  }

  on(type: string, handler: WebSocketMessageHandler) {
    if (!this.messageHandlers.has(type)) {
      this.messageHandlers.set(type, [])
    }
    this.messageHandlers.get(type)?.push(handler)
  }

  off(type: string, handler: WebSocketMessageHandler) {
    const handlers = this.messageHandlers.get(type) || []
    const index = handlers.indexOf(handler)
    if (index > -1) {
      handlers.splice(index, 1)
    }
  }

  send(message: any) {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(message))
    } else {
      console.error("[v0] WebSocket is not connected")
    }
  }

  disconnect() {
    this.stopHeartbeat()
    if (this.ws) {
      this.ws.close()
      this.ws = null
    }
  }

  isConnected(): boolean {
    return this.ws?.readyState === WebSocket.OPEN
  }
}

export const telemetryWS = new TelemetryWebSocket()
