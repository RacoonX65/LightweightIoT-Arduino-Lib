/*
 * BasicBatch Example
 * Created by Judas Sithole
 * 
 * This example demonstrates how to use batch operations to collect multiple sensor readings
 * and send them to InfluxDB in a single request. It uses a BMP280 sensor to measure
 * temperature and pressure, collecting 5 readings before sending them as a batch.
 * 
 * Hardware Required:
 * - ESP32 or compatible board
 * - BMP280 sensor
 * 
 * Circuit:
 * - Connect BMP280 to your board's I2C pins
 */


#include <LightweightIoT.h>
#include <Adafruit_BMP280.h>

// WiFi and InfluxDB settings
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";
const char* INFLUXDB_URL = "http://your-influxdb:8086";
const char* INFLUXDB_TOKEN = "your-token";
const char* INFLUXDB_ORG = "your-org";
const char* INFLUXDB_BUCKET = "your-bucket";

Adafruit_BMP280 bmp;
LightweightIoT iot;

void setup() {
  Serial.begin(115200);
  
  if (!bmp.begin()) {
    Serial.println("BMP280 not found!");
    while (1);
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  iot.setConfig(INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG, INFLUXDB_BUCKET);
  iot.begin();
  iot.addTag("device", "bmp280-sensor");
  iot.addTag("location", "outdoor");
}

void loop() {
  iot.beginBatch();
  
  // Collect 5 readings
  for (int i = 0; i < 5; i++) {
    float temp = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F; // Convert to hPa
    
    iot.addToBatch("temperature", temp);
    iot.addToBatch("pressure", pressure);
    
    delay(2000); // Wait 2 seconds between readings
  }
  
  // Send batch
  iot.endBatch();
  
  delay(300000); // Wait 5 minutes before next batch
}