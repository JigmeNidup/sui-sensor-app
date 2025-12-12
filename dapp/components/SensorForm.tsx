"use client";

import { useState } from "react";
import {
  useCurrentAccount,
  useSignAndExecuteTransaction,
} from "@mysten/dapp-kit";
import { Transaction } from "@mysten/sui/transactions";
import { SensorFormData } from "@/lib/types";

interface SensorFormProps {
  onDataStored: () => void;
}

export default function SensorForm({ onDataStored }: SensorFormProps) {
  const account = useCurrentAccount();
  const { mutateAsync: signAndExecuteTransaction, isPending: isSubmitting } =
    useSignAndExecuteTransaction();

  const [formData, setFormData] = useState<SensorFormData>({
    temperature: "",
    humidity: "",
    ec: "",
    ph: "",
    deviceId: "sensor-001",
    sensorType: "soil",
    location: "",
  });

  const [error, setError] = useState<string | null>(null);
  const [txDigest, setTxDigest] = useState<string | null>(null);

  const PACKAGE_ID = process.env.NEXT_PUBLIC_SENSOR_PACKAGE_ID || "";
  const CLOCK_OBJECT_ID = process.env.NEXT_PUBLIC_SUI_CLOCK_OBJECT_ID || "0x6";

  const handleInputChange = (
    e: React.ChangeEvent<
      HTMLInputElement | HTMLSelectElement | HTMLTextAreaElement
    >
  ) => {
    const { name, value } = e.target;
    setFormData((prev) => ({ ...prev, [name]: value }));
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();

    if (!account) {
      setError("Please connect your wallet first");
      return;
    }

    if (!PACKAGE_ID) {
      setError("Sensor contract package ID not configured");
      return;
    }

    // Validate inputs
    const errors = [];
    const temp = parseFloat(formData.temperature);
    const hum = parseFloat(formData.humidity);
    const ec = parseInt(formData.ec);
    const ph = parseFloat(formData.ph);

    if (isNaN(temp) || temp < -50 || temp > 100)
      errors.push("Temperature must be between -50 and 100°C");
    if (isNaN(hum) || hum < 0 || hum > 100)
      errors.push("Humidity must be between 0 and 100%");
    if (isNaN(ec) || ec < 0 || ec > 5000)
      errors.push("EC must be between 0 and 5000 µS/cm");
    if (isNaN(ph) || ph < 0 || ph > 14)
      errors.push("pH must be between 0 and 14");

    if (errors.length > 0) {
      setError(errors.join(", "));
      return;
    }

    setError(null);
    setTxDigest(null);

    try {
      const tx = new Transaction();

      // Get clock object for timestamp
      const clock = tx.sharedObjectRef({
        objectId: CLOCK_OBJECT_ID,
        initialSharedVersion: 1,
        mutable: false,
      });

      // Convert values to Move-compatible format
      // Temperature and humidity stored as integers (25.5°C = 2550)
      const tempInt = Math.round(temp * 100);
      const humInt = Math.round(hum * 100);
      const phInt = Math.round(ph * 100);

      // Call the Move contract
      tx.moveCall({
        target: `${PACKAGE_ID}::sensor_storage::store_sensor_data`,
        arguments: [
          tx.pure.u64(tempInt), // temperature
          tx.pure.u64(humInt), // humidity
          tx.pure.u64(ec), // ec
          tx.pure.u64(phInt), // ph
          tx.pure.string(formData.deviceId), // device_id
          tx.pure.string(formData.sensorType), // sensor_type
          tx.pure.string(formData.location || ""), // location (can be empty)
          clock, // clock
        ],
      });

      // Set gas budget
      tx.setGasBudget(100000000);

      console.log(tx);
      

      // Execute transaction
      const result = await signAndExecuteTransaction({
        transaction: tx,
        // options: {
        //   showEffects: true,
        //   showEvents: true,
        //   showObjectChanges: true,
        // },
      });

      console.log("Sensor data stored successfully:", result);
      setTxDigest(result.digest);

      // Clear form and trigger refresh
      setFormData({
        temperature: "",
        humidity: "",
        ec: "",
        ph: "",
        deviceId: formData.deviceId,
        sensorType: formData.sensorType,
        location: formData.location,
      });
      onDataStored();
    } catch (err: any) {
      console.error("Failed to store sensor data:", err);
      setError(err.message || "Failed to store sensor data");
    }
  };

  return (
    <div className="bg-gray-800 rounded-xl p-6 shadow-lg">
      <h2 className="text-2xl font-bold mb-4">Store Sensor Data</h2>

      {error && (
        <div className="mb-4 p-3 bg-red-500/20 border border-red-500 rounded-lg text-red-300">
          <strong>Error:</strong> {error}
        </div>
      )}

      {txDigest && (
        <div className="mb-4 p-3 bg-green-500/20 border border-green-500 rounded-lg text-green-300">
          <strong>Success!</strong> Sensor data stored on-chain.
          <div className="mt-1 text-sm">
            TX:{" "}
            <span className="font-mono">
              {txDigest.slice(0, 10)}...{txDigest.slice(-8)}
            </span>
            <a
              href={`https://suiscan.xyz/testnet/tx/${txDigest}`}
              target="_blank"
              rel="noopener noreferrer"
              className="ml-2 text-blue-300 hover:text-blue-200 underline"
            >
              View on Explorer
            </a>
          </div>
        </div>
      )}

      <form onSubmit={handleSubmit} className="space-y-4">
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          {/* Temperature */}
          <div>
            <label
              htmlFor="temperature"
              className="block text-sm font-medium mb-2"
            >
              Temperature (°C)
            </label>
            <input
              type="number"
              id="temperature"
              name="temperature"
              value={formData.temperature}
              onChange={handleInputChange}
              step="0.1"
              min="-50"
              max="100"
              className="w-full px-4 py-2 bg-gray-700 border border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              placeholder="25.5"
              required
            />
          </div>

          {/* Humidity */}
          <div>
            <label
              htmlFor="humidity"
              className="block text-sm font-medium mb-2"
            >
              Humidity (%)
            </label>
            <input
              type="number"
              id="humidity"
              name="humidity"
              value={formData.humidity}
              onChange={handleInputChange}
              step="0.1"
              min="0"
              max="100"
              className="w-full px-4 py-2 bg-gray-700 border border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              placeholder="65.5"
              required
            />
          </div>

          {/* EC */}
          <div>
            <label htmlFor="ec" className="block text-sm font-medium mb-2">
              EC (µS/cm)
            </label>
            <input
              type="number"
              id="ec"
              name="ec"
              value={formData.ec}
              onChange={handleInputChange}
              min="0"
              max="5000"
              className="w-full px-4 py-2 bg-gray-700 border border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              placeholder="1200"
              required
            />
          </div>

          {/* pH */}
          <div>
            <label htmlFor="ph" className="block text-sm font-medium mb-2">
              pH Level
            </label>
            <input
              type="number"
              id="ph"
              name="ph"
              value={formData.ph}
              onChange={handleInputChange}
              step="0.01"
              min="0"
              max="14"
              className="w-full px-4 py-2 bg-gray-700 border border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              placeholder="7.25"
              required
            />
          </div>

          {/* Device ID */}
          <div>
            <label
              htmlFor="deviceId"
              className="block text-sm font-medium mb-2"
            >
              Device ID
            </label>
            <input
              type="text"
              id="deviceId"
              name="deviceId"
              value={formData.deviceId}
              onChange={handleInputChange}
              className="w-full px-4 py-2 bg-gray-700 border border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              placeholder="sensor-001"
              required
            />
          </div>

          {/* Sensor Type */}
          <div>
            <label
              htmlFor="sensorType"
              className="block text-sm font-medium mb-2"
            >
              Sensor Type
            </label>
            <select
              id="sensorType"
              name="sensorType"
              value={formData.sensorType}
              onChange={handleInputChange}
              className="w-full px-4 py-2 bg-gray-700 border border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
            >
              <option value="soil">Soil Sensor</option>
              <option value="air">Air Quality</option>
              <option value="water">Water Quality</option>
              <option value="weather">Weather Station</option>
              <option value="industrial">Industrial IoT</option>
            </select>
          </div>
        </div>

        {/* Location */}
        <div>
          <label htmlFor="location" className="block text-sm font-medium mb-2">
            Location (Optional)
          </label>
          <input
            type="text"
            id="location"
            name="location"
            value={formData.location}
            onChange={handleInputChange}
            className="w-full px-4 py-2 bg-gray-700 border border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
            placeholder="Field A, Greenhouse 1"
          />
        </div>

        <button
          type="submit"
          disabled={isSubmitting || !account || !PACKAGE_ID}
          className="w-full py-3 px-4 bg-linear-to-r from-green-500 to-blue-600 hover:from-green-600 hover:to-blue-700 disabled:from-gray-600 disabled:to-gray-700 disabled:cursor-not-allowed rounded-lg font-semibold transition-all duration-200 flex items-center justify-center"
        >
          {isSubmitting ? (
            <>
              <svg className="animate-spin h-5 w-5 mr-3" viewBox="0 0 24 24">
                <circle
                  className="opacity-25"
                  cx="12"
                  cy="12"
                  r="10"
                  stroke="currentColor"
                  strokeWidth="4"
                  fill="none"
                />
                <path
                  className="opacity-75"
                  fill="currentColor"
                  d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4z"
                />
              </svg>
              Storing Data...
            </>
          ) : (
            "Store Sensor Data on Sui"
          )}
        </button>

        {!account && (
          <p className="text-center text-sm text-yellow-300">
            🔗 Connect your wallet to store sensor data
          </p>
        )}

        {!PACKAGE_ID && (
          <p className="text-center text-sm text-red-300">
            ⚠️ Sensor contract not deployed. Run `sui client publish` in
            sensor_storage folder
          </p>
        )}
      </form>

      <div className="mt-6 pt-4 border-t border-gray-700">
        <h3 className="text-sm font-semibold text-gray-300 mb-2">
          Data Format:
        </h3>
        <div className="text-xs text-gray-400 space-y-1">
          <p>• Temperature: Stored as integer (25.5°C = 2550)</p>
          <p>• Humidity: Stored as integer (65.5% = 6550)</p>
          <p>• pH: Stored as integer (7.25 = 725)</p>
          <p>• EC: Stored as integer (1200 µS/cm = 1200)</p>
          <p>• Contract: {PACKAGE_ID || "Not deployed"}</p>
        </div>
      </div>
    </div>
  );
}
