"use client";

import { useEffect, useState } from "react";
import { useCurrentAccount, useSuiClient } from "@mysten/dapp-kit";
import { SensorData, SensorStats } from "@/lib/types";
import {
  LineChart,
  Line,
  BarChart,
  Bar,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
} from "recharts";

// 1. Define the address that owns the SensorData objects.
// This MUST match the SUI_SENDER_ADDRESS in your API route's environment variables.
// NOTE: For a real app, this value should be loaded securely (e.g., from a shared config file or a public environment variable).
const SUI_SENDER_ADDRESS = process.env.NEXT_PUBLIC_SUI_SENDER_ADDRESS_FOR_QUERY || "YOUR_BACKEND_SENDER_ADDRESS";

// You can add a placeholder for the backend sender address in .env.local
// Example: NEXT_PUBLIC_SUI_SENDER_ADDRESS_FOR_QUERY="0xabc123..."

export default function SensorList() {
  const account = useCurrentAccount();
  const client = useSuiClient();
  const [sensorData, setSensorData] = useState<SensorData[]>([]);
  const [stats, setStats] = useState<SensorStats | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const PACKAGE_ID = process.env.NEXT_PUBLIC_SENSOR_PACKAGE_ID || "";

  const fetchSensorData = async () => {
    // We only need the PACKAGE_ID to fetch data, as the OWNER is hardcoded below.
    if (!PACKAGE_ID) {
      setSensorData([]);
      return;
    }

    // Use the SUI_SENDER_ADDRESS defined above as the owner for the query.
    // If the address is not configured, show an error/warning instead of fetching.
    const ownerAddress = SUI_SENDER_ADDRESS;
    if (!ownerAddress || ownerAddress === "YOUR_BACKEND_SENDER_ADDRESS") {
        setError("Configuration Error: Backend sender address (owner) is missing or a placeholder.");
        setIsLoading(false);
        setSensorData([]);
        return;
    }

    setIsLoading(true);
    setError(null);

    try {
      // 2. *** CRITICAL CHANGE HERE ***
      // Query for SensorData objects owned by the backend's address (SUI_SENDER_ADDRESS),
      // because that is where the 'transfer::public_transfer' sends the object.
      const ownedObjects = await client.getOwnedObjects({
        owner: ownerAddress, // <-- USE THE BACKEND SENDER ADDRESS
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
          if (obj.data?.objectId && obj.data?.content) {
            const content = obj.data.content as any;
            const fields = content.fields;

            // Parse sensor data from Move object
            const sensor: SensorData = {
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
            };

            return sensor;
          }
          return null;
        })
      );

      // Filter out null values and sort by timestamp
      const validSensors = sensors
        .filter((sensor): sensor is SensorData => sensor !== null)
        .sort((a, b) => b.timestamp - a.timestamp);

      setSensorData(validSensors);
      calculateStats(validSensors);
    } catch (err: any) {
      console.error("Failed to fetch sensor data:", err);
      setError("Failed to load sensor data. Check if 'SUI_SENDER_ADDRESS' is correct.");
    } finally {
      setIsLoading(false);
    }
  };

  const calculateStats = (data: SensorData[]) => {
    if (data.length === 0) {
      setStats(null);
      return;
    }

    const stats: SensorStats = {
      avgTemperature:
        data.reduce((sum, d) => sum + d.temperature, 0) / data.length,
      avgHumidity: data.reduce((sum, d) => sum + d.humidity, 0) / data.length,
      avgEC: data.reduce((sum, d) => sum + d.ec, 0) / data.length,
      avgPH: data.reduce((sum, d) => sum + d.ph, 0) / data.length,
      totalReadings: data.length,
    };

    setStats(stats);
  };

  useEffect(() => {
    // We only need to check if PACKAGE_ID exists before fetching data,
    // the user account is no longer the primary determinant of *what* to display.
    if (PACKAGE_ID) {
      fetchSensorData();
    }
  }, [PACKAGE_ID]); // Removed 'account' from dependencies as it's no longer the owner.

  const formatTime = (timestamp: number) => {
    return new Date(timestamp).toLocaleString();
  };

  const formatTemperature = (temp: number) => {
    return `${temp.toFixed(1)}°C`;
  };

  const formatHumidity = (hum: number) => {
    return `${hum.toFixed(1)}%`;
  };

  const formatPH = (ph: number) => {
    return ph.toFixed(2);
  };

  const formatEC = (ec: number) => {
    return `${ec} µS/cm`;
  };

  // Prepare chart data
  const chartData = sensorData
    .slice() // Create a copy
    .sort((a, b) => a.timestamp - b.timestamp) // Sort ascending
    .slice(Math.max(sensorData.length - 10, 0)) // Get last 10
    .map((data) => ({
      time: new Date(data.timestamp).toLocaleTimeString([], {
        hour: "2-digit",
        minute: "2-digit",
      }),
      temperature: data.temperature,
      humidity: data.humidity,
      ph: data.ph,
    }));

  return (
    <div className="bg-gray-800 rounded-xl p-6 shadow-lg">
      <div className="flex justify-between items-center mb-6">
        <div>
          <h2 className="text-2xl font-bold">Sensor Data Dashboard</h2>
          <p className="text-sm text-gray-400">
            {sensorData.length} readings from{" "}
            {SUI_SENDER_ADDRESS.slice(0, 8)}... (Backend Owner)
          </p>
        </div>
        <button
          onClick={fetchSensorData}
          disabled={isLoading || !PACKAGE_ID || SUI_SENDER_ADDRESS === "YOUR_BACKEND_SENDER_ADDRESS"}
          className="px-4 py-2 bg-gray-700 hover:bg-gray-600 rounded-lg transition-colors disabled:opacity-50"
        >
          {isLoading ? "Refreshing..." : "Refresh Data"}
        </button>
      </div>

      {error && (
        <div className="mb-4 p-3 bg-red-500/20 border border-red-500 rounded-lg text-red-300">
          {error}
        </div>
      )}

      {/* Simplified check, as the connected account no longer determines data visibility */}
      {!PACKAGE_ID ? (
        <div className="text-center py-8 text-gray-400">
          Sensor contract not deployed
        </div>
      ) : isLoading ? (
        <div className="text-center py-8">
          <div className="inline-block animate-spin rounded-full h-8 w-8 border-b-2 border-white"></div>
          <p className="mt-2">Loading sensor data...</p>
        </div>
      ) : sensorData.length === 0 && !error ? (
        <div className="text-center py-8 text-gray-400">
          No sensor data found at the backend owner address.
        </div>
      ) : (
        <>
          {/* Statistics Overview */}
          {stats && (
            <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-8">
              <div className="bg-gray-700/50 rounded-lg p-4">
                <div className="text-sm text-gray-400">Avg Temp</div>
                <div className="text-2xl font-bold text-blue-300">
                  {formatTemperature(stats.avgTemperature)}
                </div>
              </div>
              <div className="bg-gray-700/50 rounded-lg p-4">
                <div className="text-sm text-gray-400">Avg Humidity</div>
                <div className="text-2xl font-bold text-green-300">
                  {formatHumidity(stats.avgHumidity)}
                </div>
              </div>
              <div className="bg-gray-700/50 rounded-lg p-4">
                <div className="text-sm text-gray-400">Avg EC</div>
                <div className="text-2xl font-bold text-purple-300">
                  {stats.avgEC.toFixed(0)} µS/cm
                </div>
              </div>
              <div className="bg-gray-700/50 rounded-lg p-4">
                <div className="text-sm text-gray-400">Avg pH</div>
                <div className="text-2xl font-bold text-yellow-300">
                  {formatPH(stats.avgPH)}
                </div>
              </div>
            </div>
          )}

          {/* Charts */}
          {sensorData.length > 1 && (
            <div className="mb-8">
              <h3 className="text-lg font-semibold mb-4">
                Recent Readings Trend
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

          {/* Sensor Data List */}
          <div className="space-y-4 max-h-[500px] overflow-y-auto pr-2">
            {sensorData.map((data) => (
              <div
                key={data.id}
                className="bg-gray-700/50 hover:bg-gray-700/70 rounded-lg p-4 transition-all border border-gray-600/50"
              >
                <div className="flex justify-between items-start mb-3">
                  <div>
                    <h3 className="font-semibold">{data.deviceId}</h3>
                    <p className="text-sm text-gray-400">
                      {formatTime(data.timestamp)}
                    </p>
                  </div>
                  <div className="flex space-x-2">
                    <span className="text-xs px-2 py-1 bg-blue-900/50 rounded">
                      {data.sensorType}
                    </span>
                    <a
                      href={`https://suiscan.xyz/testnet/object/${data.objectId}`}
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
                    <div className="text-xs text-gray-400">Temperature</div>
                    <div className="text-lg font-bold text-blue-300">
                      {formatTemperature(data.temperature)}
                    </div>
                  </div>
                  <div className="text-center p-2 bg-green-900/20 rounded">
                    <div className="text-xs text-gray-400">Humidity</div>
                    <div className="text-lg font-bold text-green-300">
                      {formatHumidity(data.humidity)}
                    </div>
                  </div>
                  <div className="text-center p-2 bg-purple-900/20 rounded">
                    <div className="text-xs text-gray-400">EC</div>
                    <div className="text-lg font-bold text-purple-300">
                      {formatEC(data.ec)}
                    </div>
                  </div>
                  <div className="text-center p-2 bg-yellow-900/20 rounded">
                    <div className="text-xs text-gray-400">pH</div>
                    <div className="text-lg font-bold text-yellow-300">
                      {formatPH(data.ph)}
                    </div>
                  </div>
                </div>

                {/* Metadata */}
                <div className="text-xs text-gray-400 flex justify-between">
                  <span>Location: {data.location || "Not specified"}</span>
                  <span>Device: {data.deviceId}</span>
                </div>
              </div>
            ))}
          </div>
        </>
      )}

      {/* Info Footer */}
      <div className="mt-6 pt-4 border-t border-gray-700">
        <div className="text-xs text-gray-500">
          Showing {sensorData.length} sensor reading
          {sensorData.length !== 1 ? "s" : ""}
          {stats && ` • Last updated: ${new Date().toLocaleTimeString()}`}
        </div>
      </div>
    </div>
  );
}