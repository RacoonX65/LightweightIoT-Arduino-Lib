#include "LightweightIoT.h"

// Helper method to escape special characters in measurement names and string values
String escapeString(String str) {
#ifdef ARDUINO
    str.replace(" ", "\ ");
    str.replace(",", "\\,");
    str.replace("=", "\\=");
#else
    size_t pos = 0;
    while ((pos = str.find(" ", pos)) != String::npos) {
        str.replace(pos, 1, "\ ");
        pos += 2;
    }
    pos = 0;
    while ((pos = str.find(",", pos)) != String::npos) {
        str.replace(pos, 1, "\\,");
        pos += 2;
    }
    pos = 0;
    while ((pos = str.find("=", pos)) != String::npos) {
        str.replace(pos, 1, "\\=");
        pos += 2;
    }
#endif
    return str;
}

LightweightIoT::LightweightIoT(String token, String org, String bucket) {
    this->token = token;
    this->org = org;
    this->bucket = bucket;
    this->tagCount = 0;
    this->batchCount = 0;
    this->batchMode = false;
    this->lastError = NO_ERROR;
}

String LightweightIoT::formatLineProtocol(String measurement, String field, String value) {
    String line = escapeString(measurement);
    
    // Add tags if any exist
    for (int i = 0; i < tagCount; i++) {
        line += "," + escapeString(tags[i].key) + "=" + escapeString(tags[i].value);
    }
    
    // Add field
    line += " " + escapeString(field) + "=\"" + escapeString(value) + "\"";
    
    // Add timestamp in nanoseconds
    line += " " + String(millis() * 1000000);
    
    return line;
}

String LightweightIoT::formatLineProtocol(String measurement, String field, float value) {
    String line = escapeString(measurement);
    
    // Add tags if any exist
    for (int i = 0; i < tagCount; i++) {
        line += "," + escapeString(tags[i].key) + "=" + escapeString(tags[i].value);
    }
    
    // Add field (float values don't need escaping)
    line += " " + escapeString(field) + "=" + String(value);
    
    // Add timestamp in nanoseconds
    line += " " + String(millis() * 1000000);
    
    return line;
}

String LightweightIoT::formatLineProtocol(String measurement, String field, int value) {
    String line = escapeString(measurement);
    
    // Add tags if any exist
    for (int i = 0; i < tagCount; i++) {
        line += "," + escapeString(tags[i].key) + "=" + escapeString(tags[i].value);
    }
    
    // Add field (integer values don't need escaping)
    line += " " + escapeString(field) + "=" + String(value) + "i";
    
    // Add timestamp in nanoseconds
    line += " " + String(millis() * 1000000);
    
    return line;
}

bool LightweightIoT::begin(String influxUrl) {
    this->url = influxUrl + "/api/v2/write?org=" + this->org + "&bucket=" + this->bucket;
    
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Error: WiFi not connected");
        return false;
    }
    
    return true;
}

bool LightweightIoT::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void LightweightIoT::setError(ErrorCode code, String message) {
    lastError = code;
    lastErrorMessage = message;
    if (config.debugMode) {
        Serial.print("Error (");
        Serial.print(code);
        Serial.print("): ");
        Serial.println(message);
    }
}

bool LightweightIoT::retryOperation(std::function<bool()> operation) {
    for (uint8_t attempt = 0; attempt <= config.maxRetries; attempt++) {
        if (attempt > 0) {
            if (config.debugMode) {
                Serial.print("Retry attempt ");
                Serial.print(attempt);
                Serial.print(" of ");
                Serial.println(config.maxRetries);
            }
            delay(config.retryDelay);
        }
        
        if (operation()) {
            return true;
        }
    }
    return false;
}

bool LightweightIoT::sendToInfluxDB(String lineProtocol) {
    if (!isConnected()) {
        setError(NOT_CONNECTED, "WiFi not connected");
        return false;
    }
    
    return retryOperation([this, lineProtocol]() {
        HTTPClient http;
        http.begin(this->url);
        http.setTimeout(config.timeout);
        
        // Set headers
        http.addHeader("Content-Type", "text/plain");
        http.addHeader("Authorization", "Token " + this->token);
        
        // Send POST request
        int httpResponseCode = http.POST(lineProtocol);
        
        // Check response
        bool success = (httpResponseCode >= 200 && httpResponseCode < 300);
        
        if (!success) {
            String error = "HTTP error " + String(httpResponseCode);
            if (http.getString().length() > 0) {
                error += ": " + http.getString();
            }
            setError(HTTP_ERROR, error);
        }
        
        http.end();
        return success;
    });
}

bool LightweightIoT::addToBatch(String lineProtocol) {
    if (batchCount >= MAX_BATCH_SIZE) {
        setError(BATCH_FULL, "Batch buffer is full");
        return false;
    }
    batchBuffer[batchCount++] = lineProtocol;
    return true;
}

void LightweightIoT::setConfig(Config config) {
    this->config = config;
}

void LightweightIoT::beginBatch() {
    batchMode = true;
}

bool LightweightIoT::endBatch() {
    if (!batchMode || batchCount == 0) {
        return false;
    }
    bool result = flushBatch();
    batchMode = false;
    return result;
}

void LightweightIoT::clearBatch() {
    batchCount = 0;
}

bool LightweightIoT::flushBatch() {
    if (batchCount == 0) {
        return true;
    }
    
    // Combine all points with newlines
    String batchData = "";
    for (int i = 0; i < batchCount; i++) {
        if (i > 0) {
            batchData += "\n";
        }
        batchData += batchBuffer[i];
    }
    
    // Send the batch
    bool result = sendToInfluxDB(batchData);
    clearBatch();
    return result;
}

bool LightweightIoT::writePoint(String measurement, String field, float value) {
    clearError();
    String lineProtocol = formatLineProtocol(measurement, field, value);
    if (batchMode) {
        if (!addToBatch(lineProtocol)) {
            setError(BATCH_FULL, "Batch buffer is full");
            return false;
        }
        return true;
    }
    return sendToInfluxDB(lineProtocol);
}

bool LightweightIoT::writePoint(String measurement, String field, int value) {
    String lineProtocol = formatLineProtocol(measurement, field, value);
    if (batchMode) {
        return addToBatch(lineProtocol);
    }
    return sendToInfluxDB(lineProtocol);
}

bool LightweightIoT::writePoint(String measurement, String field, String value) {
    String lineProtocol = formatLineProtocol(measurement, field, value);
    if (batchMode) {
        return addToBatch(lineProtocol);
    }
    return sendToInfluxDB(lineProtocol);
}

bool LightweightIoT::addTag(String key, String value) {
    if (tagCount >= MAX_TAGS) {
        return false;
    }
    
    tags[tagCount].key = key;
    tags[tagCount].value = value;
    tagCount++;
    return true;
}

void LightweightIoT::clearTags() {
    tagCount = 0;
}

void LightweightIoT::setDevice(const Device& device) {
    currentDevice = device;
    clearTags();
    if (device.id.length() > 0) {
        addTag("device", device.id);
    }
    if (device.location.length() > 0) {
        addTag("location", device.location);
    }
}

unsigned long LightweightIoT::getCurrentTimestamp() {
    unsigned long current = millis();
    switch (timeUnit) {
        case SECONDS:
            return current / 1000;
        case MILLISECONDS:
            return current;
        case MICROSECONDS:
            return current * 1000;
        case NANOSECONDS:
            return current * 1000000;
        default:
            return current;
    }
}

String LightweightIoT::formatTimestamp(unsigned long timestamp, TimeUnit unit) {
    switch (unit) {
        case SECONDS:
            return String(timestamp * 1000000000);
        case MILLISECONDS:
            return String(timestamp * 1000000);
        case MICROSECONDS:
            return String(timestamp * 1000);
        case NANOSECONDS:
            return String(timestamp);
        default:
            return String(timestamp * 1000000); // Default to milliseconds
    }
}

bool LightweightIoT::writeMeasurement(const Measurement& measurement) {
    String lineProtocol = escapeString(measurement.name);
    
    // Add tags if any exist
    for (int i = 0; i < tagCount; i++) {
        lineProtocol += "," + escapeString(tags[i].key) + "=" + escapeString(tags[i].value);
    }
    
    // Add field
    lineProtocol += " " + escapeString(measurement.field) + "=\"" + escapeString(measurement.value) + "\"";
    
    // Add timestamp
    unsigned long timestamp = measurement.time > 0 ? measurement.time : getCurrentTimestamp();
    lineProtocol += " " + formatTimestamp(timestamp, measurement.unit);
    
    if (batchMode) {
        return addToBatch(lineProtocol);
    }
    return sendToInfluxDB(lineProtocol);
}

bool LightweightIoT::writeMeasurements(const Measurement* measurements, size_t count) {
    beginBatch();
    for (size_t i = 0; i < count; i++) {
        if (!writeMeasurement(measurements[i])) {
            return false;
        }
    }
    return endBatch();
}

bool LightweightIoT::validateCredentials() {
    if (!isConnected()) {
        setError(NOT_CONNECTED, "WiFi not connected");
        return false;
    }

    HTTPClient http;
    String healthUrl = this->url.substring(0, this->url.indexOf("/write")) + "/health";
    http.begin(healthUrl);
    http.addHeader("Authorization", "Token " + this->token);

    int httpCode = http.GET();
    bool success = (httpCode >= 200 && httpCode < 300);

    if (!success) {
        setError(AUTH_ERROR, "Invalid credentials");
    }

    http.end();
    return success;
}

size_t LightweightIoT::checkMemory() {
#ifdef ARDUINO
    size_t freeHeap = ESP.getFreeHeap();
    if (config.debugMode) {
        Serial.printf("Free heap: %d bytes\n", freeHeap);
    }

    // Warn if memory is low (less than 10KB)
    if (freeHeap < 10240) {
        setError(MEMORY_ERROR, "Low memory: " + String(freeHeap) + " bytes");
    }

    return freeHeap;
#else
    return SIZE_MAX; // Not applicable for non-Arduino platforms
#endif
}

bool LightweightIoT::validateCertificate() {
#ifdef ARDUINO
    HTTPClient http;
    http.begin(this->url, /* CA Certificate */ nullptr);
    
    // Enable certificate validation
    http.setInsecure(false);
    
    int httpCode = http.GET();
    bool success = (httpCode != HTTPC_ERROR_CONNECTION_REFUSED);

    if (!success) {
        setError(HTTP_ERROR, "Certificate validation failed");
    }

    http.end();
    return success;
#else
    return true; // Not applicable for non-Arduino platforms
#endif
}

void LightweightIoT::enablePowerSaving(uint32_t duration) {
    config.useLowPowerMode = true;
    config.deepSleepDuration = duration;
}

void LightweightIoT::managePower() {
#ifdef ARDUINO
    if (config.useLowPowerMode) {
        if (config.debugMode) {
            Serial.println("Entering deep sleep...");
        }

        // Ensure all data is sent before sleep
        if (batchMode) {
            flushBatch();
        }

        // Configure wake-up timer
        esp_sleep_enable_timer_wakeup(config.deepSleepDuration * 1000); // Convert to microseconds

        // Enter deep sleep
        esp_deep_sleep_start();
    }
#endif
}

// Add unit tests in a separate file: test_LightweightIoT.cpp