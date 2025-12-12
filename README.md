# 🌱 Sui Sensor Storage DApp

[![Sui Network](https://img.shields.io/badge/Sui-Network-4DA2FF?style=flat&logo=sui&logoColor=white)](https://sui.io)
[![Next.js](https://img.shields.io/badge/Next.js-14-black?style=flat&logo=next.js&logoColor=white)](https://nextjs.org)
[![Move](https://img.shields.io/badge/Move-Smart_Contract-FF5733?style=flat)](https://github.com/MystenLabs/sui)
[![ESP32](https://img.shields.io/badge/ESP32-IoT_Ready-00979D?style=flat&logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

A decentralized application (DApp) for storing and visualizing IoT sensor data on the Sui blockchain. This project combines a Next.js frontend with a Move smart contract and **supports three ESP32 integration approaches**: on-device transaction building (advanced), offline signing (production), and server-side signing (development).

> **🔐 Security First**: This project demonstrates production-grade blockchain integration for IoT devices with three levels of control - from building transactions on-device using BCS encoding to simple server-side signing for rapid development.

## 📋 Overview

This project enables IoT devices to store sensor data directly on the Sui blockchain, providing:
- **Immutable Data Storage**: Sensor readings stored permanently on-chain
- **Real-time Dashboard**: Interactive visualization of sensor data
- **Decentralized Architecture**: No central database required
- **Data Integrity**: Blockchain-backed verification of all readings
- **Multi-sensor Support**: Temperature, Humidity, EC, and pH measurements

## 🚀 Quick Start

Choose your integration approach:

### 🔬 Option A: ESP32 Builds Transaction (Advanced/Research)
**Best for:** Research, education, maximum control
```
ESP32 (Get Gas Info) → ESP32 (Build TX with BCS) → ESP32 (Sign) 
→ Server (Execute) → Sui Blockchain ✓
```
- ✅ Complete transaction transparency
- ✅ Learn Sui's internal structure
- ✅ Maximum control over transaction

### 🔐 Option B: Server Builds, ESP32 Signs (Production)
**Best for:** Production deployments, high-security applications
```
ESP32 (Collect Data) → Server (Build TX) → ESP32 (Sign with MicroSui) 
→ Server (Submit) → Sui Blockchain ✓
```
- ✅ Private keys stay on device
- ✅ Each device has unique blockchain identity
- ✅ Balanced security and simplicity

### ⚡ Option C: Server-Side Signing (Development)
**Best for:** Prototyping, testing, simple deployments
```
ESP32 (Collect Data) → Server (Sign & Submit) → Sui Blockchain ✓
```
- ✅ Simpler ESP32 code
- ✅ Lower memory requirements
- ✅ Faster implementation

[See detailed comparison →](#comparison-all-three-signing-approaches)

## ✨ Features

### Smart Contract (Move)
- ✅ Store sensor readings with validation
- ✅ Support for multiple sensor types
- ✅ Event emission for data tracking
- ✅ Built-in data range validation
- ✅ Device ID and location tracking

### DApp Frontend (Next.js)
- 🔐 Wallet integration (Sui Wallet, Ethos, etc.)
- 📊 Real-time data visualization with charts
- 📱 Responsive design for mobile and desktop
- 🔄 Auto-refresh dashboard (30-second intervals)
- 📝 Form-based sensor data submission
- 🔍 Transaction history and explorer links
- ⚡ Gas estimation before transactions
- 🛡️ Input validation and error handling
- 🚦 Rate limiting for API endpoints

### API Endpoints
- **GET /api/create-digest**: Get gas coin info and object digests (for Approach 1)
- **POST /api/execute-sponsored**: Execute pre-signed transaction (for Approach 1)
- **POST /api/build-tx**: Build unsigned transaction bytes (for Approach 2)
- **POST /api/submit-tx**: Submit signed transaction (for Approach 2)
- **POST /api/sensor-data**: Submit sensor data (Approach 3 - server-side signing)
- **GET /api/recent-data**: Fetch recent sensor readings
- **GET /api/build-tx**: Health check endpoint

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    IoT Sensor Device                     │
│               (Temperature, Humidity, EC, pH)            │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│                  Next.js DApp (Frontend)                 │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ Data Form    │  │  Dashboard   │  │ API Routes   │  │
│  │              │  │  (Recharts)  │  │              │  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│                  Sui Blockchain Network                  │
│  ┌──────────────────────────────────────────────────┐   │
│  │      Move Smart Contract (sensor_storage)        │   │
│  │   - Data Validation                              │   │
│  │   - Event Emission                               │   │
│  │   - Object Storage                               │   │
│  └──────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## 🔐 ESP32 Integration & Signing Mechanisms

This project supports **three different approaches** for ESP32 IoT devices to submit sensor data to the blockchain, each offering different levels of security, control, and complexity:

### Approach 1: ESP32 Builds Transaction (Maximum Control)

**Flow:**
```
ESP32 (Build TX with sui_transaction.cpp) → Server (Get Gas Info) → ESP32 (Sign) 
→ Server (Execute) → Sui Blockchain
```

**Architecture:**
```
┌─────────────────────────────────────────────────────────┐
│       ESP32 with sui_transaction.cpp Library             │
│  ┌───────────────────────────────────────────────────┐  │
│  │  1. Collect sensor data (temp, humidity, EC, pH)  │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  2. GET gas info from /api/create-digest         │  │
│  │     Request: ?senderAddress=<ESP32_address>      │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                      Server (Next.js)                    │
│  ┌───────────────────────────────────────────────────┐  │
│  │  3. Return gas coin info + object digests        │  │
│  │     - Gas Object ID, Version, Digest (Base58)    │  │
│  │     - No transaction building on server          │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│       ESP32 with sui_transaction.cpp Library             │
│  ┌───────────────────────────────────────────────────┐  │
│  │  4. Build transaction using BCS encoding          │  │
│  │     - Parse gas object info                       │  │
│  │     - Build TransactionData with sensor values    │  │
│  │     - Create ProgrammableTransaction structure    │  │
│  │     - Serialize using BCS (Binary Canonical)      │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  5. Sign transaction using MicroSui               │  │
│  │     - Hash transaction bytes (Blake2b)            │  │
│  │     - Sign with Ed25519 private key               │  │
│  │     - Generate signature (Base64)                 │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  6. POST to /api/execute-sponsored                │  │
│  │     Body: { signature, sensor data }              │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                      Server (Next.js)                    │
│  ┌───────────────────────────────────────────────────┐  │
│  │  7. Rebuild same transaction for verification     │  │
│  │     - Must match ESP32's transaction exactly      │  │
│  │     - Server acts as gas sponsor                  │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  8. Execute signed transaction on Sui             │  │
│  │     - Combine transaction + ESP32 signature       │  │
│  │     - Submit to Sui network                       │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                  Sui Blockchain Network                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  9. Store sensor data on-chain                    │  │
│  │     - Validate signature matches sender           │  │
│  │     - Execute smart contract                      │  │
│  │     - Emit SensorDataStoredEvent                  │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

**Benefits:**
- ✅ Maximum control - ESP32 builds entire transaction
- ✅ Server never sees transaction structure (only gas info)
- ✅ Complete transaction transparency
- ✅ Advanced: Custom BCS encoding on embedded device
- ✅ Educational: Learn Sui's transaction structure
- ✅ Private key never leaves device
- ✅ Server acts as pure gas sponsor

**Use Cases:**
- Advanced IoT deployments
- Research and educational projects
- Custom transaction requirements
- Maximum security with full auditability
- Learning Sui's internals

**ESP32 Code Example (Build on Device):**
```cpp
#include "sui_transaction.h"
#include <MicroSui.h>

void submitWithLocalBuild() {
  // Step 1: Get gas digest info from server
  HTTPClient http;
  String url = serverUrl + "/api/create-digest?senderAddress=" + myAddress;
  http.begin(url);
  int code = http.GET();
  
  if (code == 200) {
    String response = http.getString();
    // Parse: gasObjectId, gasVersion, gasDigest (Base58)
    DigestResponse digestInfo = parseDigestResponse(response);
    
    // Step 2: Build transaction locally using BCS encoding
    transaction_builder_t params = {0};
    
    // Set package ID, module, function
    bcs_hex_to_bytes(PACKAGE_ID, params.package_id, 32, &bytes_read);
    params.module_name = "sensor_storage";
    params.function_name = "store_sensor_data";
    
    // Set sensor data
    params.sensor_data.value1 = temperature;  // in hundredths
    params.sensor_data.value2 = humidity;     // in hundredths
    params.sensor_data.value3 = ec;
    params.sensor_data.value4 = ph;
    params.sensor_data.timestamp = getCurrentTimestamp();
    
    // Set gas object from server response
    bcs_hex_to_bytes(digestInfo.gasObjectId, params.gas_object.object_id, 32, &bytes_read);
    params.gas_object.version = digestInfo.gasVersion;
    base58_to_bytes(digestInfo.gasDigest, params.gas_object.digest, 32);
    
    params.gas_budget = 100000000;
    params.gas_price = 1000;
    
    // Build transaction (BCS serialization)
    char* txHex = nullptr;
    size_t txHexLen = 0;
    bcs_error_t err = sui_build_sensor_transaction(&params, &txHex, &txHexLen);
    
    if (err == BCS_OK) {
      // Step 3: Sign with MicroSui
      MicroSuiEd25519 keypair = SuiKeypair_fromSecretKey(PRIVATE_KEY);
      
      // Convert hex to bytes for signing
      uint8_t txBytes[txHexLen / 2];
      hexToBytes(txHex, txBytes, txHexLen / 2);
      
      // Sign transaction
      char signature_b64[256];
      SignatureResult sig = keypair.signTransaction(&keypair, txBytes, txHexLen / 2);
      bytesToBase64(sig.signature, sig.signatureLength, signature_b64);
      
      // Step 4: Submit to execute-sponsored
      http.begin(serverUrl + "/api/execute-sponsored");
      http.addHeader("Content-Type", "application/json");
      
      String payload = "{";
      payload += "\"temperature\":" + String(temperature) + ",";
      payload += "\"humidity\":" + String(humidity) + ",";
      payload += "\"ec\":" + String(ec) + ",";
      payload += "\"ph\":" + String(ph) + ",";
      payload += "\"timestamp\":" + String(getCurrentTimestamp()) + ",";
      payload += "\"signature\":\"" + String(signature_b64) + "\"";
      payload += "}";
      
      int submitCode = http.POST(payload);
      if (submitCode == 200) {
        Serial.println("✓ Transaction executed on blockchain!");
      }
      
      free(txHex);
    }
  }
  http.end();
}
```

---

### Approach 2: Server Builds Transaction (Offline Signing)

**Flow:**
```
ESP32 Device → Server (Build TX) → ESP32 (Sign with MicroSui) → Server (Execute TX) → Sui Blockchain
```

**Architecture:**
```
┌─────────────────────────────────────────────────────────┐
│              ESP32 with MicroSui Library                 │
│  ┌───────────────────────────────────────────────────┐  │
│  │  1. Collect sensor data (temp, humidity, EC, pH)  │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  2. POST sensor data to /api/build-tx             │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                      Server (Next.js)                    │
│  ┌───────────────────────────────────────────────────┐  │
│  │  3. Build unsigned transaction bytes              │  │
│  │     - Validate sensor data                        │  │
│  │     - Create Sui transaction structure            │  │
│  │     - Return txBytes (hex string)                 │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│              ESP32 with MicroSui Library                 │
│  ┌───────────────────────────────────────────────────┐  │
│  │  4. Sign transaction bytes using MicroSui         │  │
│  │     - Load private key from secure storage        │  │
│  │     - Sign txBytes using Ed25519                  │  │
│  │     - Generate signature (hex string)             │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  5. POST signature + txBytes to /api/submit-tx    │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                      Server (Next.js)                    │
│  ┌───────────────────────────────────────────────────┐  │
│  │  6. Execute signed transaction                    │  │
│  │     - Combine txBytes + signature                 │  │
│  │     - Submit to Sui network                       │  │
│  │     - Return transaction digest                   │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                  Sui Blockchain Network                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  7. Store sensor data on-chain                    │  │
│  │     - Validate with smart contract                │  │
│  │     - Emit SensorDataStoredEvent                  │  │
│  │     - Create SensorData object                    │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

**Benefits:**
- ✅ Private key never leaves the ESP32 device
- ✅ Maximum security - device maintains full control
- ✅ No server-side private key storage needed
- ✅ True decentralization - device owns its identity
- ✅ Suitable for production deployments

**Use Cases:**
- Production IoT deployments
- High-security applications
- Devices with unique blockchain identities
- Scenarios requiring transaction auditability

**ESP32 Code Example (Offline Signing):**
```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <MicroSui.h>  // MicroSui library for signing

// Your device's private key (store securely!)
const char* privateKey = "your_private_key_here";
const char* serverUrl = "https://your-server.com";

void submitSensorDataWithSigning() {
  // 1. Collect sensor data
  float temperature = readTemperature();
  float humidity = readHumidity();
  int ec = readEC();
  float ph = readPH();
  
  // 2. Request unsigned transaction from server
  HTTPClient http;
  http.begin(serverUrl + "/api/build-tx");
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{";
  payload += "\"temperature\":" + String(temperature) + ",";
  payload += "\"humidity\":" + String(humidity) + ",";
  payload += "\"ec\":" + String(ec) + ",";
  payload += "\"ph\":" + String(ph) + ",";
  payload += "\"deviceId\":\"ESP32_001\",";
  payload += "\"sensorType\":\"Soil Sensor\",";
  payload += "\"location\":\"Field A\"";
  payload += "}";
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    String txBytes = extractTxBytes(response);  // Parse JSON
    
    // 3. Sign transaction using MicroSui
    MicroSui sui;
    String signature = sui.signTransaction(txBytes, privateKey);
    
    // 4. Submit signed transaction
    http.begin(serverUrl + "/api/submit-tx");
    http.addHeader("Content-Type", "application/json");
    
    String submitPayload = "{";
    submitPayload += "\"txBytes\":\"" + txBytes + "\",";
    submitPayload += "\"signature\":\"" + signature + "\"";
    submitPayload += "}";
    
    int submitCode = http.POST(submitPayload);
    
    if (submitCode == 200) {
      Serial.println("Transaction submitted successfully!");
    }
  }
  
  http.end();
}
```

---

### Approach 3: Server-Side Signing (Simplified)

**Flow:**
```
ESP32 Device → Server (Sign & Execute) → Sui Blockchain
```

**Architecture:**
```
┌─────────────────────────────────────────────────────────┐
│                    ESP32 Device                          │
│  ┌───────────────────────────────────────────────────┐  │
│  │  1. Collect sensor data (temp, humidity, EC, pH)  │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  2. POST data to /api/sensor-data                 │  │
│  │     - Simple HTTP request with sensor readings    │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                      Server (Next.js)                    │
│  ┌───────────────────────────────────────────────────┐  │
│  │  3. Validate sensor data                          │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  4. Build transaction                             │  │
│  │     - Create Sui transaction structure            │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  5. Sign with server's private key                │  │
│  │     - Load private key from env variables         │  │
│  │     - Sign transaction                            │  │
│  └────────────────────┬──────────────────────────────┘  │
│                       ▼                                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  6. Execute transaction on Sui network            │  │
│  │     - Submit signed transaction                   │  │
│  │     - Wait for confirmation                       │  │
│  └────────────────────┬──────────────────────────────┘  │
└─────────────────────┼──────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│                  Sui Blockchain Network                  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  7. Store sensor data on-chain                    │  │
│  │     - All transactions from server's address      │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

**Benefits:**
- ✅ Simpler ESP32 code - no cryptography needed
- ✅ Lower memory requirements on device
- ✅ Faster implementation
- ✅ Single HTTP request from device
- ✅ Suitable for prototypes and development

**Use Cases:**
- Development and testing
- Proof of concept projects
- Resource-constrained devices
- Rapid prototyping
- Internal/trusted network deployments

**ESP32 Code Example (Server-Side Signing):**
```cpp
#include <WiFi.h>
#include <HTTPClient.h>

const char* serverUrl = "https://your-server.com";

void submitSensorDataSimple() {
  // 1. Collect sensor data
  float temperature = readTemperature();
  float humidity = readHumidity();
  int ec = readEC();
  float ph = readPH();
  
  // 2. Send directly to server - server handles everything
  HTTPClient http;
  http.begin(serverUrl + "/api/sensor-data");
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{";
  payload += "\"temperature\":" + String(temperature) + ",";
  payload += "\"humidity\":" + String(humidity) + ",";
  payload += "\"ec\":" + String(ec) + ",";
  payload += "\"ph\":" + String(ph) + ",";
  payload += "\"deviceId\":\"ESP32_001\",";
  payload += "\"sensorType\":\"Soil Sensor\",";
  payload += "\"location\":\"Field A\"";
  payload += "}";
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Data submitted successfully!");
    Serial.println(response);
  } else {
    Serial.println("Error submitting data");
  }
  
  http.end();
}
```

---

### Comparison: All Three Signing Approaches

| Feature | **Approach 1:<br/>ESP32 Builds TX** | **Approach 2:<br/>Server Builds TX<br/>(Offline Signing)** | **Approach 3:<br/>Server-Side Signing** |
|---------|-------------------------------------|-------------------------------------------------------------|------------------------------------------|
| **Security** | ⭐⭐⭐⭐⭐<br/>Maximum control | ⭐⭐⭐⭐⭐<br/>Private key on device | ⭐⭐⭐<br/>Private key on server |
| **Complexity** | ⭐⭐⭐⭐⭐<br/>Highest - BCS encoding | ⭐⭐⭐⭐<br/>Higher - requires MicroSui | ⭐⭐<br/>Lower - simple HTTP |
| **Device Memory** | Most (BCS + crypto) | More (crypto library) | Least (no crypto) |
| **Network Calls** | 2 (get digest + submit) | 2 (build + submit) | 1 (submit only) |
| **Transaction Control** | Full - ESP32 builds everything | Partial - Server builds structure | None - Server controls all |
| **Server Knowledge** | Only gas info | Complete TX structure | Complete TX + signing |
| **Implementation Time** | Longest (complex BCS) | Medium | Fastest |
| **Production Ready** | ✅ Advanced deployments | ✅ Standard deployments | ⚠️ Development only |
| **Decentralization** | Maximum | High | Low - centralized |
| **Gas Sponsorship** | Server sponsors | Server builds & ESP32 pays | Server pays all |
| **Transaction Auditability** | Each device unique TX | Each device unique signature | All from server address |
| **Educational Value** | ⭐⭐⭐⭐⭐<br/>Learn Sui internals | ⭐⭐⭐<br/>Learn signing | ⭐<br/>Basic integration |
| **Use Case** | Research, advanced IoT | Production IoT | Rapid prototyping |

### When to Use Each Approach

**Use Approach 1 (ESP32 Builds) when:**
- You want maximum control over transaction structure
- Learning Sui's transaction format and BCS encoding
- Building research or educational projects
- Need custom transaction requirements
- Want complete transparency of what's being signed
- Have sufficient ESP32 resources (4MB+ flash)

**Use Approach 2 (Offline Signing) when:**
- Deploying to production environments
- Each device needs unique blockchain identity
- Security is paramount but simplicity preferred
- Transaction auditability is required
- Standard transactions are sufficient
- Devices have sufficient memory (>4MB flash)

**Use Approach 3 (Server-Side Signing) when:**
- Rapid prototyping or development
- Testing sensor integrations
- Resource-constrained devices
- Centralized management is acceptable
- Quick proof-of-concept needed
- Trust relationship with server

---

### Required Libraries for ESP32

#### MicroSui Library

The offline signing approaches (1 and 2) use the **MicroSui** library - a lightweight Sui SDK for embedded devices:

**Features:**
- Ed25519 signing for ESP32
- Transaction serialization
- Minimal memory footprint
- Compatible with Sui blockchain

**Installation:**
```cpp
// Add to your platformio.ini or Arduino libraries
lib_deps = 
    microsui
```

**Basic Usage:**
```cpp
#include <MicroSui.h>

MicroSuiEd25519 keypair;

// Load keypair from private key
keypair = SuiKeypair_fromSecretKey("suiprivkey1q...");

// Get Sui address
const char* address = keypair.toSuiAddress(&keypair);

// Sign transaction bytes
SignatureResult sig = keypair.signTransaction(&keypair, txBytes, txLen);

// Verify signature (optional)
bool isValid = keypair.verifySignature(&keypair, txBytes, txLen, signature);
```

---

#### sui_transaction.cpp Library

**Approach 1** additionally uses the **sui_transaction.cpp** library for building transactions on-device:

**Features:**
- BCS (Binary Canonical Serialization) encoding
- Sui transaction structure building
- ProgrammableTransaction support
- Gas object handling
- Move function call encoding

**Core Functions:**
```cpp
#include "sui_transaction.h"
#include "bcs.h"

// Build a complete Sui transaction
bcs_error_t sui_build_sensor_transaction(
    const transaction_builder_t *params,
    char **output_hex,
    size_t *output_length
);

// Transaction builder parameters
transaction_builder_t params = {0};
params.package_id = packageIdBytes;      // 32 bytes
params.module_name = "sensor_storage";
params.function_name = "store_sensor_data";
params.sender = senderAddressBytes;      // 32 bytes
params.gas_object.object_id = gasIdBytes;
params.gas_object.version = gasVersion;
params.gas_object.digest = gasDigestBytes;
params.gas_budget = 100000000;
params.gas_price = 1000;
params.sensor_data.value1 = temperature;
params.sensor_data.value2 = humidity;
params.sensor_data.value3 = ec;
params.sensor_data.value4 = ph;

// Build transaction
char* txHex;
size_t txHexLen;
bcs_error_t err = sui_build_sensor_transaction(&params, &txHex, &txHexLen);
```

**BCS Utilities:**
```cpp
// Convert hex string to bytes
bcs_error_t bcs_hex_to_bytes(const char* hex, uint8_t* bytes, 
                              size_t max_len, size_t* bytes_read);

// Convert bytes to hex string
void bcs_bytes_to_hex(const uint8_t* bytes, size_t len, char* hex_out);

// Decode Base58 (for gas digest)
int base58_to_bytes(const char* input, size_t input_len, 
                    uint8_t* output, size_t output_size);
```

---

### Choosing the Right Approach

**Use Approach 1 (ESP32 Builds Transaction) when:**
- You want maximum control over transaction structure
- Learning Sui's transaction format and BCS encoding
- Building research or educational projects
- Need custom transaction requirements
- Want complete transparency of what's being signed
- Have sufficient ESP32 resources (4MB+ flash, 320KB+ RAM)
- Advanced IoT deployments with full auditability

**Use Approach 2 (Offline Signing) when:**
- Deploying to production environments
- Each device needs unique blockchain identity
- Security is paramount but simplicity preferred
- Transaction auditability is required
- Standard transactions are sufficient
- Devices have sufficient memory (>4MB flash)
- Want balance of security and ease of implementation

**Use Approach 3 (Server-Side Signing) when:**
- Rapid prototyping or development
- Testing sensor integrations
- Resource-constrained devices
- Centralized management is acceptable
- Quick proof-of-concept needed
- Trust relationship with server established

**Hybrid/Multi-Approach:**
You can implement multiple approaches and switch based on configuration or deployment:
```cpp
#define SIGNING_APPROACH 1  // 1=Build on ESP32, 2=Offline Sign, 3=Server Sign

void submitSensorData() {
  #if SIGNING_APPROACH == 1
    submitWithLocalBuild();    // Maximum control
  #elif SIGNING_APPROACH == 2
    submitWithOfflineSigning(); // Standard production
  #else
    submitSimple();            // Quick development
  #endif
}
```
```

## 📦 Project Structure

```
.
├── dapp/                          # Next.js frontend application
│   ├── app/
│   │   ├── api/                   # API routes
│   │   │   ├── create-digest/     # Get gas info (Approach 1)
│   │   │   ├── execute-sponsored/ # Execute signed TX (Approach 1)
│   │   │   ├── build-tx/          # Transaction builder (Approach 2)
│   │   │   ├── submit-tx/         # Submit signed TX (Approach 2)
│   │   │   ├── sensor-data/       # Data submission (Approach 3)
│   │   │   └── recent-data/       # Data retrieval
│   │   ├── dashboard/             # Dashboard page
│   │   └── page.tsx               # Home/data entry page
│   ├── components/                # React components
│   │   ├── WalletProvider.tsx     # Wallet context
│   │   ├── SensorDataChart.tsx    # Chart component
│   │   └── SensorDataForm.tsx     # Data entry form
│   ├── lib/                       # Utility functions
│   │   ├── sui-client.ts          # Sui client setup
│   │   ├── sui-transaction.ts     # Transaction handlers
│   │   └── transaction-builder.ts # TX building logic
│   └── public/                    # Static assets
│
├── sensor_storage/                # Move smart contract
│   ├── sources/
│   │   └── sensor_storage.move    # Main contract
│   ├── tests/
│   │   └── sensor_storage_tests.move
│   └── Move.toml                  # Package configuration
│
└── esp32/                         # ESP32 firmware (optional)
    ├── approach1_build_on_device/ # ESP32 builds transaction
    │   ├── main.cpp               # Main code with sui_transaction.cpp
    │   ├── sui_transaction.cpp    # BCS transaction builder
    │   ├── sui_transaction.h      # Transaction builder header
    │   └── bcs.h                  # BCS encoding utilities
    ├── approach2_offline_signing/ # Server builds, ESP32 signs
    │   └── main.cpp               # Offline signing implementation
    └── approach3_server_signing/  # Server builds and signs
        └── main.cpp               # Simple HTTP submission
```

## 🚀 Getting Started

### Prerequisites

- Node.js 18+ and npm/yarn/pnpm
- Sui CLI ([Installation guide](https://docs.sui.io/build/install))
- A Sui wallet (Sui Wallet, Ethos Wallet, etc.)
- Test SUI tokens from [faucet](https://faucet.sui.io/)
- **(Optional)** ESP32 development board for IoT integration
- **(Optional)** MicroSui library for ESP32 offline signing

### Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd <project-directory>
   ```

2. **Install frontend dependencies**
   ```bash
   cd dapp
   npm install
   # or
   yarn install
   # or
   pnpm install
   ```

3. **Set up environment variables**
   
   Create a `.env.local` file in the `dapp` directory:
   ```env
   # Sui Network Configuration
   SUI_NETWORK=testnet  # or mainnet, devnet
   NEXT_PUBLIC_SUI_NETWORK=testnet
   
   # Smart Contract Package ID (after deployment)
   NEXT_PUBLIC_PACKAGE_ID=<your_package_id>
   
   # Wallet Private Key (for backend transactions - KEEP SECURE!)
   SUI_PRIVATE_KEY=<your_private_key>
   ```

4. **Deploy the smart contract**
   ```bash
   cd sensor_storage
   
   # Build the contract
   sui move build
   
   # Deploy to network
   sui client publish --gas-budget 100000000
   ```
   
   Save the Package ID from the deployment output and add it to your `.env.local`.

5. **Start the development server**
   ```bash
   cd dapp
   npm run dev
   ```
   
   Open [http://localhost:3000](http://localhost:3000) in your browser.

6. **(Optional) Set up ESP32 firmware**
   
   **For offline signing approach:**
   ```bash
   cd esp32/offline_signing
   # Configure your WiFi credentials and server URL in main.cpp
   # Upload to ESP32 using PlatformIO or Arduino IDE
   ```
   
   **For server-side signing approach:**
   ```bash
   cd esp32/server_signing
   # Configure your WiFi credentials and server URL in main.cpp
   # Upload to ESP32 using PlatformIO or Arduino IDE
   ```

## 📖 Usage

### ESP32 Device Setup

**For Offline Signing (Production):**
1. Install MicroSui library in your ESP32 project
2. Generate a new Sui keypair for your device:
   ```bash
   sui client new-address ed25519
   ```
3. Store the private key securely in ESP32 flash
4. Fund the device's wallet with test SUI from faucet
5. Configure server URL in ESP32 code
6. Upload `esp32/offline_signing/main.cpp` to your device

**For Server-Side Signing (Development):**
1. Configure WiFi credentials and server URL
2. Upload `esp32/server_signing/main.cpp` to your device
3. No additional setup needed - server handles signing

### Submitting Sensor Data

1. **Connect your Sui wallet** via the dashboard
2. **Fill in the sensor data form**:
   - Temperature (°C)
   - Humidity (%)
   - EC - Electrical Conductivity (µS/cm)
   - pH level
   - Device ID
   - Sensor Type
   - Location (optional)
3. **Review gas estimation**
4. **Submit transaction** and approve in your wallet
5. **View confirmation** with transaction digest and explorer link

### Viewing Dashboard

Navigate to `/dashboard` to see:
- Real-time sensor data charts
- Temperature and humidity trends
- Statistics cards with averages
- Recent sensor readings table
- Auto-refreshing data (every 30 seconds)

### API Integration

#### Approach 1: ESP32 Builds Transaction (On-Device)

**Step 1: Get gas object info from server**

```bash
curl -X GET "http://localhost:3000/api/create-digest?senderAddress=0x..." \
  -H "Content-Type: application/json"
```

**Response:**
```json
{
  "sensorObjectId": "0x1234...",
  "sensorVersion": "12345",
  "gasObjectId": "0x5678...",
  "gasVersion": "67890",
  "gasDigest": "2mzbV7WevTHB..." 
}
```

**Step 2: Build transaction on ESP32 using sui_transaction.cpp**

```cpp
// Parse gas info from server response
DigestResponse digestInfo = parseResponse(response);

// Build transaction locally
transaction_builder_t params = {0};
// ... set all parameters (see code example above)

char* txHex;
size_t txHexLen;
bcs_error_t err = sui_build_sensor_transaction(&params, &txHex, &txHexLen);
```

**Step 3: Sign transaction on ESP32**

```cpp
// Sign with MicroSui
MicroSuiEd25519 keypair = SuiKeypair_fromSecretKey(PRIVATE_KEY);
SignatureResult sig = keypair.signTransaction(&keypair, txBytes, txLen);

// Convert to Base64
char signature_b64[256];
bytesToBase64(sig.signature, sig.signatureLength, signature_b64);
```

**Step 4: Submit signed transaction to server**

```bash
curl -X POST http://localhost:3000/api/execute-sponsored \
  -H "Content-Type: application/json" \
  -d '{
    "temperature": 2550,
    "humidity": 6500,
    "ec": 1200,
    "ph": 700,
    "timestamp": 1700000000,
    "signature": "AQNqKT7e8l5TZ2N8uW9vXY3zABcDEF..."
  }'
```

**Response:**
```json
{
  "success": true,
  "digest": "XYZ789...",
  "explorerUrl": "https://suiscan.xyz/testnet/tx/XYZ789...",
  "effects": { "status": "success", "gasUsed": "1234567" }
}
```

---

#### Approach 2: Server Builds, ESP32 Signs (Offline Signing)

**Submit sensor data directly:**

```bash
curl -X POST http://localhost:3000/api/sensor-data \
  -H "Content-Type: application/json" \
  -d '{
    "temperature": 25.5,
    "humidity": 65.0,
    "ec": 1200,
    "ph": 7.0,
    "deviceId": "SENSOR_001",
    "sensorType": "Soil Sensor",
    "location": "Farm Block A"
  }'
```

**Response:**
```json
{
  "success": true,
  "message": "Sensor data stored successfully on Sui blockchain",
  "transactionDigest": "ABC123...",
  "explorerUrl": "https://suiscan.xyz/testnet/tx/ABC123...",
  "timestamp": "2024-01-01T12:00:00.000Z"
}
```

---

#### Approach 2: Server Builds, ESP32 Signs (Offline Signing)

**Step 1: Build unsigned transaction**

```bash
curl -X POST http://localhost:3000/api/build-tx \
  -H "Content-Type: application/json" \
  -d '{
    "temperature": 25.5,
    "humidity": 65.0,
    "ec": 1200,
    "ph": 7.0,
    "deviceId": "SENSOR_001",
    "sensorType": "Soil Sensor",
    "location": "Farm Block A"
  }'
```

**Response:**
```json
{
  "success": true,
  "txBytes": "AAACAAgQJwAAAAAAAAgZAAAAAAAAABB4CgAAAAAAAAjMAQAAAAA...",
  "data": {
    "temperature": 2550,
    "humidity": 6500,
    "ec": 1200,
    "ph": 700,
    "deviceId": "SENSOR_001",
    "sensorType": "Soil Sensor",
    "location": "Farm Block A"
  },
  "timestamp": "2024-01-01T12:00:00.000Z",
  "instructions": "Sign these bytes with your private key and POST to /api/submit-tx"
}
```

**Step 2: Sign on ESP32 and submit signed transaction**

```bash
# After signing txBytes with MicroSui on ESP32
curl -X POST http://localhost:3000/api/submit-tx \
  -H "Content-Type: application/json" \
  -d '{
    "txBytes": "AAACAAgQJwAAAAAAAAgZAAAAAAAAABB4CgAAAAAAAAjMAQAAAAA...",
    "signature": "AQNqKT7e8l5TZ2N8uW9vXY3zABcDEF..."
  }'
```

**Response:**
```json
{
  "success": true,
  "transactionDigest": "XYZ789...",
  "explorerUrl": "https://suiscan.xyz/testnet/tx/XYZ789...",
  "effects": {
    "status": "success",
    "gasUsed": "1234567"
  }
}
```

---

#### Approach 3: Server-Side Signing (Simple)

**Submit sensor data directly (server handles everything):**

```bash
curl -X POST http://localhost:3000/api/sensor-data \
  -H "Content-Type: application/json" \
  -d '{
    "temperature": 25.5,
    "humidity": 65.0,
    "ec": 1200,
    "ph": 7.0,
    "deviceId": "SENSOR_001",
    "sensorType": "Soil Sensor",
    "location": "Farm Block A"
  }'
```

**Response:**
```json
{
  "success": true,
  "message": "Sensor data stored successfully on Sui blockchain",
  "transactionDigest": "ABC123...",
  "explorerUrl": "https://suiscan.xyz/testnet/tx/ABC123...",
  "timestamp": "2024-01-01T12:00:00.000Z"
}
```

---

#### ESP32 Complete Example (Approach 2 - Offline Signing)

```cpp
void loop() {
  // Collect sensor readings
  SensorData data = {
    .temperature = readTemperature(),
    .humidity = readHumidity(),
    .ec = readEC(),
    .ph = readPH(),
    .deviceId = "ESP32_001",
    .sensorType = "Soil Sensor",
    .location = "Field A"
  };
  
  // Step 1: Build transaction
  String txBytes = buildTransaction(data);
  if (txBytes == "") {
    Serial.println("Failed to build transaction");
    return;
  }
  
  // Step 2: Sign transaction
  MicroSui sui;
  String signature = sui.signTransaction(txBytes, PRIVATE_KEY);
  
  // Step 3: Submit signed transaction
  bool success = submitSignedTransaction(txBytes, signature);
  
  if (success) {
    Serial.println("✓ Data stored on blockchain!");
  }
  
  delay(60000);  // Wait 1 minute before next reading
}

String buildTransaction(SensorData data) {
  HTTPClient http;
  http.begin(SERVER_URL "/api/build-tx");
  http.addHeader("Content-Type", "application/json");
  
  String payload = createJsonPayload(data);
  int code = http.POST(payload);
  
  if (code == 200) {
    String response = http.getString();
    return parseJsonField(response, "txBytes");
  }
  
  return "";
}

bool submitSignedTransaction(String txBytes, String signature) {
  HTTPClient http;
  http.begin(SERVER_URL "/api/submit-tx");
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{\"txBytes\":\"" + txBytes + 
                   "\",\"signature\":\"" + signature + "\"}";
  
  int code = http.POST(payload);
  return (code == 200);
}
```

## 📊 Data Validation

### Smart Contract Validation
- **Temperature**: 0°C to 100°C
- **Humidity**: 0% to 100%
- **EC**: 0 to 5000 µS/cm
- **pH**: 0 to 14

### Frontend Validation
- Real-time input validation
- Range checking before submission
- Error messages for invalid data
- Gas estimation warnings

## 🔐 Security Features

- **Rate Limiting**: 10 requests per minute per IP
- **Input Validation**: Both frontend and smart contract levels
- **Range Checking**: Prevents out-of-bounds sensor values
- **Transaction Signing**: All writes require wallet approval
- **Error Handling**: Comprehensive error messages and recovery

## 🛠️ Development

### Running Tests

**Smart Contract Tests:**
```bash
cd sensor_storage
sui move test
```

**Frontend Tests:**
```bash
cd dapp
npm run test
```

### Code Quality

```bash
# Run linter
npm run lint

# Type checking
npm run type-check

# Format code
npm run format
```

### Building for Production

```bash
# Build frontend
cd dapp
npm run build

# Build smart contract
cd sensor_storage
sui move build --release
```

## 🌐 Network Configuration

The application supports multiple Sui networks:

| Network | Purpose | Faucet |
|---------|---------|--------|
| **Testnet** | Development & testing | ✅ Available |
| **Devnet** | Feature testing | ✅ Available |
| **Mainnet** | Production | ❌ Buy SUI |

Switch networks in your `.env.local` and wallet settings.

## 📚 Smart Contract Details

### Main Functions

**store_sensor_data()**
```move
entry fun store_sensor_data(
    temperature: u64,
    humidity: u64,
    ec: u64,
    ph: u64,
    device_id: vector<u8>,
    sensor_type: vector<u8>,
    location: vector<u8>,
    clock: &Clock,
    ctx: &mut TxContext
)
```

### Data Structure

```move
public struct SensorData has key, store {
    id: UID,
    temperature: u64,        // Celsius * 100
    humidity: u64,           // Percentage * 100
    ec: u64,                 // µS/cm
    ph: u64,                 // pH * 100
    timestamp: u64,          // Unix timestamp
    device_id: string::String,
    location: string::String,
    sensor_type: string::String,
}
```

### Events

```move
public struct SensorDataStoredEvent has copy, drop {
    object_id: address,
    device_id: string::String,
    temperature: u64,
    humidity: u64,
    ec: u64,
    ph: u64,
    timestamp: u64,
    sensor_type: string::String,
}
```

## 🔧 Troubleshooting

### Common Issues

**"Insufficient gas" error**
- Ensure your wallet has enough SUI tokens
- Get test tokens from the faucet for testnet

**"Transaction failed" error**
- Check if sensor values are within valid ranges
- Verify your package ID is correct in `.env.local`
- Ensure you're on the correct network

**"Rate limit exceeded" error**
- Wait 1 minute before submitting again
- Each IP is limited to 10 requests per minute

**Wallet connection issues**
- Refresh the page and try connecting again
- Ensure your wallet extension is installed and unlocked
- Check you're on the correct network in your wallet

**ESP32 connection issues**
- Verify WiFi credentials are correct
- Check server URL is accessible from ESP32's network
- Monitor serial output for error messages
- Ensure ESP32 has sufficient power supply (5V, 500mA+)

**ESP32 offline signing issues**
- Verify MicroSui library is properly installed
- Check private key format is correct (hex string)
- Ensure device has enough free memory for crypto operations
- Verify device wallet has sufficient SUI for gas fees

**ESP32 memory issues**
- Use ESP32 with at least 4MB flash for offline signing
- Enable PSRAM if available
- Reduce buffer sizes if out-of-memory errors occur
- Consider server-side signing for memory-constrained devices

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- Built on [Sui Blockchain](https://sui.io/)
- UI components from [shadcn/ui](https://ui.shadcn.com/)
- Charts powered by [Recharts](https://recharts.org/)
- Icons from [Lucide](https://lucide.dev/)

## ❓ FAQ

### General Questions

**Q: Which signing approach should I use?**
A: 
- **Approach 1 (ESP32 Builds)**: For research, education, or when you need maximum control and want to learn Sui's internals
- **Approach 2 (Offline Signing)**: For production deployments where security is critical and standard transactions are sufficient
- **Approach 3 (Server Signing)**: For development, testing, and proof-of-concept projects

**Q: Can I switch between signing approaches later?**
A: Yes! The smart contract and API support all three approaches. You can start with Approach 3 for development, then migrate to Approach 2 for production, or use Approach 1 for research.

**Q: How much does it cost to store sensor data?**
A: Gas costs vary by network. On testnet, transactions typically cost ~0.001 SUI. Get test tokens free from the faucet.

### ESP32 Questions

**Q: What ESP32 board should I use?**
A: 
- **Approach 1**: ESP32 with 4MB+ flash and 320KB+ RAM (ESP32-WROOM-32, ESP32-WROVER)
- **Approach 2**: ESP32 with 4MB+ flash (ESP32-WROOM-32, ESP32-DevKitC)
- **Approach 3**: Any ESP32 board (even ESP32-C3 or ESP8266)

**Q: How much memory does each approach need?**
A:
- **Approach 1**: ~150KB flash (BCS + crypto), ~40KB RAM during TX building
- **Approach 2**: ~80KB flash (crypto only), ~20KB RAM
- **Approach 3**: ~20KB flash (HTTP only), ~10KB RAM

**Q: How do I secure the private key on ESP32?**
A: Store it in ESP32's encrypted flash partition or use secure elements like ATECC608A for production deployments.

**Q: Can multiple ESP32 devices share the same private key?**
A: For server-side signing, yes (server's key). For offline signing, no - each device should have unique keys for proper auditability.

**Q: How often can ESP32 submit data?**
A: The API has rate limiting (10 requests/minute). For higher frequency, consider batching readings or contact about rate limit increases.

### Blockchain Questions

**Q: Where is my sensor data stored?**
A: Data is stored as objects on the Sui blockchain. Each sensor reading is an immutable on-chain object with a unique ID.

**Q: Can I delete or modify sensor data?**
A: No. Blockchain data is immutable. You can only add new readings. This ensures data integrity and prevents tampering.

**Q: How do I query historical data?**
A: Use the dashboard to view recent data, or query directly using Sui SDK with the device ID or owner address.

**Q: What happens if the server goes down?**
A: With offline signing, devices can still submit data directly to Sui RPC nodes. With server-side signing, the server must be available.

## 📞 Support

For questions or issues:
- Open an issue on GitHub
- Contact: [your-email@example.com]
- Documentation: [Link to docs]

---

**Built with ❤️ for IoT and Blockchain**
