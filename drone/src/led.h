#pragma once

#define FASTLED_INTERNAL // Disable FastLED #pragma messages
#include <FastLED.h>
#include "shm.h"
#include "config.h"
#include "thread.h"

class Led {
	public:
		// Show pattern indicated in shm
		static void showShm();

		// Show pattern directly (useful before threads are running)
		static void off();
		static void dynamic();
		static void calibration();
		static void flying();
		static void lowBatt();
		static void critBatt();

	private:
		enum Pattern {
			OFF,
			DYNAMIC,
			CALIBRATION,
			FLYING,
			LOW_BATT,
			CRIT_BATT,
		};
		CRGB m_leds[NUM_LEDS];

		Thread m_offThread;
		Thread m_calibrationThread;
		Thread m_flyingThread;
		Thread m_lowBattThread;
		Thread m_critBattThread;

		int m_step;

		Led();
		static Led& get();
};
