#pragma once

#include <cstdint>

/* Constants which describe the physical state of the drone.
 * Although it may be more consistent to define these as default values in shm,
 * compile-time optimizations may be performed if we define them here.
 *
 * The drone uses a right-handed coordinate system with X pointing right,
 * Y pointing forward, and Z pointing up.
 */

namespace Config {
	namespace Thrust {
		constexpr float COPTER_RADIUS = 0.26924,
				  THRUSTER0_ANGLE = 90,
				  FORCE_PER_THRUST = 3;
		constexpr bool THRUSTER0_CCW = true;

		// Pins are specified counterclockwise
		constexpr int PINS[] = {3, 23, 22, 6, 5, 4},
				  NUM_THRUSTERS = sizeof(PINS) / sizeof(PINS[0]),
				  MIN_PULSE = 700,
				  MAX_PULSE = 2000,
				  CALIBRATED_ADDRESS = 0;
	}

	namespace Ble {
		constexpr int REQ_PIN = 10,
				  RDY_PIN = 2,
				  RST_PIN = 9,
				  SCK_PIN = 13,
				  MISO_PIN = 12,
				  MOSI_PIN = 11;
	}

	namespace Imu {
		// Change to adjust for placement / coordinate system of imu
	
		// Offset by 180
		constexpr bool OFFSET_YAW = false,
				  OFFSET_PITCH = false,
				  OFFSET_ROLL = true,

				  NEGATE_YAW = true,
				  NEGATE_PITCH = true,
				  NEGATE_ROLL = true;

		constexpr bool UPSIDE_DOWN = true; // Rotated halfway about X
		constexpr int INT_PIN = 8,
				  SDA_PIN = 17,
				  SCL_PIN = 16,
				  CALIBRATED_ADDRESS = 1,
				  CALIBRATION_ADDRESS = 2;
	}

	namespace Power {
		constexpr int VOLTAGE_PIN = 14,
				  BATTERY_CELLS = 3;
		constexpr float VOLTAGE_FACTOR = 4.01356;
		constexpr float LOW_VOLTAGE = 3.6 * BATTERY_CELLS,
				  CRITICAL_VOLTAGE = 3.4 * BATTERY_CELLS;
	}

	namespace Led {
		constexpr int PIN = 21,
				  ROWS = 6,
				  COLS = 10,
				  NUM_LEDS = ROWS * COLS;
		constexpr float VOLTAGE = 5,
				  CURRENT_MA = 250;
	}
}
