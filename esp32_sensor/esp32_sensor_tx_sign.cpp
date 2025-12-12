#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// MicroSui and BCS includes (Ensure MicroSui library is installed via Library Manager)
#include <MicroSui.h>
// Note: BCS and sui_transaction headers are included within the MicroSui library
// but are not needed for direct inclusion here if using the keypair methods.

// ===== CONFIGURATION =====
// WiFi Credentials
const char *ssid = "bruh";
const char *password = "megabruh";

// Server API Endpoints (UPDATE WITH YOUR SERVER'S IP)
const char *buildTxUrl = "http://192.168.137.1:3000/api/build-tx";
const char *submitTxUrl = "http://192.168.137.1:3000/api/submit-tx";

// Sensor and Device Configuration
const char *deviceId = "ESP32_SENSOR_001";
const char *sensorType = "soil"; // soil, air, water, weather, industrial
const char *location = "Greenhouse A";

// SUI CONFIGURATION
// !!! CRITICAL: REPLACE THIS WITH YOUR ACTUAL SUI PRIVATE KEY IN BECH32 FORMAT (starts with suiprivkey1...)
// THIS IS A MOCK KEY: REPLACE IT.
const char *SUI_PRIVATE_KEY_BECH32 = ".....";

// Data generation settings
unsigned long sendInterval = 60000; // 60 seconds
unsigned long lastSendTime = 0;

// MicroSui Keypair globally accessible
MicroSuiEd25519 keypair;

// ===== HELPER FUNCTIONS (MicroSui & Utility) =====

// Safe MicroSui keypair initialization
bool initializeMicroSuiKeypair()
{
    Serial.println("Initializing MicroSui keypair...");

    // Try to load from Bech32
    keypair = SuiKeypair_fromSecretKey(SUI_PRIVATE_KEY_BECH32);

    // Check if keypair is valid by trying to get address
    const char *address = keypair.toSuiAddress(&keypair);
    if (address && strlen(address) > 0)
    {
        Serial.print("Keypair loaded - Address: ");
        Serial.println(address);
        return true;
    }

    Serial.println("❌ Failed to load keypair from Bech32. Check SUI_PRIVATE_KEY_BECH32.");
    return false;
}

/**
 * @brief Signs the transaction bytes (Hex string) using MicroSui and outputs the signature in Base64 format.
 * * @param transactionHex The unsigned transaction bytes as a Hex string (received from /api/build-tx).
 * @param signature_b64_out Buffer to store the resulting Base64 signature (must be at least 256 bytes).
 * @return true if signing was successful, false otherwise.
 */
bool signTransactionHex(const char *transactionHex, char *signature_b64_out)
{
    Serial.println("\n--- Starting Signature Process ---");
    Serial.printf("Signing Tx Hex (First 64): %.64s...\n", transactionHex);

    if (!keypair.isInitialized)
    {
        Serial.println("Cannot sign: Keypair not initialized.");
        return false;
    }

    // keypair.signTransaction expects the message in a string format (Hex)
    // It prepends the correct signature scheme and transaction intent for Sui.
    SuiSignature sig = keypair.signTransaction(&keypair, transactionHex);

    if (sig.signature)
    {
        Serial.print("✅ Signature Base64: ");
        Serial.println(sig.signature);

        // Copy signature (Base64 string) to output buffer
        // Using 255 to ensure null termination and avoid buffer overflow
        strncpy(signature_b64_out, sig.signature, 255);
        signature_b64_out[255] = '\0';
        Serial.println("--- Signature Process Complete ---");
        return true;
    }
    else
    {
        Serial.println("❌ Signature generation failed - no base64 returned");
        return false;
    }
}

float randomFloat(float min, float max)
{
    return min + static_cast<float>(random(0, 1000)) / 1000.0 * (max - min);
}

// ===== MAIN WORKFLOW FUNCTION =====

void generateAndSendData()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected. Attempting to reconnect...");
        WiFi.reconnect();
        delay(1000);
        return;
    }

    if (!keypair.isInitialized)
    {
        Serial.println("Cannot proceed: Keypair not initialized.");
        return;
    }

    // 1. Generate Sensor Data
    float temperature_f = randomFloat(20.0, 30.0);
    float humidity_f = randomFloat(40.0, 80.0);
    int ec = random(500, 1500);
    float ph_f = randomFloat(6.0, 7.5);

    // Convert float readings to u16 integer format (assuming 2 decimal places, e.g., 25.50C -> 2550)
    uint16_t temperature = (uint16_t)(temperature_f * 100);
    uint16_t humidity = (uint16_t)(humidity_f * 100);
    uint16_t ph = (uint16_t)(ph_f * 100);

    Serial.println("\n=== Starting Prepare-Sign-Submit Workflow ===");
    Serial.printf("Data: Temp=%d, Humid=%d, EC=%d, pH=%d\n", temperature, humidity, ec, ph);

    // 2. --- STEP 1: POST to /api/build-tx (Get Transaction Hex) ---
    Serial.printf("\n1. Requesting transaction bytes from: %s\n", buildTxUrl);

    HTTPClient buildHttp;
    buildHttp.begin(buildTxUrl);
    buildHttp.addHeader("Content-Type", "application/json");

    // Dynamic document is used to ensure it fits the data and location
    DynamicJsonDocument buildDoc(512);
    buildDoc["temperature"] = temperature;
    buildDoc["humidity"] = humidity;
    buildDoc["ec"] = ec;
    buildDoc["ph"] = ph;
    buildDoc["deviceId"] = deviceId;
    buildDoc["sensorType"] = sensorType;
    buildDoc["location"] = location;

    String buildPayload;
    serializeJson(buildDoc, buildPayload);

    int buildHttpCode = buildHttp.POST(buildPayload);

    String transactionHex = "";
    bool buildSuccess = false;

    if (buildHttpCode == 200)
    {
        String buildResponse = buildHttp.getString();
        // Use DynamicJsonDocument for response as txBytes string can be long
        DynamicJsonDocument responseDoc(1536);
        DeserializationError error = deserializeJson(responseDoc, buildResponse);

        if (!error && responseDoc["success"])
        {
            transactionHex = responseDoc["txBytes"].as<String>();
            Serial.printf("✅ Tx Bytes Received (Length: %d)\n", transactionHex.length());
            Serial.printf("  Tx Bytes Hex (First 64): %.64s...\n", transactionHex.c_str());
            buildSuccess = true;
        }
        else
        {
            Serial.printf("❌ Build TX API Error: %s\n", responseDoc["error"].as<const char *>());
        }
    }
    else
    {
        Serial.printf("❌ Build TX HTTP Error: %d - %s\n", buildHttpCode, buildHttp.errorToString(buildHttpCode).c_str());
    }
    buildHttp.end();

    if (!buildSuccess)
    {
        Serial.println("Workflow failed at BUILD TX stage.");
        return;
    }

    // 3. --- STEP 2: Sign Transaction Locally ---
    char signatureBase64[256]; // Buffer for Base64 signature (~130 chars)

    bool signSuccess = signTransactionHex(transactionHex.c_str(), signatureBase64);

    if (!signSuccess)
    {
        Serial.println("Workflow failed at SIGNATURE stage.");
        return;
    }

    // 4. --- STEP 3: POST to /api/submit-tx (Submit Transaction and Signature) ---
    Serial.printf("\n3. Submitting transaction to: %s\n", submitTxUrl);

    HTTPClient submitHttp;
    submitHttp.begin(submitTxUrl);
    submitHttp.addHeader("Content-Type", "application/json");

    // Use DynamicJsonDocument for payload as txBytes is a long string
    DynamicJsonDocument submitDoc(1536);
    submitDoc["txBytes"] = transactionHex;    // Send the original bytes received
    submitDoc["signature"] = signatureBase64; // Send the generated Base64 signature

    String submitPayload;
    serializeJson(submitDoc, submitPayload);

    int submitHttpCode = submitHttp.POST(submitPayload);

    if (submitHttpCode == 200)
    {
        String submitResponse = submitHttp.getString();
        StaticJsonDocument<512> resultDoc;
        DeserializationError error = deserializeJson(resultDoc, submitResponse);

        if (!error && resultDoc["success"])
        {
            Serial.println("✅ Transaction submitted successfully!");
            Serial.printf("  TX Digest: %s\n", resultDoc["digest"].as<const char *>());
            Serial.printf("  Explorer URL: %s\n", resultDoc["explorerUrl"].as<const char *>());
        }
        else
        {
            Serial.printf("❌ Submit TX API Error: %s\n", resultDoc["error"].as<const char *>());
        }
    }
    else
    {
        Serial.printf("❌ Submit TX HTTP Error: %d - %s\n", submitHttpCode, submitHttp.errorToString(submitHttpCode).c_str());
        String errorResponse = submitHttp.getString();
        Serial.printf("  Error Response: %s\n", errorResponse.c_str());
    }

    submitHttp.end();
    Serial.println("\n=== Workflow Complete ===");
}

// ===== SETUP & LOOP (Modified) =====
void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== ESP32 Sui Sensor Data Sender (Prepare-Sign-Submit) ===");

    // Connect to WiFi
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n✅ WiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\n❌ WiFi Connection Failed!");
    }

    // Initialize MicroSui Keypair
    if (!initializeMicroSuiKeypair())
    {
        // If keypair fails to load, halt.
        Serial.println("System Halted due to Keypair Error.");
        while (true)
        {
            delay(1000);
        }
    }

    // Initialize random seed
    randomSeed(analogRead(0));
}

void loop()
{
    unsigned long currentTime = millis();

    // Check if it's time to send data
    if (currentTime - lastSendTime >= sendInterval)
    {
        generateAndSendData();
        lastSendTime = currentTime;
    }

    // Optional: Add manual trigger via Serial
    if (Serial.available() > 0)
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "send")
        {
            generateAndSendData();
            lastSendTime = currentTime;
        }
        else if (command == "interval")
        {
            Serial.print("Current interval: ");
            Serial.print(sendInterval / 1000);
            Serial.println(" seconds");
            Serial.println("Enter new interval in seconds:");

            while (!Serial.available())
                ;
            String newInterval = Serial.readStringUntil('\n');
            sendInterval = newInterval.toInt() * 1000;
            Serial.print("Interval set to: ");
            Serial.print(sendInterval / 1000);
            Serial.println(" seconds");
        }
    }

    delay(100);
}