#include <WiFi.h>
#include <LightweightIoT.h>
#include <DHT.h>
#include <BMP280.h>

// WiFi and InfluxDB settings
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* influxToken = "YOUR_INFLUXDB_TOKEN";
const char* influxOrg = "YOUR_ORGANIZATION";
const char* influxBucket = "YOUR_BUCKET";

// Sensor pins
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define BMP_SDA 21
#define BMP_SCL 22

// Initialize sensors
DHT dht(DHT_PIN, DHT_TYPE);
BMP280 bmp;

// Create IoT client
LightweightIoT iot(influxToken, influxOrg, influxBucket);

// Device configuration
LightweightIoT::Location location("Office", "Floor-3", "Room-305", "Window");
LightweightIoT::Device device("env-sensor-001", location, "environmental", "Window-side environmental sensor");

void setup() {
    Serial.begin(115200);
    
    // Initialize sensors
    dht.begin();
    Wire.begin(BMP_SDA, BMP_SCL);
    bmp.begin();
    
    // Configure IoT client with power saving
    LightweightIoT::Config config;
    config.maxRetries = 3;
    config.useLowPowerMode = true;
    config.deepSleepDuration = 300000; // 5 minutes
    iot.setConfig(config);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    
    // Set device
    iot.setDevice(device);
    
    // Initialize IoT client
    if (!iot.begin()) {
        Serial.println("Failed to initialize!");
        return;
    }
}

void loop() {
    // Read sensors
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float pressure = bmp.readPressure() / 100.0F; // Convert to hPa
    
    // Create measurements
    LightweightIoT::Measurement measurements[] = {
        {"temperature", "value", String(temperature)},
        {"humidity", "value", String(humidity)},
        {"pressure", "value", String(pressure)}
    };
    
    // Send data
    if (!iot.writeMeasurements(measurements, 3)) {
        Serial.println("Error sending data!");
    }
    
    // Use power management for deep sleep
    iot.managePower();
}