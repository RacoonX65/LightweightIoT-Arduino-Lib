#include <WiFi.h>
#include <LightweightIoT.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// InfluxDB credentials
const char* influxToken = "YOUR_INFLUXDB_TOKEN";
const char* influxOrg = "YOUR_ORGANIZATION";
const char* influxBucket = "YOUR_BUCKET";

// Sensor pins
const int TEMP_SENSOR_PIN = 34;    // Temperature sensor
const int HUMID_SENSOR_PIN = 35;   // Humidity sensor
const int LIGHT_SENSOR_PIN = 36;   // Light sensor

// Create IoT client
LightweightIoT iot(influxToken, influxOrg, influxBucket);

// Device configurations
LightweightIoT::Location location1("Building-A", "Floor-1", "Room-101", "Zone-1");
LightweightIoT::Location location2("Building-A", "Floor-1", "Room-102", "Zone-1");

LightweightIoT::Device tempDevice("temp-001", location1, "temperature", "Temperature sensor");
LightweightIoT::Device humidDevice("humid-001", location1, "humidity", "Humidity sensor");
LightweightIoT::Device lightDevice("light-001", location2, "light", "Light sensor");

void setup() {
    Serial.begin(115200);

    // Configure IoT client with power saving
    LightweightIoT::Config config;
    config.maxRetries = 3;
    config.retryDelay = 1000;
    config.timeout = 5000;
    config.debugMode = true;
    config.useLowPowerMode = true;
    config.deepSleepDuration = 60000; // 1 minute
    iot.setConfig(config);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // Validate credentials
    if (!iot.validateCredentials()) {
        Serial.println("Invalid InfluxDB credentials!");
        return;
    }

    // Initialize IoT client
    if (!iot.begin()) {
        Serial.println("Failed to initialize IoT client!");
        return;
    }

    // Check memory
    size_t availableMemory = iot.checkMemory();
    Serial.printf("Available memory: %d bytes\n", availableMemory);
}

void loop() {
    if (!iot.isConnected()) {
        Serial.println("WiFi disconnected. Reconnecting...");
        WiFi.begin(ssid, password);
        delay(5000);
        return;
    }

    // Read sensors
    float temperature = readTemperature();
    float humidity = readHumidity();
    float light = readLight();

    // Create measurements
    LightweightIoT::Measurement measurements[] = {
        {"temperature", "value", String(temperature)},
        {"humidity", "value", String(humidity)},
        {"light", "value", String(light)}
    };

    // Send data from each device
    iot.setDevice(tempDevice);
    if (!iot.writeMeasurement(measurements[0])) {
        Serial.println("Error sending temperature data!");
    }

    iot.setDevice(humidDevice);
    if (!iot.writeMeasurement(measurements[1])) {
        Serial.println("Error sending humidity data!");
    }

    iot.setDevice(lightDevice);
    if (!iot.writeMeasurement(measurements[2])) {
        Serial.println("Error sending light data!");
    }

    // Use power management
    iot.managePower();
}

// Sensor reading functions
float readTemperature() {
    int rawValue = analogRead(TEMP_SENSOR_PIN);
    return rawValue * (3.3 / 4095.0) * 100; // Convert to Celsius
}

float readHumidity() {
    int rawValue = analogRead(HUMID_SENSOR_PIN);
    return rawValue * (100.0 / 4095.0); // Convert to percentage
}

float readLight() {
    int rawValue = analogRead(LIGHT_SENSOR_PIN);
    return rawValue * (100.0 / 4095.0); // Convert to percentage
}