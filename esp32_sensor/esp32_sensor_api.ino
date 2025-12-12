#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===== CONFIGURATION =====
const char* ssid = "bruh";
const char* password = "megabruh";

// Your Next.js development server URL
// For local development: http://192.168.x.x:3000 (your computer's IP)
// For production: https://your-domain.com
const char* apiUrl = "http://192.168.137.1:3000/api/sensor-data";

// Sensor configuration
const char* deviceId = "ESP32_SENSOR_001";
const char* sensorType = "soil"; // soil, air, water, weather, industrial
const char* location = "Greenhouse A";

// Data generation settings
unsigned long sendInterval = 60000; // 60 seconds
unsigned long lastSendTime = 0;

// ===== HELPER FUNCTIONS =====
float randomFloat(float min, float max) {
  return min + static_cast<float>(random(0, 1000)) / 1000.0 * (max - min);
}

void generateAndSendData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Attempting to reconnect...");
    WiFi.reconnect();
    delay(1000);
    return;
  }

  // Generate random sensor data
  float temperature = randomFloat(20.0, 30.0);      // 20-30°C
  float humidity = randomFloat(40.0, 80.0);         // 40-80%
  int ec = random(500, 1500);                       // 500-1500 µS/cm
  float ph = randomFloat(6.0, 7.5);                 // 6.0-7.5 pH

  // Create JSON document
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["ec"] = ec;
  doc["ph"] = ph;
  doc["deviceId"] = deviceId;
  doc["sensorType"] = sensorType;
  doc["location"] = location;

  // Serialize JSON
  String jsonString;
  serializeJson(doc, jsonString);

  // Print to Serial
  Serial.println("\n=== Generated Sensor Data ===");
  Serial.println("Temperature: " + String(temperature) + "°C");
  Serial.println("Humidity: " + String(humidity) + "%");
  Serial.println("EC: " + String(ec) + " µS/cm");
  Serial.println("pH: " + String(ph));
  Serial.println("JSON: " + jsonString);

  // Send HTTP POST request
  HTTPClient http;
  http.begin(apiUrl);
  http.addHeader("Content-Type", "application/json");

  Serial.println("Sending to: " + String(apiUrl));
  int httpCode = http.POST(jsonString);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Response Code: " + String(httpCode));
    Serial.println("Response: " + response);

    // Parse response
    StaticJsonDocument<256> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);

    if (!error) {
      bool success = responseDoc["success"];
      if (success) {
        Serial.println("✅ Data stored on Sui!");
        Serial.println("TX Digest: " + String(responseDoc["transactionDigest"].as<const char*>()));
      } else {
        Serial.println("❌ Error: " + String(responseDoc["error"].as<const char*>()));
      }
    }
  } else {
    Serial.println("HTTP Error: " + String(httpCode));
  }

  http.end();
}

// ===== SETUP & LOOP =====
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32 Sui Sensor Data Sender ===");
  
  // Connect to WiFi
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("API Endpoint: ");
    Serial.println(apiUrl);
  } else {
    Serial.println("\n❌ WiFi Connection Failed!");
  }

  // Initialize random seed
  randomSeed(analogRead(0));
}

void loop() {
  unsigned long currentTime = millis();

  // Check if it's time to send data
  if (currentTime - lastSendTime >= sendInterval) {
    generateAndSendData();
    lastSendTime = currentTime;
  }

  // Optional: Add manual trigger via Serial
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "send") {
      generateAndSendData();
      lastSendTime = currentTime;
    } else if (command == "interval") {
      Serial.print("Current interval: ");
      Serial.print(sendInterval / 1000);
      Serial.println(" seconds");
      Serial.println("Enter new interval in seconds:");
      
      while (!Serial.available());
      String newInterval = Serial.readStringUntil('\n');
      sendInterval = newInterval.toInt() * 1000;
      Serial.print("Interval set to: ");
      Serial.print(sendInterval / 1000);
      Serial.println(" seconds");
    }
  }

  delay(100);
}