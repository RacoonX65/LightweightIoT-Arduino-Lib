#include <LightweightIoT.h>
#include <DHT.h>

// WiFi and InfluxDB settings
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";
const char* INFLUXDB_URL = "http://your-influxdb:8086";
const char* INFLUXDB_TOKEN = "your-token";
const char* INFLUXDB_ORG = "your-org";
const char* INFLUXDB_BUCKET = "your-bucket";

// DHT sensor settings
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Create IoT client
LightweightIoT iot;

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  // Configure IoT client
  iot.setConfig(INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG, INFLUXDB_BUCKET);
  iot.begin();
  
  // Add basic tags
  iot.addTag("device", "dht22-sensor");
  iot.addTag("location", "living-room");
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (!isnan(temp) && !isnan(hum)) {
    // Write temperature and humidity
    iot.writePoint("temperature", temp);
    iot.writePoint("humidity", hum);
  }
  
  delay(60000); // Read every minute
}