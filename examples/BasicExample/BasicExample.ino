/*
 * BasicExample
 * Created by Judas Sithole
 * 
 * This example shows the basic usage of the LightweightIoT library with error handling
 * and WiFi connection management. It demonstrates both single point and batch data
 * writing to InfluxDB using an analog sensor.
 * 
 * Hardware Required:
 * - ESP32 or compatible board
 * - Analog sensor connected to pin 34
 * 
 * Circuit:
 * - Connect analog sensor to pin 34
 */

#include <WiFi.h>
#include <LightweightIoT.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// InfluxDB credentials
const char* influxToken = "YOUR_INFLUXDB_TOKEN";
const char* influxOrg = "YOUR_ORGANIZATION";
const char* influxBucket = "YOUR_BUCKET";

// Create IoT client
LightweightIoT iot(influxToken, influxOrg, influxBucket);

// Example sensor pin
const int SENSOR_PIN = 34;  // ADC pin on ESP32

void setup() {
    Serial.begin(115200);
    
    // Configure IoT client
    LightweightIoT::Config config;
    config.maxRetries = 3;
    config.retryDelay = 1000;
    config.timeout = 5000;
    config.debugMode = true;
    iot.setConfig(config);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    
    // Initialize IoT client
    if (!iot.begin()) {
        Serial.print("Failed to initialize IoT client: ");
        Serial.println(iot.getLastErrorMessage());
        return;
    }
    
    // Add some tags
    iot.addTag("device", "esp32");
    iot.addTag("location", "room1");
}

void loop() {
    if (!iot.isConnected()) {
        Serial.println("WiFi disconnected. Reconnecting...");
        WiFi.begin(ssid, password);
        delay(5000);
        return;
    }
    
    // Read sensor value
    int sensorValue = analogRead(SENSOR_PIN);
    float voltage = sensorValue * (3.3 / 4095.0);
    
    // Send data with error handling
    if (!iot.writePoint("sensors", "voltage", voltage)) {
        Serial.print("Error sending voltage: ");
        Serial.println(iot.getLastErrorMessage());
    }
    
    // Try batch writing with error handling
    iot.beginBatch();
    for (int i = 0; i < 5; i++) {
        if (!iot.writePoint("sensors", "reading_" + String(i), analogRead(SENSOR_PIN))) {
            Serial.print("Error adding to batch: ");
            Serial.println(iot.getLastErrorMessage());
            break;
        }
        delay(100);
    }
    
    if (!iot.endBatch()) {
        Serial.print("Error sending batch: ");
        Serial.println(iot.getLastErrorMessage());
    }
    
    delay(5000);
}