#include <Arduino.h>
#include "shm.h"
#include "config.h"
#include "led.h"

void Led::off() {
	fill_solid(get().m_leds, NUM_LEDS, CRGB::Black);
	FastLED.show();
}

void Led::show() {
	// TODO show reflection of copter state
	off();
}

void Led::calibration() {
	for (int row = 0; row < LED_ROWS; row++) {
		for (int col = 0; col < LEDS_PER_ROW; col++) {
			auto color = col >= 7 ? CRGB::Green : CRGB::Black;
			get().m_leds[row * LEDS_PER_ROW + col] = color;
		}
	}

	FastLED.show();
}

Led::Led() {
	FastLED.addLeds<NEOPIXEL, LEDS_PIN>(m_leds, NUM_LEDS);
}

Led& Led::get() {
	static Led led;
	return led;
}
