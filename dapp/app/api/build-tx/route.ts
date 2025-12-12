import { NextRequest, NextResponse } from "next/server";
import { buildSensorTransactionBytes } from "@/lib/transaction-builder";

// Simple rate limiting
const requestCache = new Map<string, { count: number; timestamp: number }>();
const RATE_LIMIT = 10; // requests per minute
const WINDOW_MS = 60000;

function checkRateLimit(ip: string): boolean {
  const now = Date.now();
  const record = requestCache.get(ip);

  if (!record) {
    requestCache.set(ip, { count: 1, timestamp: now });
    return false;
  }

  if (now - record.timestamp > WINDOW_MS) {
    // Window expired, reset
    requestCache.set(ip, { count: 1, timestamp: now });
    return false;
  }

  if (record.count >= RATE_LIMIT) {
    return true;
  }

  record.count++;
  return false;
}

export async function POST(request: NextRequest) {
  try {
    // Rate limiting
    const ip =request.headers.get("x-forwarded-for") || "unknown";
    if (checkRateLimit(ip)) {
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

    // Validate data ranges
    const errors: string[] = [];
    const tempInt = parseInt(body.temperature, 10);
    const humInt = parseInt(body.humidity, 10);
    const ecInt = parseInt(body.ec, 10);
    const phInt = parseInt(body.ph, 10);

    if (tempInt < 0 || tempInt > 10000) errors.push("Temperature out of range");
    if (humInt < 0 || humInt > 10000) errors.push("Humidity out of range");
    if (ecInt < 0 || ecInt > 50000) errors.push("EC out of range");
    if (phInt < 0 || phInt > 1400) errors.push("pH out of range");

    if (errors.length > 0) {
      return NextResponse.json(
        {
          success: false,
          error: errors.join(", "),
        },
        { status: 400 }
      );
    }

    // Build transaction bytes
    const txData = {
      temperature: tempInt,
      humidity: humInt,
      ec: ecInt,
      ph: phInt,
      deviceId: body.deviceId,
      sensorType: body.sensorType,
      location: body.location || "",
    };
    const txBytesHex = await buildSensorTransactionBytes(txData);

    return NextResponse.json({
      success: true,
      txBytes: txBytesHex,
      data: txData,
      timestamp: new Date().toISOString(),
      instructions:
        "Sign these bytes with your private key and POST to /api/submit-tx",
    });
  } catch (error: any) {
    console.error("Build TX Error:", error);

    return NextResponse.json(
      {
        success: false,
        error: error.message || "Failed to build transaction",
      },
      { status: 500 }
    );
  }
}

export async function GET() {
  return NextResponse.json({
    status: "active",
    service: "Sui Transaction Builder API",
    endpoint: "POST /api/build-tx with sensor data",
    network: process.env.SUI_NETWORK || "testnet",
    timestamp: new Date().toISOString(),
  });
}
