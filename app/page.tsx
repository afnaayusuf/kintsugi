"use client"

import { useEffect } from "react"
import { useRouter } from "next/navigation"
import { getStoredToken } from "@/lib/api"
import { useVehicleStore } from "@/lib/store"

export default function Home() {
  const router = useRouter()
  const isAuthenticated = useVehicleStore((state) => state.isAuthenticated)

  useEffect(() => {
    const token = getStoredToken()
    if (token) {
      router.push("/dashboard")
    } else {
      router.push("/login")
    }
  }, [router, isAuthenticated])

  return null
}
