
# LightweightIoT Arduino Library

[![Arduino Library Manager](https://www.ardu-badge.com/badge/LightweightIoT.svg)](https://www.ardu-badge.com/LightweightIoT)
[![Arduino Library Specification](https://img.shields.io/badge/Arduino%20Library%20Specification-1.5-blue)](https://arduino.github.io/arduino-cli/latest/library-specification/)
[![Arduino Library CI](https://github.com/RacoonX65/LightweightIoT-Arduino-Lib/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/RacoonX65/LightweightIoT-Arduino-Lib/actions)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/RacoonX65/LightweightIoT-Arduino-Lib)](https://github.com/RacoonX65/LightweightIoT-Arduino-Lib/releases)

A lightweight Arduino library for sending sensor data to InfluxDB Cloud. This library provides an easy-to-use interface for IoT data collection and transmission, with support for multiple sensors, batch operations, and power management features.

## Table of Contents
- [Features](#features)
- [Installation](#installation)
- [Dependencies](#dependencies)
- [Basic Usage](#basic-usage)
- [Examples](#examples)
- [Advanced Features](#advanced-features)
- [Compatibility](#compatibility)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [Author](#author)
- [Support](#support)

## Features

- Simple API for sending data to InfluxDB
- Support for multiple sensors
- Batch operations for efficient data transmission
- Power management capabilities
- Built-in error handling and retry logic
- HTTPS secure communication
- Compatible with ESP8266 and ESP32 boards

## Installation

### Using Arduino Library Manager
1. Open Arduino IDE
2. Go to Sketch -> Include Library -> Manage Libraries
3. Search for "LightweightIoT"
4. Click Install

### Manual Installation
1. Download this repository as ZIP
2. Open Arduino IDE
3. Go to Sketch -> Include Library -> Add .ZIP Library
4. Select the downloaded ZIP file

## Dependencies

- ArduinoJson (>=6.0.0)
- ESP8266 or ESP32 board package (includes WiFi and HTTPClient)

## Basic Usage

```cpp
#include <LightweightIoT.h>

// InfluxDB configuration
const char* INFLUXDB_URL = "your-influxdb-url";
const char* INFLUXDB_TOKEN = "your-token";
const char* INFLUXDB_ORG = "your-org";
const char* INFLUXDB_BUCKET = "your-bucket";

// WiFi configuration
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";

// Create IoT client
LightweightIoT iot;

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    // Configure and begin
    iot.setConfig(INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG, INFLUXDB_BUCKET);
    iot.begin();
    
    // Add tags
    iot.addTag("device", "sensor1");
    iot.addTag("location", "room1");
}

void loop() {
    // Read sensor data
    float temperature = 23.5; // Replace with actual sensor reading
    
    // Send data
    if (iot.writePoint("temperature", temperature)) {
        Serial.println("Data sent successfully");
    } else {
        Serial.println("Failed to send data");
    }
    
    delay(60000); // Wait for 1 minute
}
```

## Examples

The library includes several example sketches demonstrating different features:

1. **BasicExample**: Simple data transmission
2. **BasicTemperature**: Temperature monitoring
3. **BasicBatch**: Batch data collection
4. **BasicMultiSensor**: Multiple sensor handling
5. **EnergyMonitoring**: Power consumption monitoring
6. **EnvironmentalMonitoring**: Environmental data with power saving
7. **MultiSensorExample**: Advanced sensor management

Find these examples in the Arduino IDE under File -> Examples -> LightweightIoT

## Advanced Features

### Batch Operations

```cpp
iot.beginBatch();
for (int i = 0; i < 5; i++) {
    float temperature = readTemperature(); // Your sensor reading function
    iot.addToBatch("temperature", temperature);
    delay(1000);
}
iot.endBatch();
```

### Error Handling

```cpp
if (!iot.writePoint("temperature", temperature)) {
    Serial.print("Error: ");
    Serial.println(iot.getLastErrorMessage());
}
```

### Power Management

```cpp
// Enable low power mode with 5-minute deep sleep
iot.setDeepSleepDuration(300000);
iot.enableLowPowerMode(true);
```

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This library is released under the MIT License. See the LICENSE file for details.

## Author

Judas Sithole (judassithole@duck.com)

## Support

For issues and feature requests, please create an issue in the GitHub repository:
https://github.com/RacoonX65/LightweightIoT-Arduino-Lib

## Compatibility

This library has been tested with:
- ESP8266 boards (NodeMCU, Wemos D1 Mini)
- ESP32 boards (ESP32 DevKit, ESP32-WROOM)
- Arduino IDE 1.8.x and 2.x

## Troubleshooting

### Common Issues
1. **Connection Failed**: Ensure your WiFi credentials and InfluxDB configuration are correct
2. **Data Not Appearing**: Verify your bucket and organization settings in InfluxDB
3. **Compilation Errors**: Make sure you have the correct board package installed

For more detailed troubleshooting, check the [Issues](https://github.com/RacoonX65/LightweightIoT-Arduino-Lib/issues) page.

