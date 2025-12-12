# 🌱 Sui Sensor Storage DApp
A decentralized application (DApp) for storing and visualizing IoT sensor data on the Sui blockchain. This project combines a Next.js frontend with a Move smart contract and **supports two ESP32 integration approaches**: offline signing (secure) and server-side signing (simple).

> **🔐 Security First**: This project demonstrates production-grade blockchain integration for IoT devices with offline transaction signing using MicroSui on ESP32 microcontrollers.

## 📋 Overview

This project enables IoT devices to store sensor data directly on the Sui blockchain, providing:
- **Immutable Data Storage**: Sensor readings stored permanently on-chain
- **Real-time Dashboard**: Interactive visualization of sensor data
- **Decentralized Architecture**: No central database required
- **Data Integrity**: Blockchain-backed verification of all readings
- **Multi-sensor Support**: Temperature, Humidity, EC, and pH measurements

## 🚀 Quick Start

Choose your integration approach:

### 🔐 Option A: Offline Signing (Production-Ready)
**Best for:** Production deployments, high-security applications
```
ESP32 (Collect Data) → Server (Build TX) → ESP32 (Sign with MicroSui) 
→ Server (Submit) → Sui Blockchain ✓
```
- ✅ Private keys stay on device
- ✅ Each device has unique blockchain identity
- ✅ Maximum security

### ⚡ Option B: Server-Side Signing (Rapid Development)
**Best for:** Prototyping, testing, simple deployments
```
ESP32 (Collect Data) → Server (Sign & Submit) → Sui Blockchain ✓
```
- ✅ Simpler ESP32 code
- ✅ Lower memory requirements
- ✅ Faster implementation

[See detailed comparison →](#comparison-offline-signing-vs-server-side-signing)

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
- **POST /api/sensor-data**: Submit sensor data (server-side signing)
- **POST /api/build-tx**: Build unsigned transaction bytes (for offline signing)
- **POST /api/submit-tx**: Submit signed transaction (for offline signing)
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

This project supports two different approaches for ESP32 IoT devices to submit sensor data to the blockchain:

### Approach 1: Offline Signing (Client-Side Signing)

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

### Approach 2: Server-Side Signing (Simplified)

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

### Comparison: Offline Signing vs Server-Side Signing

| Feature | Offline Signing (ESP32) | Server-Side Signing |
|---------|------------------------|---------------------|
| **Security** | ⭐⭐⭐⭐⭐ Private key on device | ⭐⭐⭐ Private key on server |
| **Complexity** | Higher - requires MicroSui | Lower - simple HTTP |
| **Device Memory** | More (crypto library) | Less (no crypto) |
| **Network Calls** | 2 requests (build + submit) | 1 request |
| **Implementation Time** | Longer | Faster |
| **Production Ready** | ✅ Yes | ⚠️ Development only |
| **Decentralization** | Full - each device has identity | Centralized - server identity |
| **Gas Costs** | Device pays from own wallet | Server pays for all devices |
| **Transaction Speed** | Slightly slower (2 round trips) | Faster (1 round trip) |
| **Auditability** | Each device traceable | All from same address |

---

### MicroSui Library

The offline signing approach uses the **MicroSui** library - a lightweight Sui SDK for embedded devices:

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

MicroSui sui;

// Sign transaction bytes
String signature = sui.signTransaction(txBytesHex, privateKeyHex);

// Verify signature (optional)
bool isValid = sui.verifySignature(txBytesHex, signature, publicKeyHex);
```

---

### Choosing the Right Approach

**Use Offline Signing when:**
- Deploying to production environments
- Each device needs unique blockchain identity
- Security is paramount
- Transaction auditability is required
- Devices have sufficient memory (>4MB flash)

**Use Server-Side Signing when:**
- Rapid prototyping or development
- Testing sensor integrations
- Resource-constrained devices
- Centralized management is acceptable
- Quick proof-of-concept needed

**Hybrid Approach:**
You can also implement both approaches and switch based on configuration:
```cpp
#define USE_OFFLINE_SIGNING true  // Toggle between approaches

void submitSensorData() {
  #if USE_OFFLINE_SIGNING
    submitSensorDataWithSigning();
  #else
    submitSensorDataSimple();
  #endif
}
```

## 📦 Project Structure

```
.
├── dapp/                          # Next.js frontend application
│   ├── app/
│   │   ├── api/                   # API routes
│   │   │   ├── build-tx/          # Transaction builder endpoint
│   │   │   ├── sensor-data/       # Data submission endpoint
│   │   │   ├── submit-tx/         # Transaction submission
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
    ├── offline_signing/           # ESP32 code with MicroSui
    │   └── main.cpp               # Offline signing implementation
    └── server_signing/            # ESP32 code without signing
        └── main.cpp               # Server-side signing implementation
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

#### Approach 1: Server-Side Signing (Simple)

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

#### Approach 2: Offline Signing (Two-Step Process)

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

#### ESP32 Complete Example (Offline Signing)

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


## 🙏 Acknowledgments

- Built on [Sui Blockchain](https://sui.io/)
- UI components from [shadcn/ui](https://ui.shadcn.com/)
- Charts powered by [Recharts](https://recharts.org/)
- Icons from [Lucide](https://lucide.dev/)

## ❓ FAQ

### General Questions

**Q: Which signing approach should I use?**
A: Use offline signing for production deployments where security is critical. Use server-side signing for development, testing, and proof-of-concept projects.

**Q: Can I switch between signing approaches later?**
A: Yes! The smart contract and API support both approaches. You can start with server-side signing for development and migrate to offline signing for production.

**Q: How much does it cost to store sensor data?**
A: Gas costs vary by network. On testnet, transactions typically cost ~0.001 SUI. Get test tokens free from the faucet.

### ESP32 Questions

**Q: What ESP32 board should I use?**
A: Any ESP32 board works for server-side signing. For offline signing, use ESP32 with 4MB+ flash (ESP32-WROOM-32, ESP32-DevKitC).

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


**Built with ❤️ for IoT and Blockchain**#   s u i - s e n s o r - a p p  
 