"use client";

import { useEffect, useState } from "react";
import { useCurrentAccount, useSuiClient } from "@mysten/dapp-kit";
import { SensorData, SensorStats } from "@/lib/types";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
} from "recharts";

// 1. Define the Backend Address for querying (CRITICAL FIX)
// This MUST match the SUI_SENDER_ADDRESS in your API route's environment variables.
// You need to set this as a NEXT_PUBLIC_ variable in your .env file.
const SYSTEM_OWNER_ADDRESS = process.env.NEXT_PUBLIC_SUI_SENDER_ADDRESS || "";

// Helper to calculate stats for a given dataset
const calculateStats = (data: SensorData[]): SensorStats | null => {
  if (data.length === 0) {
    return null;
  }
  return {
    avgTemperature:
      data.reduce((sum, d) => sum + d.temperature, 0) / data.length,
    avgHumidity: data.reduce((sum, d) => sum + d.humidity, 0) / data.length,
    avgEC: data.reduce((sum, d) => sum + d.ec, 0) / data.length,
    avgPH: data.reduce((sum, d) => sum + d.ph, 0) / data.length,
    totalReadings: data.length,
  };
};

export default function SensorList() {
  const account = useCurrentAccount();
  const client = useSuiClient();
  const PACKAGE_ID = process.env.NEXT_PUBLIC_SENSOR_PACKAGE_ID || "";

  // State for User-Owned Data (The 'Old' Logic)
  const [userData, setUserData] = useState<SensorData[]>([]);
  const [userStats, setUserStats] = useState<SensorStats | null>(null);
  const [userLoading, setUserLoading] = useState(false);
  const [userError, setUserError] = useState<string | null>(null);

  // State for System-Owned Data (The 'New' Logic)
  const [systemData, setSystemData] = useState<SensorData[]>([]);
  const [systemStats, setSystemStats] = useState<SensorStats | null>(null);
  const [systemLoading, setSystemLoading] = useState(false);
  const [systemError, setSystemError] = useState<string | null>(null);

  // --- Fetching Logic for User-Owned Data ---
  const fetchUserData = async () => {
    if (!account || !PACKAGE_ID) {
      setUserData([]);
      return;
    }

    setUserLoading(true);
    setUserError(null);

    try {
      // Query for SensorData objects OWNED BY THE CURRENT CONNECTED WALLET
      const ownedObjects = await client.getOwnedObjects({
        owner: account.address, // <--- User's Wallet Address
        filter: {
          StructType: `${PACKAGE_ID}::sensor_storage::SensorData`,
        },
        options: {
          showContent: true,
          showType: true,
        },
      });

      const sensors = await Promise.all(
        ownedObjects.data.map(async (obj) => {
          if (obj.data?.content) {
            const fields = (obj.data.content as any).fields;
            return {
              id: obj.data.objectId,
              temperature: Number(fields.temperature) / 100,
              humidity: Number(fields.humidity) / 100,
              ec: Number(fields.ec),
              ph: Number(fields.ph) / 100,
              timestamp: Number(fields.timestamp),
              deviceId: fields.device_id,
              sensorType: fields.sensor_type,
              location: fields.location || "",
              transactionDigest: obj.data.previousTransaction as string,
              objectId: obj.data.objectId,
            } as SensorData;
          }
          return null;
        })
      );

      const validSensors = sensors
        .filter((sensor): sensor is SensorData => sensor !== null)
        .sort((a, b) => b.timestamp - a.timestamp);

      setUserData(validSensors);
      setUserStats(calculateStats(validSensors));
    } catch (err: any) {
      console.error("Failed to fetch user sensor data:", err);
      setUserError("Failed to load user-owned sensor data.");
    } finally {
      setUserLoading(false);
    }
  };

  // --- Fetching Logic for System-Owned Data (The Fix) ---
  const fetchSystemData = async () => {
    if (!PACKAGE_ID || !SYSTEM_OWNER_ADDRESS) {
        setSystemData([]);
        setSystemError("System Owner Address is not configured.");
        return;
    }

    setSystemLoading(true);
    setSystemError(null);

    try {
      // Query for SensorData objects OWNED BY THE BACKEND SENDER
      const ownedObjects = await client.getOwnedObjects({
        owner: SYSTEM_OWNER_ADDRESS, // <--- Backend/Gas SENDER Address
        filter: {
          StructType: `${PACKAGE_ID}::sensor_storage::SensorData`,
        },
        options: {
          showContent: true,
          showType: true,
        },
      });
      
      const sensors = await Promise.all(
        ownedObjects.data.map(async (obj) => {
          if (obj.data?.content) {
            const fields = (obj.data.content as any).fields;
            return {
              id: obj.data.objectId,
              temperature: Number(fields.temperature) / 100,
              humidity: Number(fields.humidity) / 100,
              ec: Number(fields.ec),
              ph: Number(fields.ph) / 100,
              timestamp: Number(fields.timestamp),
              deviceId: fields.device_id,
              sensorType: fields.sensor_type,
              location: fields.location || "",
              transactionDigest: obj.data.previousTransaction as string,
              objectId: obj.data.objectId,
            } as SensorData;
          }
          return null;
        })
      );

      const validSensors = sensors
        .filter((sensor): sensor is SensorData => sensor !== null)
        .sort((a, b) => b.timestamp - a.timestamp);

      setSystemData(validSensors);
      setSystemStats(calculateStats(validSensors));
    } catch (err: any) {
      console.error("Failed to fetch system sensor data:", err);
      setSystemError("Failed to load system-owned sensor data.");
    } finally {
      setSystemLoading(false);
    }
  };


  useEffect(() => {
    fetchUserData();
    fetchSystemData();
  }, [account, PACKAGE_ID]); // Keep 'account' to refresh user data when wallet changes

  // Helper formatting functions (kept as is)
  const formatTime = (timestamp: number) => new Date(timestamp).toLocaleString();
  const formatTemperature = (temp: number) => `${temp.toFixed(1)}°C`;
  const formatHumidity = (hum: number) => `${hum.toFixed(1)}%`;
  const formatPH = (ph: number) => ph.toFixed(2);
  const formatEC = (ec: number) => `${ec} µS/cm`;
  
  // Choose the data source for the main chart/stats display
  const primaryData = systemData.length > 0 ? systemData : userData;
  const primaryStats = systemData.length > 0 ? systemStats : userStats;
  const primaryOwnerName = systemData.length > 0 ? "Backend" : account ? "Connected User" : "Unknown";

  // Prepare chart data (using primary data source)
  const chartData = primaryData
    .slice() 
    .sort((a, b) => a.timestamp - b.timestamp)
    .slice(Math.max(primaryData.length - 10, 0))
    .map((data) => ({
      time: new Date(data.timestamp).toLocaleTimeString([], {
        hour: "2-digit",
        minute: "2-digit",
      }),
      temperature: data.temperature,
      humidity: data.humidity,
      ph: data.ph,
    }));
    
  const isDashboardLoading = userLoading || systemLoading;
  const totalReadings = userData.length + systemData.length;


  return (
    <div className="bg-gray-800 rounded-xl p-6 shadow-lg text-white">
      <div className="flex justify-between items-center mb-6 border-b border-gray-700 pb-4">
        <div>
          <h2 className="text-2xl font-bold">Sensor Data Dashboard</h2>
          <p className="text-sm text-gray-400">
            Total {totalReadings} readings found across all sources.
          </p>
        </div>
        <button
          onClick={() => { fetchUserData(); fetchSystemData(); }}
          disabled={isDashboardLoading || !PACKAGE_ID}
          className="px-4 py-2 bg-gray-700 hover:bg-gray-600 rounded-lg transition-colors disabled:opacity-50"
        >
          {isDashboardLoading ? "Refreshing..." : "Refresh All Data"}
        </button>
      </div>

      {/* General Error Handling */}
      {(!account && !SYSTEM_OWNER_ADDRESS) && (
        <div className="text-center py-8 text-gray-400">
          Connect your wallet AND configure the backend sender address to view data.
        </div>
      )}
      
      {/* Loading State */}
      {isDashboardLoading && (
        <div className="text-center py-8">
          <div className="inline-block animate-spin rounded-full h-8 w-8 border-b-2 border-white"></div>
          <p className="mt-2">Loading sensor data...</p>
        </div>
      )}

      {/* Main Dashboard Content */}
      {!isDashboardLoading && (
        <>
          {/* Statistics Overview (Uses the Data Source with Readings) */}
          {primaryStats && (
            <div className="mb-8">
                <h3 className="text-xl font-semibold mb-4 border-l-4 border-blue-500 pl-2">
                    System Overview (Readings from {primaryOwnerName})
                </h3>
                <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
                    <div className="bg-gray-700/50 rounded-lg p-4">
                        <div className="text-sm text-gray-400">Avg Temp</div>
                        <div className="text-2xl font-bold text-blue-300">
                            {formatTemperature(primaryStats.avgTemperature)}
                        </div>
                    </div>
                    <div className="bg-gray-700/50 rounded-lg p-4">
                        <div className="text-sm text-gray-400">Avg Humidity</div>
                        <div className="text-2xl font-bold text-green-300">
                            {formatHumidity(primaryStats.avgHumidity)}
                        </div>
                    </div>
                    <div className="bg-gray-700/50 rounded-lg p-4">
                        <div className="text-sm text-gray-400">Avg EC</div>
                        <div className="text-2xl font-bold text-purple-300">
                            {primaryStats.avgEC.toFixed(0)} µS/cm
                        </div>
                    </div>
                    <div className="bg-gray-700/50 rounded-lg p-4">
                        <div className="text-sm text-gray-400">Avg pH</div>
                        <div className="text-2xl font-bold text-yellow-300">
                            {formatPH(primaryStats.avgPH)}
                        </div>
                    </div>
                </div>
            </div>
          )}

          {/* Charts (Uses the Data Source with Readings) */}
          {primaryData.length > 1 && (
            <div className="mb-8">
              <h3 className="text-lg font-semibold mb-4">
                Recent Readings Trend ({primaryOwnerName} Data)
              </h3>
              <div className="h-64">
                <ResponsiveContainer width="100%" height="100%">
                  <LineChart data={chartData}>
                    <CartesianGrid strokeDasharray="3 3" stroke="#444" />
                    <XAxis dataKey="time" stroke="#888" />
                    <YAxis stroke="#888" />
                    <Tooltip
                      contentStyle={{
                        backgroundColor: "#1f2937",
                        borderColor: "#4b5563",
                      }}
                    />
                    <Legend />
                    <Line
                      type="monotone"
                      dataKey="temperature"
                      stroke="#3b82f6"
                      name="Temperature (°C)"
                      strokeWidth={2}
                    />
                    <Line
                      type="monotone"
                      dataKey="humidity"
                      stroke="#10b981"
                      name="Humidity (%)"
                      strokeWidth={2}
                    />
                  </LineChart>
                </ResponsiveContainer>
              </div>
            </div>
          )}

          {/* --- Section for Backend-Owned Data (The working data) --- */}
          <DataSection
            title={`✅ All System Readings (${systemData.length} items)`}
            data={systemData}
            ownerAddress={SYSTEM_OWNER_ADDRESS}
            error={systemError}
            formatters={{ formatTime, formatTemperature, formatHumidity, formatPH, formatEC }}
            isUserOwned={false}
          />
          <hr className="my-8 border-gray-700" />
          
          {/* --- Section for User-Owned Data (The original data) --- */}
          <DataSection
            title={`⚠️ User-Owned Readings (${userData.length} items)`}
            data={userData}
            ownerAddress={account?.address || "N/A"}
            error={userError}
            formatters={{ formatTime, formatTemperature, formatHumidity, formatPH, formatEC }}
            isUserOwned={true}
          />

        </>
      )}

      {/* Info Footer */}
      <div className="mt-6 pt-4 border-t border-gray-700">
        <div className="text-xs text-gray-500">
          Total {totalReadings} sensor reading{totalReadings !== 1 ? "s" : ""}.
          {primaryStats && ` • Displaying stats from ${primaryOwnerName} • Last updated: ${new Date().toLocaleTimeString()}`}
        </div>
      </div>
    </div>
  );
}


// --- New Reusable Component to Render the List ---
interface DataSectionProps {
    title: string;
    data: SensorData[];
    ownerAddress: string;
    error: string | null;
    formatters: {
        formatTime: (ts: number) => string;
        formatTemperature: (t: number) => string;
        formatHumidity: (h: number) => string;
        formatPH: (p: number) => string;
        formatEC: (e: number) => string;
    }
    isUserOwned: boolean;
}

const DataSection: React.FC<DataSectionProps> = ({ title, data, ownerAddress, error, formatters, isUserOwned }) => {
    const { formatTime, formatTemperature, formatHumidity, formatPH, formatEC } = formatters;
    
    return (
        <div className="mb-8">
            <h3 className="text-xl font-semibold mb-4 text-gray-200">
                {title} 
                <span className="text-sm font-normal text-gray-400 ml-2">
                    (Owner: {ownerAddress.slice(0, 6)}...{ownerAddress.slice(-4)})
                </span>
            </h3>

            {error && (
                <div className="mb-4 p-3 bg-red-500/20 border border-red-500 rounded-lg text-red-300">
                    {error}
                </div>
            )}

            {data.length === 0 && !error && (
                <div className={`p-4 rounded-lg text-center ${isUserOwned ? 'bg-yellow-900/20 text-yellow-300' : 'bg-gray-700/50 text-gray-400'}`}>
                    {isUserOwned 
                        ? "No sensor data owned by your wallet. (This is expected with the current API setup)."
                        : "No system sensor data found."}
                </div>
            )}

            {/* Sensor Data List */}
            {data.length > 0 && (
                <div className="space-y-4 max-h-[400px] overflow-y-auto pr-2">
                    {data.map((item) => (
                        <div
                            key={item.id}
                            className="bg-gray-700/50 hover:bg-gray-700/70 rounded-lg p-4 transition-all border border-gray-600/50"
                        >
                            <div className="flex justify-between items-start mb-3">
                                <div>
                                    <h3 className="font-semibold">{item.deviceId}</h3>
                                    <p className="text-sm text-gray-400">
                                        {formatTime(item.timestamp)}
                                    </p>
                                </div>
                                <div className="flex space-x-2">
                                    <span className="text-xs px-2 py-1 bg-blue-900/50 rounded">
                                        {item.sensorType}
                                    </span>
                                    <a
                                        href={`https://suiscan.xyz/testnet/object/${item.objectId}`}
                                        target="_blank"
                                        rel="noopener noreferrer"
                                        className="text-xs px-2 py-1 bg-purple-900/50 hover:bg-purple-800 rounded transition-colors"
                                    >
                                        View
                                    </a>
                                </div>
                            </div>

                            {/* Sensor Readings Grid */}
                            <div className="grid grid-cols-2 md:grid-cols-4 gap-3 mb-3">
                                <div className="text-center p-2 bg-blue-900/20 rounded">
                                    <div className="text-xs text-gray-400">Temp</div>
                                    <div className="text-lg font-bold text-blue-300">
                                        {formatTemperature(item.temperature)}
                                    </div>
                                </div>
                                <div className="text-center p-2 bg-green-900/20 rounded">
                                    <div className="text-xs text-gray-400">Humidity</div>
                                    <div className="text-lg font-bold text-green-300">
                                        {formatHumidity(item.humidity)}
                                    </div>
                                </div>
                                <div className="text-center p-2 bg-purple-900/20 rounded">
                                    <div className="text-xs text-gray-400">EC</div>
                                    <div className="text-lg font-bold text-purple-300">
                                        {formatEC(item.ec)}
                                    </div>
                                </div>
                                <div className="text-center p-2 bg-yellow-900/20 rounded">
                                    <div className="text-xs text-gray-400">pH</div>
                                    <div className="text-lg font-bold text-yellow-300">
                                        {formatPH(item.ph)}
                                    </div>
                                </div>
                            </div>

                            {/* Metadata */}
                            <div className="text-xs text-gray-400 flex justify-between">
                                <span>Location: {item.location || "Not specified"}</span>
                                <span>Device: {item.deviceId}</span>
                            </div>
                        </div>
                    ))}
                </div>
            )}
        </div>
    );
};