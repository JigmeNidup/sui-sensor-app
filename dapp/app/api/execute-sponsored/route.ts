// dapp/app/api/execute-sponsored/route.ts
import { NextRequest, NextResponse } from "next/server";
import { SuiClient, getFullnodeUrl } from "@mysten/sui/client";
import { Transaction } from "@mysten/sui/transactions";
import { toBase64 } from "@mysten/sui/utils";
import { z } from "zod";

// Schema for request validation
const ExecuteSponsoredSchema = z.object({
  temperature: z.number().int().min(0).max(10000), // 0-100°C in hundredths
  humidity: z.number().int().min(0).max(10000), // 0-100% in hundredths
  ec: z.number().int().min(0).max(50000), // 0-5000 in tens
  ph: z.number().int().min(0).max(1400), // 0-14 in hundredths
  timestamp: z.number().int().positive(),
  signature: z.string().min(10),
});

// CORS headers
const corsHeaders = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "POST, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type, X-API-Key",
};

export async function POST(request: NextRequest) {
  try {
    // Parse request body
    const body = await request.json();

    // Validate request data
    const validatedData = ExecuteSponsoredSchema.parse(body);

    const { temperature, humidity, ec, ph, timestamp, signature } =
      validatedData;

    console.log("[Execute Sponsored] Received:", {
      temperature,
      humidity,
      ec,
      ph,
      timestamp,
      signature: signature,
    });

    // Get configuration from environment
    const network = process.env.SUI_NETWORK || "testnet";
    const packageId = process.env.NEXT_PUBLIC_SENSOR_PACKAGE_ID;
    const sensorObjectId = process.env.NEXT_PUBLIC_SENSOR_OBJECT_ID;
    const senderAddress = process.env.SUI_SENDER_ADDRESS;
    const clockObjectId = process.env.NEXT_PUBLIC_SUI_CLOCK_OBJECT_ID || "0x6";

    if (!packageId || !sensorObjectId || !senderAddress) {
      return NextResponse.json(
        {
          error: "Configuration missing",
          details:
            "Set NEXT_PUBLIC_SENSOR_PACKAGE_ID, NEXT_PUBLIC_SENSOR_OBJECT_ID, and SUI_SENDER_ADDRESS in environment",
        },
        { status: 500, headers: corsHeaders }
      );
    }

    // Initialize Sui client
    const client = new SuiClient({
      url: getFullnodeUrl(network as any),
    });

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

    // Use the first coin
    const gasCoin = coins.data[0];

    console.log("[Execute Sponsored] Building transaction with:", {
      packageId,
      gasObjectId: gasCoin.coinObjectId,
      gasVersion: gasCoin.version,
      clockObjectId,
    });

    // Build the transaction
    const tx = new Transaction();
    tx.setSender(senderAddress);

    // Set gas payment
    tx.setGasPayment([
      {
        objectId: gasCoin.coinObjectId,
        version: gasCoin.version,
        digest: gasCoin.digest,
      },
    ]);

    tx.setGasBudget(100000000);

    // Create the move call - matching your Move contract exactly
    tx.moveCall({
      target: `${packageId}::sensor_storage::store_sensor_data`,
      arguments: [
        tx.pure.u64(temperature), // temperature
        tx.pure.u64(humidity), // humidity
        tx.pure.u64(ec), // ec
        tx.pure.u64(ph), // ph
        tx.pure.string("esp32-device"), // device_id
        tx.pure.string("soil"), // sensor_type
        tx.pure.string(""), // location
        tx.sharedObjectRef({
          // clock object
          objectId: clockObjectId,
          initialSharedVersion: 1,
          mutable: false,
        }),
      ],
    });

    // Build transaction bytes
    const txBytes = await tx.build({
      client,
      onlyTransactionKind: false,
    });

    let txHex = Buffer.from(txBytes).toString("hex");
    console.log("build: ", txHex);

    const txBytesB64 = toBase64(txBytes);
    console.log(
      "[Execute Sponsored] Transaction built, length:",
      txBytes.length
    );

    // Execute the transaction with the ESP32's signature
    const result = await client.executeTransactionBlock({
      transactionBlock: txBytesB64,
      signature: signature,
      options: {
        showEffects: true,
        showEvents: true,
        showObjectChanges: true,
      },
    });

    console.log("[Execute Sponsored] Transaction executed:", {
      digest: result.digest,
      status: result.effects?.status?.status,
    });

    return NextResponse.json(
      {
        success: true,
        digest: result.digest,
        effects: result.effects,
        events: result.events,
        objectChanges: result.objectChanges,
        explorerUrl: `https://suiscan.xyz/${network}/tx/${result.digest}`,
        timestamp: Date.now(),
      },
      { status: 200, headers: corsHeaders }
    );
  } catch (error: any) {
    console.error("[Execute Sponsored] Error:", error);

    if (error instanceof z.ZodError) {
      return NextResponse.json(
        {
          error: "Invalid request data",
          details: error.message,
        },
        { status: 400, headers: corsHeaders }
      );
    }

    return NextResponse.json(
      {
        error: "Failed to execute sponsored transaction",
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
