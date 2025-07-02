#include <WiFi.h>
#include <LightweightIoT.h>
#include <PZEM004Tv30.h>

// WiFi and InfluxDB settings
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* influxToken = "YOUR_INFLUXDB_TOKEN";
const char* influxOrg = "YOUR_ORGANIZATION";
const char* influxBucket = "YOUR_BUCKET";

// PZEM-004T sensor (Software Serial)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
PZEM004Tv30 pzem(PZEM_RX_PIN, PZEM_TX_PIN);

// Create IoT client
LightweightIoT iot(influxToken, influxOrg, influxBucket);

// Device configurations
LightweightIoT::Location location("Home", "Floor-1", "Utility", "MainPanel");
LightweightIoT::Device device("energy-001", location, "power_meter", "Main electrical panel meter");

void setup() {
    Serial.begin(115200);
    
    // Configure IoT client
    LightweightIoT::Config config;
    config.maxRetries = 3;
    config.debugMode = true;
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
    // Read energy metrics
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();
    
    // Create measurements
    LightweightIoT::Measurement measurements[] = {
        {"voltage", "value", String(voltage)},
        {"current", "value", String(current)},
        {"power", "value", String(power)},
        {"energy", "value", String(energy)},
        {"frequency", "value", String(frequency)},
        {"power_factor", "value", String(pf)}
    };
    
    // Send data in batch
    if (!iot.writeMeasurements(measurements, 6)) {
        Serial.println("Error sending data!");
    }
    
    delay(10000); // Read every 10 seconds
}