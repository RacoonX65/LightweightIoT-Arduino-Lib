#ifndef LIGHTWEIGHT_IOT_H
#define LIGHTWEIGHT_IOT_H

#ifdef ARDUINO
    #include <Arduino.h>
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <ArduinoJson.h>
    using String = ::String;
#else
    #include <string>
    #include <functional>
    using String = std::string;
#endif

/**
 * @brief A lightweight IoT library for sending data to InfluxDB Cloud
 * 
 * This library provides an easy way to send sensor data to InfluxDB Cloud
 * from Arduino devices. It supports:
 * - Hierarchical location tracking
 * - Multiple sensor management
 * - Batch operations
 * - Error handling and retry logic
 * - Memory management
 */
class LightweightIoT {
public:
    /**
     * @brief Error codes for operation status
     */
    enum ErrorCode {
        NO_ERROR = 0,        ///< Operation completed successfully
        NOT_CONNECTED = 1,   ///< WiFi or network connection failed
        HTTP_ERROR = 2,      ///< HTTP request failed
        BATCH_FULL = 3,      ///< Batch buffer is full
        INVALID_DATA = 4,    ///< Invalid data format or value
        TIMEOUT = 5,         ///< Operation timed out
        MEMORY_ERROR = 6,    ///< Memory allocation failed
        INVALID_CONFIG = 7,  ///< Invalid configuration
        AUTH_ERROR = 8       ///< Authentication failed
    };

    /**
     * @brief Configuration options for the IoT client
     */
    struct Config {
        uint8_t maxRetries = 3;         ///< Maximum number of retry attempts
        uint16_t retryDelay = 1000;     ///< Delay between retries (ms)
        uint16_t timeout = 5000;        ///< Operation timeout (ms)
        bool debugMode = false;         ///< Enable debug output
        uint16_t reconnectDelay = 5000; ///< Delay before reconnection attempt (ms)
        bool autoReconnect = true;      ///< Automatically attempt reconnection
        size_t maxPointSize = 1024;     ///< Maximum size of a single point (bytes)
        bool useStaticBuffer = false;   ///< Use pre-allocated buffer
        size_t staticBufferSize = 2048; ///< Static buffer size (bytes)
        bool useLowPowerMode = false;   ///< Enable power saving features
        uint32_t deepSleepDuration = 0; ///< Deep sleep duration (ms, 0 = disabled)
    };

    /**
     * @brief Location structure for hierarchical organization
     */
    struct Location {
        String building;    ///< Building identifier
        String floor;      ///< Floor number/identifier
        String room;       ///< Room number/identifier
        String zone;       ///< Zone within the room

        Location(String b = "", String f = "", String r = "", String z = "") 
            : building(b), floor(f), room(r), zone(z) {}

        /**
         * @brief Validates the location configuration
         * @return true if valid, false otherwise
         */
        bool isValid() const {
            return !building.isEmpty() && building.length() <= 64 &&
                   floor.length() <= 32 &&
                   room.length() <= 32 &&
                   zone.length() <= 32;
        }

        String getPath() const;
    };

    /**
     * @brief Device structure with location and metadata
     */
    struct Device {
        String id;           ///< Unique device identifier
        Location location;   ///< Hierarchical location
        String type;        ///< Device type
        String description; ///< Optional description

        Device(String did = "", Location loc = Location(), String t = "", String desc = "") 
            : id(did), location(loc), type(t), description(desc) {}

        /**
         * @brief Validates the device configuration
         * @return true if valid, false otherwise
         */
        bool isValid() const {
            return !id.isEmpty() && id.length() <= 64 &&
                   !type.isEmpty() && type.length() <= 32 &&
                   description.length() <= 128 &&
                   location.isValid();
        }
    };

    /**
     * @brief Measurement structure for data points
     */
    struct Measurement {
        String name;         ///< Measurement name
        String field;        ///< Field name
        String value;        ///< The actual value
        unsigned long time;  ///< Timestamp

        Measurement(String n, String f, String v, unsigned long t = 0) 
            : name(n), field(f), value(v), time(t) {}

        /**
         * @brief Validates the measurement data
         * @return true if valid, false otherwise
         */
        bool isValid() const {
            return !name.isEmpty() && name.length() <= 64 &&
                   !field.isEmpty() && field.length() <= 32 &&
                   !value.isEmpty() && value.length() <= 64;
        }
    };

    // Constructor and basic methods
    LightweightIoT(String token, String org, String bucket);

    /**
     * @brief Validates InfluxDB credentials
     * @return true if credentials are valid, false otherwise
     */
    bool validateCredentials();

    /**
     * @brief Checks available memory and warns if low
     * @return Available memory in bytes
     */
    size_t checkMemory();

    /**
     * @brief Enables power saving features
     * @param duration Deep sleep duration in milliseconds
     */
    void enablePowerSaving(uint32_t duration);

    size_t getPointSize(String measurement, String field, String value);
    bool reserveBuffer(size_t size);
    void freeBuffer();
    bool reconnect();
    void setAutoReconnect(bool enabled);
    bool getAutoReconnect() const { return config.autoReconnect; }
    
private:
    String token;
    String org;
    String bucket;
    String url;
    Config config;
    ErrorCode lastError;
    String lastErrorMessage;
    
    // Tag storage
    struct Tag {
        String key;
        String value;
    };
    static const int MAX_TAGS = 10;
    Tag tags[MAX_TAGS];
    int tagCount;
    
    // Batch storage
    static const int MAX_BATCH_SIZE = 50;
    String batchBuffer[MAX_BATCH_SIZE];
    int batchCount;
    bool batchMode;
    
    // Helper methods
    String formatLineProtocol(String measurement, String field, String value);
    String formatLineProtocol(String measurement, String field, float value);
    String formatLineProtocol(String measurement, String field, int value);
    bool sendToInfluxDB(String lineProtocol);
    bool addToBatch(String lineProtocol);
    void setError(ErrorCode code, String message);
    bool retryOperation(std::function<bool()> operation);
    
public:
    LightweightIoT(String token, String org, String bucket);
    
    // Configuration
    void setConfig(Config config);
    Config getConfig() const { return config; }
    
    // Error handling
    ErrorCode getLastError() const { return lastError; }
    String getLastErrorMessage() const { return lastErrorMessage; }
    void clearError() { lastError = NO_ERROR; lastErrorMessage = ""; }
    
    // Connection methods
    bool begin(String influxUrl = "https://cloud2.influxdata.com");
    bool isConnected();
    
    // Data methods
    bool writePoint(String measurement, String field, float value);
    bool writePoint(String measurement, String field, int value);
    bool writePoint(String measurement, String field, String value);
    
    // Tag methods
    bool addTag(String key, String value);
    void clearTags();
    
    // Batch methods
    void beginBatch();
    bool endBatch();
    void clearBatch();
    bool flushBatch();
    int getBatchSize() { return batchCount; }
};

#endif

// Add these new methods to the public section
public:
    // Multiple fields in one point
    bool writePoint(String measurement, std::initializer_list<std::pair<String, float>> fields);
    bool writePoint(String measurement, std::initializer_list<std::pair<String, int>> fields);
    bool writePoint(String measurement, std::initializer_list<std::pair<String, String>> fields);
    
    // Timestamp control
    bool writePoint(String measurement, String field, float value, unsigned long timestamp);
    bool writePoint(String measurement, String field, int value, unsigned long timestamp);
    bool writePoint(String measurement, String field, String value, unsigned long timestamp);
    
private:
    bool validateMeasurement(String measurement);
    bool validateField(String field);
    bool validateValue(String value);
    bool validateTag(String key, String value);
    // Timestamp control
    bool writePoint(String measurement, String field, float value, unsigned long timestamp);
    bool writePoint(String measurement, String field, int value, unsigned long timestamp);
    bool writePoint(String measurement, String field, String value, unsigned long timestamp);
    
public:
    enum LogLevel {
        LOG_NONE = 0,
        LOG_ERROR = 1,
        LOG_WARN = 2,
        LOG_INFO = 3,
        LOG_DEBUG = 4
    };
    
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const { return logLevel; }
    void setLogCallback(void (*callback)(LogLevel level, const char* message));

private:
    LogLevel logLevel = LOG_ERROR;
    void (*logCallback)(LogLevel level, const char* message) = nullptr;
    void log(LogLevel level, const char* format, ...);

public:
    class LightweightIoT {
    public:
        // Device and Location structure
        struct Device {
            String id;           // Unique device identifier
            String location;     // Location identifier (e.g., "building_A")
            String description;  // Optional description
            
            Device(String did = "", String loc = "", String desc = "") 
                : id(did), location(loc), description(desc) {}
        };
    
        // Time series settings
        enum TimeUnit {
            SECONDS,
            MILLISECONDS,
            MICROSECONDS,
            NANOSECONDS
        };
    
        // Measurement structure for better organization
        struct Measurement {
            String name;         // Measurement name (e.g., "temperature")
            String field;        // Field name (e.g., "value")
            String value;        // The actual value
            unsigned long time;  // Timestamp
            TimeUnit unit;       // Time unit
            
            Measurement(String n, String f, String v, unsigned long t = 0, TimeUnit u = MILLISECONDS)
                : name(n), field(f), value(v), time(t), unit(u) {}
        };
    
        // New methods for better data organization
        void setDevice(const Device& device);
        Device getDevice() const { return currentDevice; }
        void setTimeUnit(TimeUnit unit) { timeUnit = unit; }
        
        // Enhanced write methods
        bool writeMeasurement(const Measurement& measurement);
        bool writeMeasurements(const Measurement* measurements, size_t count);
    
        // Helper method to get current timestamp in the configured unit
        unsigned long getCurrentTimestamp();
    
    private:
        Device currentDevice;
        TimeUnit timeUnit = MILLISECONDS;
        String formatTimestamp(unsigned long timestamp, TimeUnit unit);
    
    /**
     * @brief Validates HTTPS certificate
     * @return true if valid, false otherwise
     */
    bool validateCertificate();

    /**
     * @brief Manages power state
     */
    void managePower();
};

#endif

// Add these new methods to the public section
public:
    // Multiple fields in one point
    bool writePoint(String measurement, std::initializer_list<std::pair<String, float>> fields);
    bool writePoint(String measurement, std::initializer_list<std::pair<String, int>> fields);
    bool writePoint(String measurement, std::initializer_list<std::pair<String, String>> fields);
    
    // Timestamp control
    bool writePoint(String measurement, String field, float value, unsigned long timestamp);
    bool writePoint(String measurement, String field, int value, unsigned long timestamp);
    bool writePoint(String measurement, String field, String value, unsigned long timestamp);
    
private:
    bool validateMeasurement(String measurement);
    bool validateField(String field);
    bool validateValue(String value);
    bool validateTag(String key, String value);
    // Timestamp control
    bool writePoint(String measurement, String field, float value, unsigned long timestamp);
    bool writePoint(String measurement, String field, int value, unsigned long timestamp);
    bool writePoint(String measurement, String field, String value, unsigned long timestamp);
    
public:
    enum LogLevel {
        LOG_NONE = 0,
        LOG_ERROR = 1,
        LOG_WARN = 2,
        LOG_INFO = 3,
        LOG_DEBUG = 4
    };
    
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const { return logLevel; }
    void setLogCallback(void (*callback)(LogLevel level, const char* message));

private:
    LogLevel logLevel = LOG_ERROR;
    void (*logCallback)(LogLevel level, const char* message) = nullptr;
    void log(LogLevel level, const char* format, ...);

public:
    class LightweightIoT {
    public:
        // Device and Location structure
        struct Device {
            String id;           // Unique device identifier
            String location;     // Location identifier (e.g., "building_A")
            String description;  // Optional description
            
            Device(String did = "", String loc = "", String desc = "") 
                : id(did), location(loc), description(desc) {}
        };
    
        // Time series settings
        enum TimeUnit {
            SECONDS,
            MILLISECONDS,
            MICROSECONDS,
            NANOSECONDS
        };
    
        // Measurement structure for better organization
        struct Measurement {
            String name;         // Measurement name (e.g., "temperature")
            String field;        // Field name (e.g., "value")
            String value;        // The actual value
            unsigned long time;  // Timestamp
            TimeUnit unit;       // Time unit
            
            Measurement(String n, String f, String v, unsigned long t = 0, TimeUnit u = MILLISECONDS)
                : name(n), field(f), value(v), time(t), unit(u) {}
        };
    
        // New methods for better data organization
        void setDevice(const Device& device);
        Device getDevice() const { return currentDevice; }
        void setTimeUnit(TimeUnit unit) { timeUnit = unit; }
        
        // Enhanced write methods
        bool writeMeasurement(const Measurement& measurement);
        bool writeMeasurements(const Measurement* measurements, size_t count);
    
        // Helper method to get current timestamp in the configured unit
        unsigned long getCurrentTimestamp();
    
    private:
        Device currentDevice;
        TimeUnit timeUnit = MILLISECONDS;
        String formatTimestamp(unsigned long timestamp, TimeUnit unit);
    