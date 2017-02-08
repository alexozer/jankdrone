#pragma once

#include <cstdint>
#include <RFM69.h>
#include <i2c_t3.h>

/* Constants which describe fixed properties of the drone. Although it may be
 * more consistent and convenient to define these as default values in shm,
 * compile-time optimizations may be performed if we define them here, and
 * sometimes a constexpr value is mandatory (like in a template parameter).
 *
 * The drone uses a right-handed coordinate system with X pointing right, Y
 * pointing forward, and Z pointing up.
 */

constexpr float COPTER_RADIUS = 0.26924,
		  THRUSTER0_ANGLE = 90,
		  FORCE_PER_THRUST = 3;
constexpr bool THRUSTER0_CCW = true;

// Pins are specified counterclockwise
constexpr int THRUSTER_PINS[] = {3, 23, 22, 6, 5, 4},
		  NUM_THRUSTERS = sizeof(THRUSTER_PINS) / sizeof(THRUSTER_PINS[0]),
		  MIN_ESC_PULSE = 700,
		  MAX_ESC_PULSE = 2000,
		  ESCS_CALIBRATED_ADDRESS = 0;

constexpr int RADIO_NETWORK_ID = 100,
		  RADIO_NODE_ID = 2,
		  RADIO_RECEIVER_ID = 1,

		  // Set based on your RF69 module
		  RADIO_FREQUENCY = RF69_915MHZ; 
constexpr bool HAVE_RFM69HCW = false;

constexpr int RADIO_CS_PIN = 10,
		  RADIO_IRQ_PIN = 2,
		  RADIO_IRQN = 0,
		  RADIO_RST_PIN = 9,
		  RADIO_POWER = 31; // [0, 31]

// Change to adjust for placement / coordinate system of IMU
// Offset means add 180 degrees
constexpr bool OFFSET_YAW = false,
		  OFFSET_PITCH = false,
		  OFFSET_ROLL = true,

		  NEGATE_YAW = true,
		  NEGATE_PITCH = true,
		  NEGATE_ROLL = true;

constexpr int IMU_INT_PIN = 8,
		  IMU_CALIBRATED_ADDRESS = 1,
		  IMU_CALIBRATION_ADDRESS = 2;

constexpr int VOLTAGE_PIN = 14,
		  BATTERY_CELLS = 3;
constexpr float VOLTAGE_FACTOR = 4.01356;
constexpr float LOW_VOLTAGE = 3.6 * BATTERY_CELLS,
		  CRITICAL_VOLTAGE = 3.4 * BATTERY_CELLS;

constexpr int LED_PIN = 21,
		  LED_ROWS = 6,
		  LED_COLS = 10,
		  NUM_LEDS = LED_ROWS * LED_COLS;
constexpr float LED_VOLTAGE = 5,
		  LED_CURRENT_MA = 250;

inline void beginI2C() {
	// Setup for Master mode, pins 16/17, external pullups, 400kHz for Teensy 3.1
	Wire.begin(I2C_MASTER, 0x00, I2C_PINS_16_17, I2C_PULLUP_EXT, I2C_RATE_400);
}

constexpr unsigned long SERIAL_BAUD = 115200;
