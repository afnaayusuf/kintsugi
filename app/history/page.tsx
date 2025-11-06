"use client"

import { useEffect } from "react"
import { useRouter } from "next/navigation"
import { getStoredToken } from "@/lib/api"
import { Sidebar } from "@/components/sidebar"
import { TopNavigation } from "@/components/top-navigation"
import { HistoricalDataViewer } from "@/components/historical-data-viewer"

export default function HistoryPage() {
  const router = useRouter()

  useEffect(() => {
    const token = getStoredToken()
    if (!token) {
      router.push("/login")
    }
  }, [router])

  return (
    <div className="flex h-screen overflow-hidden">
      <Sidebar />
      <div className="flex-1 flex flex-col overflow-hidden">
        <TopNavigation />
        <div className="flex-1 overflow-y-auto">
          <div className="p-6">
            <HistoricalDataViewer />
          </div>
        </div>
      </div>
    </div>
  )
}
