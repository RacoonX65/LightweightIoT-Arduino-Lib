#include <unity.h>
#include "LightweightIoT.h"

LightweightIoT* iot;

void setUp(void) {
    iot = new LightweightIoT("test_token", "test_org", "test_bucket");
}

void tearDown(void) {
    delete iot;
}

void test_location_validation(void) {
    LightweightIoT::Location location("Building-A", "Floor-1", "Room-101", "Zone-1");
    TEST_ASSERT_TRUE(location.isValid());

    // Test invalid location (empty building)
    LightweightIoT::Location invalidLocation("", "Floor-1", "Room-101", "Zone-1");
    TEST_ASSERT_FALSE(invalidLocation.isValid());
}

void test_device_validation(void) {
    LightweightIoT::Location location("Building-A", "Floor-1", "Room-101", "Zone-1");
    LightweightIoT::Device device("device-001", location, "sensor", "Test device");
    TEST_ASSERT_TRUE(device.isValid());

    // Test invalid device (empty id)
    LightweightIoT::Device invalidDevice("", location, "sensor", "Test device");
    TEST_ASSERT_FALSE(invalidDevice.isValid());
}

void test_measurement_validation(void) {
    LightweightIoT::Measurement measurement("temperature", "value", "23.5");
    TEST_ASSERT_TRUE(measurement.isValid());

    // Test invalid measurement (empty name)
    LightweightIoT::Measurement invalidMeasurement("", "value", "23.5");
    TEST_ASSERT_FALSE(invalidMeasurement.isValid());
}

void test_memory_check(void) {
    size_t memory = iot->checkMemory();
    TEST_ASSERT_GREATER_THAN(0, memory);
}

void test_batch_memory(void) {
    // Fill batch to test memory management
    iot->beginBatch();
    for (int i = 0; i < 50; i++) {
        bool result = iot->writePoint("test", "value", i);
        if (i < LightweightIoT::MAX_BATCH_SIZE) {
            TEST_ASSERT_TRUE(result);
        } else {
            TEST_ASSERT_FALSE(result);
        }
    }
    iot->clearBatch();
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_location_validation);
    RUN_TEST(test_device_validation);
    RUN_TEST(test_measurement_validation);
    RUN_TEST(test_memory_check);
    RUN_TEST(test_batch_memory);
    UNITY_END();
}

void loop() {}