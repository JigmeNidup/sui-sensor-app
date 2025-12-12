import { SuiClient, getFullnodeUrl } from '@mysten/sui/client';
import { Transaction } from '@mysten/sui/transactions';
import { Ed25519Keypair } from '@mysten/sui/keypairs/ed25519';
import { fromBase64 } from '@mysten/bcs';
import { SensorData } from './types';
import { decodeSuiPrivateKey } from '@mysten/sui/cryptography';

// Initialize Sui client
const network = process.env.SUI_NETWORK || 'testnet';
export const client = new SuiClient({
  url: getFullnodeUrl(network as any)
});

// Create keypair from environment variable
const PRIVATE_KEY = process.env.SUI_SENDER_PRIVATE_KEY;
if (!PRIVATE_KEY) {
  throw new Error('SUI_SENDER_PRIVATE_KEY is not set in environment variables');
}



// Use the helper function `decodeSuiPrivateKey` if you have a Bech32 string.
// If your env variable is a base64 string, use `fromB64`.
// If it's a hex string, you can keep your original logic.
let secretKey: Uint8Array;
if (PRIVATE_KEY.startsWith('suiprivkey')) {
    // For Bech32-encoded keys (from 'sui keytool export')
    const { secretKey: decoded } = decodeSuiPrivateKey(PRIVATE_KEY);
    secretKey = decoded;
} else if (PRIVATE_KEY.length === 64 || (PRIVATE_KEY.startsWith('0x') && PRIVATE_KEY.length === 66)) {
    // For hex strings (64 chars, optionally prefixed with 0x)
    const hex = PRIVATE_KEY.startsWith('0x') ? PRIVATE_KEY.slice(2) : PRIVATE_KEY;
    secretKey = Uint8Array.from(Buffer.from(hex, 'hex'));
} else {
    // Try decoding as base64
    secretKey = fromBase64(PRIVATE_KEY);
}

const keypair = Ed25519Keypair.fromSecretKey(secretKey);


export interface SuiTransactionResult {
  success: boolean;
  digest?: string;
  error?: string;
  explorerUrl?: string;
}

/**
 * Send sensor data to Sui blockchain
 */
export async function sendSensorDataToSui(sensorData: SensorData): Promise<SuiTransactionResult> {
  try {
    const {
      temperature,
      humidity,
      ec,
      ph,
      deviceId,
      sensorType,
      location = ''
    } = sensorData;

    const PACKAGE_ID = process.env.NEXT_PUBLIC_SENSOR_PACKAGE_ID;
    const CLOCK_OBJECT_ID = process.env.NEXT_PUBLIC_SUI_CLOCK_OBJECT_ID || '0x6';

    if (!PACKAGE_ID) {
      throw new Error('SENSOR_PACKAGE_ID is not configured');
    }

    console.log('Building transaction for:', {
      deviceId,
      sensorType,
      temperature,
      humidity,
      ec,
      ph
    });

    // Create transaction
    const tx = new Transaction();

    // Add clock object reference
    const clock = tx.sharedObjectRef({
      objectId: CLOCK_OBJECT_ID,
      initialSharedVersion: 1,
      mutable: false,
    });

    // Call the Move function
    tx.moveCall({
      target: `${PACKAGE_ID}::sensor_storage::store_sensor_data`,
      arguments: [
        tx.pure.u64(Math.floor(temperature * 100)),  // Convert to Move format
        tx.pure.u64(Math.floor(humidity * 100)),     // Convert to Move format
        tx.pure.u64(Math.floor(ec)),
        tx.pure.u64(Math.floor(ph * 100)),           // Convert to Move format
        tx.pure.string(deviceId),
        tx.pure.string(sensorType),
        tx.pure.string(location),
        clock,
      ],
    });

    // Set gas budget
    tx.setGasBudget(100000000);

    console.log(tx);
    console.log(keypair);
    

    // Sign and execute transaction
    const result = await client.signAndExecuteTransaction({
      transaction: tx,
      signer: keypair,
      options: {
        showEffects: true,
        showEvents: true,
      },
    });

    console.log('Transaction successful:', result.digest);

    return {
      success: true,
      digest: result.digest,
      explorerUrl: `https://suiscan.xyz/${network}/tx/${result.digest}`
    };

  } catch (error: any) {
    console.error('Error sending to Sui:', error);
    return {
      success: false,
      error: error.message || 'Unknown error'
    };
  }
}