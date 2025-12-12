import { NextRequest, NextResponse } from "next/server";
import { sendSensorDataToSui } from "@/lib/sui-transaction";
import { SensorData } from "@/lib/types";

// Rate limiting - adjust as needed
const RATE_LIMIT_WINDOW = 60000; // 1 minute
const MAX_REQUESTS = 10; // Max 10 requests per minute per IP
const requestCounts = new Map<string, { count: number; resetTime: number }>();

function isRateLimited(ip: string): boolean {
  const now = Date.now();
  const record = requestCounts.get(ip);

  if (!record || record.resetTime < now) {
    // New window or window expired
    requestCounts.set(ip, { count: 1, resetTime: now + RATE_LIMIT_WINDOW });
    return false;
  }

  if (record.count >= MAX_REQUESTS) {
    return true;
  }

  record.count++;
  return false;
}

export async function POST(request: NextRequest) {
  try {
    // Get client IP for rate limiting
    const ip = request.headers.get("x-forwarded-for") || "unknown";

    // Check rate limit
    if (isRateLimited(ip)) {
      return NextResponse.json(
        {
          success: false,
          error: "Rate limit exceeded. Please try again later.",
        },
        { status: 429 }
      );
    }

    // Parse request body
    const body = await request.json();

    // Validate required fields
    const requiredFields = [
      "temperature",
      "humidity",
      "ec",
      "ph",
      "deviceId",
      "sensorType",
    ];
    for (const field of requiredFields) {
      if (!(field in body)) {
        return NextResponse.json(
          {
            success: false,
            error: `Missing required field: ${field}`,
          },
          { status: 400 }
        );
      }
    }

    // Type validation
    const sensorData: SensorData = {
      id: "", // Will be assigned by Sui
      temperature: parseFloat(body.temperature),
      humidity: parseFloat(body.humidity),
      ec: parseInt(body.ec, 10),
      ph: parseFloat(body.ph),
      timestamp: Date.now(),
      deviceId: body.deviceId,
      sensorType: body.sensorType,
      location: body.location || "",
      transactionDigest: "",
      objectId: "",
    };

    // Validate ranges (matching Move contract validation)
    const errors: string[] = [];

    if (sensorData.temperature < -50 || sensorData.temperature > 100) {
      errors.push("Temperature must be between -50 and 100°C");
    }

    if (sensorData.humidity < 0 || sensorData.humidity > 100) {
      errors.push("Humidity must be between 0 and 100%");
    }

    if (sensorData.ec < 0 || sensorData.ec > 5000) {
      errors.push("EC must be between 0 and 5000 µS/cm");
    }

    if (sensorData.ph < 0 || sensorData.ph > 14) {
      errors.push("pH must be between 0 and 14");
    }

    if (errors.length > 0) {
      return NextResponse.json(
        {
          success: false,
          error: errors.join(", "),
        },
        { status: 400 }
      );
    }

    // Send to Sui
    const result = await sendSensorDataToSui(sensorData);

    if (!result.success) {
      return NextResponse.json(
        {
          success: false,
          error: result.error || "Failed to store sensor data on Sui",
        },
        { status: 500 }
      );
    }

    // Return success response
    return NextResponse.json({
      success: true,
      message: "Sensor data stored successfully on Sui blockchain",
      transactionDigest: result.digest,
      explorerUrl: result.explorerUrl,
      timestamp: new Date().toISOString(),
    });
  } catch (error: any) {
    console.error("API Error:", error);

    return NextResponse.json(
      {
        success: false,
        error: error.message || "Internal server error",
      },
      { status: 500 }
    );
  }
}

// Optional: Add GET method to check API status
export async function GET() {
  return NextResponse.json({
    status: "active",
    service: "Sui Sensor Data API",
    network: process.env.SUI_NETWORK || "testnet",
    timestamp: new Date().toISOString(),
  });
}
