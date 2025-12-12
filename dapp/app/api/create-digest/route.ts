// dapp/app/api/create-digest/route.ts
import { NextRequest, NextResponse } from "next/server";
import { SuiClient, getFullnodeUrl } from "@mysten/sui/client";
import { z } from "zod";

// Schema for request validation
const CreateDigestSchema = z.object({
  senderAddress: z.string().optional(),
});

// CORS headers
const corsHeaders = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "GET, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type, X-API-Key",
};

export async function GET(request: NextRequest) {
  try {
    // Get query parameters
    const { searchParams } = new URL(request.url);
    const senderAddress = searchParams.get("senderAddress") || process.env.SUI_SENDER_ADDRESS;

    if (!senderAddress) {
      return NextResponse.json(
        {
          error: "Sender address is required",
          details: "Provide senderAddress query parameter or set SUI_SENDER_ADDRESS in environment",
        },
        { status: 400, headers: corsHeaders }
      );
    }

    // Initialize Sui client
    const network = process.env.SUI_NETWORK || "testnet";
    const client = new SuiClient({
      url: getFullnodeUrl(network as any),
    });

    // Get sensor object from environment
    const sensorObjectId = process.env.NEXT_PUBLIC_SENSOR_OBJECT_ID;
    if (!sensorObjectId) {
      return NextResponse.json(
        {
          error: "Sensor object ID not configured",
          details: "Set NEXT_PUBLIC_SENSOR_OBJECT_ID in environment",
        },
        { status: 500, headers: corsHeaders }
      );
    }

    console.log("[Create Digest] Fetching objects for:", {
      sensorObjectId,
      senderAddress,
      network,
    });

    // Get sensor object details
    let sensorObject;
    try {
      sensorObject = await client.getObject({
        id: sensorObjectId,
        options: {
          showType: true,
          showOwner: true,
          showPreviousTransaction: true,
          showStorageRebate: true,
          showContent: true,
        },
      });
    } catch (error) {
      console.error("[Create Digest] Failed to fetch sensor object:", error);
      return NextResponse.json(
        {
          error: "Sensor object not found",
          details: `No sensor found with ID: ${sensorObjectId}`,
        },
        { status: 404, headers: corsHeaders }
      );
    }

    if (!sensorObject.data) {
      return NextResponse.json(
        {
          error: "Sensor object has no data",
          details: `Sensor object ${sensorObjectId} exists but has no data`,
        },
        { status: 500, headers: corsHeaders }
      );
    }

    const sensorVersion = sensorObject.data.version;
    const sensorDigest = sensorObject.data.digest;

    // Get gas coins for the sender
    const coins = await client.getCoins({
      owner: senderAddress,
      coinType: "0x2::sui::SUI",
    });

    if (coins.data.length === 0) {
      return NextResponse.json(
        {
          error: "No gas coins available",
          details: `Sender ${senderAddress} has no SUI coins for gas`,
        },
        { status: 400, headers: corsHeaders }
      );
    }

    // Use the first coin with sufficient balance
    const gasCoin = coins.data[0];

    const response = {
      success: true,
      sensorObjectId,
      sensorVersion,
      gasObjectId: gasCoin.coinObjectId,
      gasVersion: gasCoin.version,
      gasDigest: gasCoin.digest,
      timestamp: Date.now(),
    };

    console.log("[Create Digest] Success:", {
      sensorObjectId,
      sensorVersion,
      gasObjectId: gasCoin.coinObjectId,
      gasVersion: gasCoin.version,
    });

    return NextResponse.json(response, {
      status: 200,
      headers: corsHeaders,
    });
  } catch (error: any) {
    console.error("[Create Digest] Error:", error);

    return NextResponse.json(
      {
        error: "Failed to create digest",
        details: error.message || "Unknown error",
      },
      { status: 500, headers: corsHeaders }
    );
  }
}

export async function OPTIONS() {
  return NextResponse.json(null, {
    status: 204,
    headers: corsHeaders,
  });
}