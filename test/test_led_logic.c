#include "unity.h"
#include "led_logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_get_blink_delay_ms_hot_temperature(void) {
    TEST_ASSERT_EQUAL(100, get_blink_delay_ms(35));
}

void test_get_blink_delay_ms_cold_temperature(void) {
    TEST_ASSERT_EQUAL(500, get_blink_delay_ms(20));
}

void test_get_blink_delay_ms_boundary(void) {
    TEST_ASSERT_EQUAL(500, get_blink_delay_ms(30));
}

