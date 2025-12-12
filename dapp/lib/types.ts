export interface SensorData {
  id: string;
  temperature: number;  // Stored as integer (25.5°C = 2550)
  humidity: number;     // Stored as integer (65.5% = 6550)
  ec: number;          // Electrical Conductivity (µS/cm)
  ph: number;          // Stored as integer (7.25 = 725)
  timestamp: number;
  deviceId: string;
  sensorType: string;
  location: string;    // Now just a string (empty if not provided)
  transactionDigest: string;
  objectId: string;
}

export interface SensorFormData {
  temperature: string;
  humidity: string;
  ec: string;
  ph: string;
  deviceId: string;
  sensorType: string;
  location: string;
}

export interface SensorStats {
  avgTemperature: number;
  avgHumidity: number;
  avgEC: number;
  avgPH: number;
  totalReadings: number;
}