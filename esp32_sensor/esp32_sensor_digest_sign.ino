#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MicroSui.h>
#include "bcs.h"
#include "sui_transaction.h"

// WiFi credentials
const char* ssid = "bruh";
const char* password = "megabruh";

// Server configuration
const char* serverBaseUrl = "http://192.168.137.1:3000";
const char* createDigestUrl = "/api/create-digest";
const char* executeSponsoredUrl = "/api/execute-sponsored";

// Your ESP32's private key for signing (in Bech32 format)
const char* SUI_PRIVATE_KEY_BECH32 = "suiprivkey1q.........em";

// Configuration
#define SENSOR_READ_INTERVAL 60000  // Read every 60 seconds
#define SENSOR_PACKAGE_ID "" // e.g., "0x1234..."
#define SENSOR_MODULE "sensor_storage"
#define SENSOR_FUNCTION "store_sensor_data"

// Sensor data structure
struct SensorData {
  uint16_t temperature;  // in hundredths (25.50°C = 2550)
  uint16_t humidity;     // in hundredths (65.50% = 6550)
  uint16_t ec;           // Electrical Conductivity (µS/cm)
  uint16_t ph;           // in hundredths (7.25 = 725)
  uint64_t timestamp;
};

// Digest response structure
struct DigestResponse {
  char sensorObjectId[67];    // 0x + 64 hex chars
  char sensorVersion[32];
  char gasObjectId[67];       // 0x + 64 hex chars
  char gasVersion[32];
  char gasDigest[88];         // Base58 encoded
};

// Global variables
MicroSuiEd25519 keypair;
SensorData currentSensorData;
bool timeSynchronized = false;
unsigned long lastSensorRead = 0;
unsigned long lastTimeUpdate = 0;
const unsigned long TIME_UPDATE_INTERVAL = 3600000; // Update time every hour

// Helper function declarations
void initializeWiFi();
void initializeTime();
bool readSensorData();
void processAndSubmitTransaction();
bool getDigestInfo(DigestResponse* digestInfo);
bool buildTransaction(DigestResponse* digestInfo, char** transactionHex, size_t* transactionHexLen);
bool signTransactionWithMicroSui(const char* transactionHex, char* signature_b64);
void executeSponsoredTransaction(const char* signature_b64, DigestResponse* digestInfo);
uint64_t getCurrentTimestamp();
void trimString(char* str);
void printLocalTime();
void updateTime();

// Base58 decoding helper
static const char base58_table[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static int base58_char_value(char c) {
  const char* pos = strchr(base58_table, c);
  if (pos == NULL) return -1;
  return pos - base58_table;
}

int base58_to_bytes(const char* input, size_t input_len, uint8_t* output, size_t output_size) {
  if (input_len == 0) {
    Serial.println("   Base58 input is empty");
    return -1;
  }

  // Base58 decoding: treat as big-endian base-58 number and convert to bytes
  uint8_t* temp = (uint8_t*)malloc(output_size * 2);  // Temporary buffer for intermediate calculations
  if (!temp) {
    Serial.println("   Base58 decode: memory allocation failed");
    return -1;
  }

  memset(temp, 0, output_size * 2);
  size_t temp_len = 0;

  // Process each base58 character
  for (size_t i = 0; i < input_len; i++) {
    int char_value = base58_char_value(input[i]);
    if (char_value < 0) {
      Serial.printf("   Invalid base58 char at position %d: '%c'\n", i, input[i]);
      free(temp);
      return -1;
    }

    // Multiply current value by 58 and add new digit
    uint32_t carry = char_value;
    for (size_t j = 0; j < temp_len || carry > 0; j++) {
      if (j >= output_size * 2) {
        Serial.println("   Base58 decode: buffer overflow");
        free(temp);
        return -1;
      }
      carry += (uint32_t)temp[j] * 58;
      temp[j] = carry & 0xFF;
      carry >>= 8;
      if (j >= temp_len) temp_len = j + 1;
    }
  }

  // Reverse the bytes (big-endian to little-endian) and copy to output
  // Find the actual length (skip leading zeros)
  size_t actual_len = temp_len;
  while (actual_len > 0 && temp[actual_len - 1] == 0) {
    actual_len--;
  }

  if (actual_len > output_size) {
    Serial.printf("   Base58 decode: output too large (%d bytes, expected max %d)\n", actual_len, output_size);
    free(temp);
    return -1;
  }

  // Copy reversed bytes to output (pad with zeros at the beginning if needed)
  memset(output, 0, output_size);
  for (size_t i = 0; i < actual_len; i++) {
    output[output_size - actual_len + i] = temp[actual_len - 1 - i];
  }

  free(temp);
  return 0;
}

// Base64 decoding helper (for sensor digest if needed)
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int base64_char_value(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '+') return 62;
  if (c == '/') return 63;
  return -1;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 Sensor Node (Digest Sign) Starting...");
  Serial.println("==========================================");

  // Initialize WiFi
  initializeWiFi();

  // Initialize MicroSui keypair
  Serial.println("Initializing MicroSui keypair...");
  keypair = SuiKeypair_fromSecretKey(SUI_PRIVATE_KEY_BECH32);
  
  const char* address = keypair.toSuiAddress(&keypair);
  if (address && strlen(address) > 0) {
    Serial.print("Keypair loaded - Address: ");
    Serial.println(address);
  } else {
    Serial.println("Failed to load keypair");
  }

  // Initialize time
  initializeTime();

  Serial.println("ESP32 Sensor Node Ready");
  Serial.println("=======================");
}

void loop() {
  unsigned long currentTime = millis();

  // Update time periodically
  if (WiFi.status() == WL_CONNECTED && millis() - lastTimeUpdate > TIME_UPDATE_INTERVAL) {
    updateTime();
  }

  // Read sensor data at intervals
  if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL) {
    Serial.println("\n=== Reading Sensor Data ===");
    
    if (readSensorData()) {
      Serial.println("Sensor data collected successfully");
      
      // Convert to human-readable format for display
      float temp = currentSensorData.temperature / 100.0;
      float hum = currentSensorData.humidity / 100.0;
      float ph = currentSensorData.ph / 100.0;
      
      Serial.printf("Temperature: %.2f°C\n", temp);
      Serial.printf("Humidity: %.2f%%\n", hum);
      Serial.printf("EC: %d µS/cm\n", currentSensorData.ec);
      Serial.printf("pH: %.2f\n", ph);
      Serial.printf("Timestamp: %llu\n", currentSensorData.timestamp);
      
      // Process and submit transaction
      processAndSubmitTransaction();
    } else {
      Serial.println("Failed to read sensor data");
    }
    
    lastSensorRead = currentTime;
  }

  delay(1000);
}

void initializeWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connection failed");
  }
}

void initializeTime() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - cannot initialize time");
    return;
  }

  updateTime();
}

void updateTime() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected for time update");
    return;
  }

  Serial.println("Updating time...");
  
  // For now, we'll use a simple HTTP-based time service
  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/ip");
  
  int httpCode = http.GET();
  if (httpCode == 200) {
    String response = http.getString();
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
      uint64_t unix_time = doc["unixtime"];
      if (unix_time > 0) {
        // Set system time (simplified - ESP32 doesn't have real time clock)
        timeSynchronized = true;
        lastTimeUpdate = millis();
        Serial.printf("Time updated: %llu\n", unix_time);
      }
    }
  }
  
  http.end();
}

uint64_t getCurrentTimestamp() {
  if (timeSynchronized) {
    // For simplicity, we'll use millis offset
    // In production, you'd want to store the actual Unix time
    return 1700000000 + (millis() / 1000);
  } else {
    return 1700000000 + (millis() / 1000);
  }
}

bool readSensorData() {
  // Simulate sensor reading - replace with actual sensor code
  randomSeed(millis());
  
  currentSensorData.temperature = random(2000, 3000);  // 20.00-30.00°C
  currentSensorData.humidity = random(4000, 8000);     // 40.00-80.00%
  currentSensorData.ec = random(500, 2000);            // 500-2000 µS/cm
  currentSensorData.ph = random(600, 800);             // 6.00-8.00
  currentSensorData.timestamp = getCurrentTimestamp();
  
  return true;
}

void processAndSubmitTransaction() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - cannot process transaction");
    return;
  }

  Serial.println("\n=== STARTING TRANSACTION PROCESS ===");

  // Step 1: Get digest info from API
  DigestResponse digestInfo;
  if (!getDigestInfo(&digestInfo)) {
    Serial.println("Failed to get digest info");
    return;
  }

  // Step 2: Build transaction locally
  char* transactionHex = nullptr;
  size_t transactionHexLen = 0;
  if (!buildTransaction(&digestInfo, &transactionHex, &transactionHexLen)) {
    Serial.println("Failed to build transaction");
    return;
  }

  // Step 3: Sign transaction
  char signature_b64[256];
  if (!signTransactionWithMicroSui(transactionHex, signature_b64)) {
    Serial.println("Failed to sign transaction");
    free(transactionHex);
    return;
  }

  // Step 4: Submit to execute-sponsored API
  executeSponsoredTransaction(signature_b64, &digestInfo);

  // Cleanup
  free(transactionHex);

  Serial.println("=== TRANSACTION PROCESS COMPLETE ===");
}

bool getDigestInfo(DigestResponse* digestInfo) {
  Serial.println("Getting digest info from API...");

  HTTPClient http;
  
  // Get sender address from keypair
  const char* address = keypair.toSuiAddress(&keypair);
  String url = String(serverBaseUrl) + createDigestUrl + "?senderAddress=" + String(address);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  Serial.print("Sending GET request to: ");
  Serial.println(url);

  int httpCode = http.GET();

  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Received digest info from API");
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.printf("JSON parse failed: %s\n", error.c_str());
      http.end();
      return false;
    }

    // Parse response
    const char* sensorObjectId = doc["sensorObjectId"];
    const char* sensorVersion = doc["sensorVersion"];
    const char* gasObjectId = doc["gasObjectId"];
    const char* gasVersion = doc["gasVersion"];
    const char* gasDigest = doc["gasDigest"];

    if (!sensorObjectId || !sensorVersion || !gasObjectId || !gasVersion || !gasDigest) {
      Serial.println("Missing required fields in digest response");
      http.end();
      return false;
    }

    // Copy to struct
    strncpy(digestInfo->sensorObjectId, sensorObjectId, sizeof(digestInfo->sensorObjectId) - 1);
    strncpy(digestInfo->sensorVersion, sensorVersion, sizeof(digestInfo->sensorVersion) - 1);
    strncpy(digestInfo->gasObjectId, gasObjectId, sizeof(digestInfo->gasObjectId) - 1);
    strncpy(digestInfo->gasVersion, gasVersion, sizeof(digestInfo->gasVersion) - 1);
    strncpy(digestInfo->gasDigest, gasDigest, sizeof(digestInfo->gasDigest) - 1);

    Serial.println("Digest info parsed successfully:");
    Serial.printf("  Sensor Object ID: %s\n", digestInfo->sensorObjectId);
    Serial.printf("  Sensor Version: %s\n", digestInfo->sensorVersion);
    Serial.printf("  Gas Object ID: %s\n", digestInfo->gasObjectId);
    Serial.printf("  Gas Version: %s\n", digestInfo->gasVersion);
    Serial.printf("  Gas Digest (Base58): %.32s...\n", digestInfo->gasDigest);

    http.end();
    return true;
  } else {
    Serial.printf("HTTP GET failed: %d\n", httpCode);
    String response = http.getString();
    if (response.length() > 0) {
      Serial.println(response);
    }
    http.end();
    return false;
  }
}

bool buildTransaction(DigestResponse* digestInfo, char** transactionHex, size_t* transactionHexLen) {
  Serial.println("Building transaction locally...");

  // Prepare transaction builder parameters
  transaction_builder_t params = { 0 };

  // Package ID (convert from hex string)
  size_t bytes_read;
  Serial.printf("Converting Package ID: %s\n", SENSOR_PACKAGE_ID);
  bcs_error_t err = bcs_hex_to_bytes(SENSOR_PACKAGE_ID, params.package_id, 32, &bytes_read);
  if (err != BCS_OK || bytes_read != 32) {
    Serial.printf("Failed to convert Package ID: error %d, bytes_read: %d\n", err, bytes_read);
    return false;
  }
  Serial.println("Package ID converted successfully");

  // Module and function names
  params.module_name = SENSOR_MODULE;
  params.function_name = SENSOR_FUNCTION;

  // Sender address (from keypair)
  const char* address = keypair.toSuiAddress(&keypair);
  Serial.printf("Converting Sender Address: %s\n", address);
  err = bcs_hex_to_bytes(address, params.sender, 32, &bytes_read);
  if (err != BCS_OK || bytes_read != 32) {
    Serial.printf("Failed to convert Sender Address: error %d, bytes_read: %d\n", err, bytes_read);
    return false;
  }
  Serial.println("Sender Address converted successfully");

  // Sensor object (convert from hex string)
  Serial.printf("Converting Sensor Object ID: %s\n", digestInfo->sensorObjectId);
  
  // Extract hex part (skip 0x prefix if present)
  const char* sensorIdHex = digestInfo->sensorObjectId;
  if (strncmp(sensorIdHex, "0x", 2) == 0) {
    sensorIdHex += 2;
  }
  
  // Truncate to exactly 64 hex characters (32 bytes)
  char sensorIdHexClean[65] = { 0 };
  size_t hexLen = strlen(sensorIdHex);
  size_t copyLen = (hexLen > 64) ? 64 : hexLen;
  strncpy(sensorIdHexClean, sensorIdHex, copyLen);
  sensorIdHexClean[64] = '\0';

  err = bcs_hex_to_bytes(sensorIdHexClean, params.sensor_object_id, 32, &bytes_read);
  if (err != BCS_OK || bytes_read != 32) {
    Serial.printf("Failed to convert Sensor Object ID: error %d, bytes_read: %d\n", err, bytes_read);
    return false;
  }
  Serial.println("Sensor Object ID converted successfully");

  // Sensor version and digest
  params.sensor_initial_shared_version = strtoull(digestInfo->sensorVersion, NULL, 10);
  
  // Note: For owned objects, we need the digest. In this flow, the server doesn't provide it.
  // We'll need to fetch it separately or use a placeholder.
  // For now, we'll use zeros (this is a limitation - in production you'd need to get the actual digest)
  memset(params.sensor_digest, 0, 32);
  Serial.println("Note: Using placeholder sensor digest (need to fetch from network)");

  params.sensor_mutable = false;  // Typically not mutable for sensor objects
  params.only_transaction_kind = false;  // Build full transaction block

  // Sensor data
  params.sensor_data.value1 = currentSensorData.temperature;
  params.sensor_data.value2 = currentSensorData.humidity;
  params.sensor_data.value3 = currentSensorData.ec;
  params.sensor_data.value4 = currentSensorData.ph;
  params.sensor_data.timestamp = currentSensorData.timestamp;

  // Gas object (convert from hex string)
  Serial.printf("Converting Gas Object ID: %s\n", digestInfo->gasObjectId);
  
  // Extract hex part (skip 0x prefix if present)
  const char* gasIdHex = digestInfo->gasObjectId;
  if (strncmp(gasIdHex, "0x", 2) == 0) {
    gasIdHex += 2;
  }
  
  // Truncate to exactly 64 hex characters (32 bytes)
  char gasIdHexClean[65] = { 0 };
  size_t gasHexLen = strlen(gasIdHex);
  size_t gasCopyLen = (gasHexLen > 64) ? 64 : gasHexLen;
  strncpy(gasIdHexClean, gasIdHex, gasCopyLen);
  gasIdHexClean[64] = '\0';

  err = bcs_hex_to_bytes(gasIdHexClean, params.gas_object.object_id, 32, &bytes_read);
  if (err != BCS_OK || bytes_read != 32) {
    Serial.printf("Failed to convert Gas Object ID: error %d, bytes_read: %d\n", err, bytes_read);
    return false;
  }
  Serial.println("Gas Object ID converted successfully");

  // Gas version and digest (Base58 decode)
  params.gas_object.version = strtoull(digestInfo->gasVersion, NULL, 10);
  
  // Convert gas digest from Base58 to bytes
  Serial.printf("Decoding gas digest (Base58, length: %d): %.32s...\n", strlen(digestInfo->gasDigest), digestInfo->gasDigest);
  if (base58_to_bytes(digestInfo->gasDigest, strlen(digestInfo->gasDigest), params.gas_object.digest, 32) != 0) {
    Serial.println("Failed to decode gas digest from Base58");
    return false;
  }
  Serial.println("Gas digest decoded from Base58 to bytes");

  // Gas budget and price
  params.gas_budget = 100000000;
  params.gas_price = 1000;

  Serial.println("Transaction parameters prepared:");
  Serial.printf("  Temperature: %u\n", params.sensor_data.value1);
  Serial.printf("  Humidity: %u\n", params.sensor_data.value2);
  Serial.printf("  pH: %u\n", params.sensor_data.value3);
  Serial.printf("  EC: %u\n", params.sensor_data.value4);
  Serial.printf("  Timestamp: %llu\n", params.sensor_data.timestamp);
  Serial.printf("  Gas budget: %llu\n", params.gas_budget);
  Serial.printf("  Gas price: %llu\n", params.gas_price);

  // Build the transaction
  err = sui_build_sensor_transaction(&params, transactionHex, transactionHexLen);

  if (err != BCS_OK) {
    Serial.printf("Failed to build transaction: error code %d\n", err);
    return false;
  }

  Serial.printf("Transaction built successfully: %zu bytes (hex length: %zu)\n",
                *transactionHexLen / 2, *transactionHexLen);
  Serial.print("Transaction hex (first 128 chars): ");
  for (int i = 0; i < 128 && i < strlen(*transactionHex); i++) {
    Serial.print((*transactionHex)[i]);
  }
  Serial.println("...");

  return true;
}

bool signTransactionWithMicroSui(const char* transactionHex, char* signature_b64_out) {
  Serial.println("\n[2/2] Generating Signature...");

  Serial.printf("Signing Tx Hex: %s\n", transactionHex);

  SuiSignature sig = keypair.signTransaction(&keypair, transactionHex);

  if (sig.signature) {
    Serial.print("Signature in BASE64 format: ");
    Serial.println(sig.signature);

    // Copy signature (Base64 string) to output buffer
    strncpy(signature_b64_out, sig.signature, 255);
    signature_b64_out[255] = '\0';
    return true;
  } else {
    Serial.println("Signature generation failed - no base64 returned");
    return false;
  }
}

void executeSponsoredTransaction(const char* signature_b64, DigestResponse* digestInfo) {
  Serial.println("Submitting transaction to execute-sponsored API...");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;
  String url = String(serverBaseUrl) + executeSponsoredUrl;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(1024);
  doc["temperature"] = currentSensorData.temperature;
  doc["humidity"] = currentSensorData.humidity;
  doc["ec"] = currentSensorData.ec;
  doc["ph"] = currentSensorData.ph;
  doc["timestamp"] = currentSensorData.timestamp;
  doc["signature"] = signature_b64;
 

  Serial.println("Including sensor data and signature in POST request:");
  Serial.printf("  Temperature: %u\n", currentSensorData.temperature);
  Serial.printf("  Humidity: %u\n", currentSensorData.humidity);
  Serial.printf("  EC: %u\n", currentSensorData.ec);
  Serial.printf("  pH: %u\n", currentSensorData.ph);
  Serial.printf("  Timestamp: %llu\n", currentSensorData.timestamp);
  Serial.printf("  Signature: %s\n", signature_b64);

  String payload;
  serializeJson(doc, payload);

  Serial.println("Sending POST request...");
  Serial.printf("Payload size: %d bytes\n", payload.length());

  int httpCode = http.POST(payload);

  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("POST successful");
    Serial.println(response);
  } else {
    Serial.printf("POST failed: %d\n", httpCode);
    String response = http.getString();
    if (response.length() > 0) {
      Serial.println(response);
    }
  }

  http.end();
}

void trimString(char* str) {
  if (!str) return;

  // Trim leading whitespace
  char* start = str;
  while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
    start++;
  }

  // Move trimmed string to beginning
  if (start != str) {
    size_t len = strlen(start);
    memmove(str, start, len + 1);
  }

  // Trim trailing whitespace
  char* end = str + strlen(str) - 1;
  while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
    *end = '\0';
    end--;
  }
}