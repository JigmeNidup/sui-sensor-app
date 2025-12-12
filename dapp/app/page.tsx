"use client"

import { ConnectButton, useCurrentAccount } from "@mysten/dapp-kit"
import SensorForm from "@/components/SensorForm"
import SensorList from "@/components/SensorList"
import { useState } from "react"

export default function Home() {
  const account = useCurrentAccount()
  const [refreshKey, setRefreshKey] = useState(0)

  const handleDataStored = () => {
    setRefreshKey(prev => prev + 1)
  }

  return (
    <div className="mx-auto px-4 py-8 max-w-7xl">
      {/* Header */}
      <header className="flex flex-col md:flex-row justify-between items-center mb-8">
        <div>
          <h1 className="text-4xl font-bold bg-linear-to-r from-green-400 to-blue-500 bg-clip-text text-transparent">
            Sui Sensor Data Storage
          </h1>
          <p className="text-gray-400 mt-2">
            Store IoT sensor data permanently on the Sui blockchain
          </p>
        </div>
        <div className="mt-4 md:mt-0">
          <ConnectButton />
        </div>
      </header>

      {/* Connection Status */}
      {account && (
        <div className="mb-6 p-4 bg-green-500/10 border border-green-500/30 rounded-lg">
          <div className="flex items-center">
            <div className="h-3 w-3 rounded-full bg-green-500 mr-2"></div>
            <span className="text-green-300">
              Wallet Connected: {account.address.slice(0, 8)}...{account.address.slice(-6)}
            </span>
          </div>
        </div>
      )}

      {/* Main Content */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-8 mb-12">
        {/* Left Column - Sensor Form */}
        <div>
          <SensorForm onDataStored={handleDataStored} />
          
          {/* Info Box */}
          <div className="mt-8 p-4 bg-blue-500/10 border border-blue-500/30 rounded-lg">
            <h3 className="font-semibold text-blue-300 mb-2">Supported Sensor Types:</h3>
            <ul className="text-sm text-gray-300 space-y-1">
              <li>• Soil Sensors: Temperature, Humidity, EC, pH</li>
              <li>• Air Quality: Temperature, Humidity</li>
              <li>• Water Quality: Temperature, EC, pH</li>
              <li>• Weather Stations: Temperature, Humidity</li>
              <li>• Industrial IoT: All sensor types</li>
            </ul>
          </div>
        </div>

        {/* Right Column - Sensor Data List */}
        <div>
          <SensorList key={refreshKey} />
        </div>
      </div>

      {/* Footer Info */}
      <div className="mt-12 pt-8 border-t border-gray-700">
        <div className="text-center text-gray-400 text-sm">
          <p>
            Powered by Sui Blockchain • 
            <a 
              href="https://docs.sui.io/" 
              target="_blank" 
              rel="noopener noreferrer"
              className="text-blue-400 hover:text-blue-300 ml-1"
            >
              Sui Documentation
            </a>
          </p>
          <p className="mt-1">
            Need test SUI? Get it from the{" "}
            <a 
              href="https://docs.sui.io/guides/developer/getting-started/sui-faucet" 
              target="_blank" 
              rel="noopener noreferrer"
              className="text-purple-400 hover:text-purple-300"
            >
              Sui Faucet
            </a>
          </p>
        </div>
      </div>
    </div>
  )
}