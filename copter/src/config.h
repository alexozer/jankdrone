#pragma once

/* Constants which describe the physical state of the drone.
 * Although it may be more consistent to define these as default values in shm,
 * compile-time optimizations may be performed if we define them here.
 *
 * The drone uses a right-handed coordinate system with X pointing right,
 * Y pointing forward, and Z pointing up.
 */

constexpr float COPTER_RADIUS = 0.26924;

constexpr int THRUSTER_PINS[] = {3, 23, 4, 22, 6, 5},
		  NUM_THRUSTERS = sizeof(THRUSTER_PINS) / sizeof(THRUSTER_PINS[0]),
		  MIN_THRUSTER_PULSE = 700,
		  MAX_THRUSTER_PULSE = 2000;

constexpr float FORCE_PER_THRUST = 3;

constexpr float DEFAULT_YAW_P = 0.001,
		  DEFAULT_YAW_I = 0,
		  DEFAULT_YAW_D = 0,

		  DEFAULT_PITCH_P = 0.01,
		  DEFAULT_PITCH_I = 0,
		  DEFAULT_PITCH_D = 0,

		  DEFAULT_ROLL_P = 0.01,
		  DEFAULT_ROLL_I = 0,
		  DEFAULT_ROLL_D = 0;

constexpr int BLUEFRUIT_REQ_PIN = 10,
		  BLUEFRUIT_RDY_PIN = 2,
		  BLUEFRUIT_RST_PIN = 9,
		  BLUEFRUIT_SCK_PIN = 13,
		  BLUEFRUIT_MISO_PIN = 12,
		  BLUEFRUIT_MOSI_PIN = 11;

constexpr bool IMU_UPSIDE_DOWN = true; // Rotated halfway about X
constexpr int IMU_INT_PIN = 8,
		  IMU_SDA_PIN = 17,
		  IMU_SCL_PIN = 16;

constexpr int VOLTAGE_PIN = 14,
		  BATTERY_CELLS = 3;
constexpr float VOLTAGE_FACTOR = 4.01356;
constexpr float LOW_BATTERY_VOLTAGE = 3.6 * BATTERY_CELLS,
		  CRITICAL_BATTERY_VOLTAGE = 3.4 * BATTERY_CELLS;

constexpr int LEDS_PIN = 21,
		  LED_ROWS = 6,
		  LEDS_PER_ROW = 10,
		  NUM_LEDS = LED_ROWS * LEDS_PER_ROW;

constexpr int CALIBRATED_ESCS_ADDRESS = 0,
		  CALIBRATED_IMU_ADDRESS = 1,
		  IMU_CALIBRATION_ADDRESS = 2;
