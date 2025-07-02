/*
 * BasicMultiSensor Example
 * Created by Judas Sithole
 * 
 * This example shows how to use multiple sensors with different tags for each sensor.
 * It demonstrates reading from a DHT22 temperature/humidity sensor and a BH1750
 * light sensor, sending data to InfluxDB with appropriate location tags.
 * 
 * Hardware Required:
 * - ESP32 or compatible board
 * - DHT22 temperature/humidity sensor
 * - BH1750 light sensor
 * 
 * Circuit:
 * - Connect DHT22 to pin 2
 * - Connect BH1750 to I2C pins
 */


#include <LightweightIoT.h>
#include <DHT.h>
#include <BH1750.h>

// WiFi and InfluxDB settings
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";
const char* INFLUXDB_URL = "http://your-influxdb:8086";
const char* INFLUXDB_TOKEN = "your-token";
const char* INFLUXDB_ORG = "your-org";
const char* INFLUXDB_BUCKET = "your-bucket";

// Sensors
DHT dht(2, DHT22);
BH1750 lightMeter;

LightweightIoT iot;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin();
  lightMeter.begin();
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  iot.setConfig(INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG, INFLUXDB_BUCKET);
  iot.begin();
}

void loop() {
  // Read DHT22 (indoor)
  iot.clearTags();
  iot.addTag("device", "dht22");
  iot.addTag("location", "indoor");
  iot.addTag("room", "living-room");
  
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (!isnan(temp) && !isnan(hum)) {
    iot.writePoint("temperature", temp);
    iot.writePoint("humidity", hum);
  }
  
  // Read Light Sensor (outdoor)
  iot.clearTags();
  iot.addTag("device", "bh1750");
  iot.addTag("location", "outdoor");
  iot.addTag("area", "garden");
  
  float lux = lightMeter.readLightLevel();
  if (lux >= 0) {
    iot.writePoint("light", lux);
  }
  
  delay(60000); // Read every minute
}