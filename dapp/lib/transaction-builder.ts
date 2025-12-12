import { Transaction } from "@mysten/sui/transactions";
import { bcs } from "@mysten/sui/bcs";
import { SuiClient } from "@mysten/sui/client";
import { toBase64 } from "@mysten/bcs";

export interface SensorTxData {
  temperature: number; // In hundredths (25.5°C = 2550)
  humidity: number; // In hundredths (65.5% = 6550)
  ec: number; // Raw µS/cm (1200)
  ph: number; // In hundredths (7.25 = 725)
  deviceId: string;
  sensorType: string;
  location: string;
}

/**
 * Builds transaction bytes for sensor data storage
 * Returns hex string of BCS-serialized transaction
 */
export async function buildSensorTransactionBytes(
  txData: SensorTxData
): Promise<string> {
  const PACKAGE_ID = process.env.NEXT_PUBLIC_SENSOR_PACKAGE_ID;
  const CLOCK_OBJECT_ID = process.env.NEXT_PUBLIC_SUI_CLOCK_OBJECT_ID || "0x6";
  const senderAddress = process.env.SUI_SENDER_ADDRESS;

  if (!PACKAGE_ID) {
    throw new Error("SENSOR_PACKAGE_ID is not configured");
  }

  // Create a Transaction object (similar to frontend)
  const tx = new Transaction();

  tx.setSender(senderAddress as string);

  // Get clock object reference
  const clock = tx.sharedObjectRef({
    objectId: CLOCK_OBJECT_ID,
    initialSharedVersion: 1,
    mutable: false,
  });

  // Add the move call (same as frontend)
  tx.moveCall({
    target: `${PACKAGE_ID}::sensor_storage::store_sensor_data`,
    arguments: [
      tx.pure.u64(txData.temperature),
      tx.pure.u64(txData.humidity),
      tx.pure.u64(txData.ec),
      tx.pure.u64(txData.ph),
      tx.pure.string(txData.deviceId),
      tx.pure.string(txData.sensorType),
      tx.pure.string(txData.location),
      clock,
    ],
  });

  // Set gas budget
  tx.setGasBudget(100000000);

  // Build the transaction to get bytes
  const txBytes = await tx.build({
    client: new SuiClient({ url: "https://fullnode.testnet.sui.io:443" }),
    onlyTransactionKind: false,
  });

  // Convert to hex string for ESP32
  let txHex = Buffer.from(txBytes).toString("hex");
  // const txHex = Buffer.from(txBytes).toString("base64");
  console.log("build: ",txHex);
  return txHex
}

/**
 * Parse signed transaction response from ESP32 and submit to Sui
 */
export async function submitSignedTransaction(
  signedTxHex: string,
  signatureHex: string
): Promise<any> {
  const { SuiClient, getFullnodeUrl } = await import("@mysten/sui/client");

  const client = new SuiClient({
    url: getFullnodeUrl(process.env.SUI_NETWORK || ("testnet" as any)),
  });

  // Convert hex strings back to bytes
  // const txBytes = Buffer.from(signedTxHex, "hex");
  const rawTxBytes = Buffer.from(signedTxHex, 'hex')
  const txBytesB64 = toBase64(rawTxBytes);

  console.log("submit: ",signedTxHex);
  // console.log(signatureHex);
  
  const signature =signatureHex; 
  // const signature = Buffer.from(signatureHex, "hex").toString("base64");

  // Execute the signed transaction
  const result = await client.executeTransactionBlock({
    transactionBlock:  txBytesB64,
    signature: signature,
    options: {
      showEffects: true,
      showEvents: true,
    },
  });

  return result;
}
