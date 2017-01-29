#pragma once

#define FASTLED_INTERNAL // Disable FastLED #pragma messages
#include <FastLED.h>
#include "shm.h"
#include "config.h"

class Led {
	public:
		static void off();
		static void show();
		static void calibration();

	private:
		CRGB m_leds[NUM_LEDS];

		Led();
		static Led& get();
};
