#include <Arduino.h>
#include "shm.h"
#include "config.h"
#include "led.h"

void Led::showShm() {
	FastLED.setBrightness(shm().led.brightness);

	switch (shm().led.pattern) {
		case DYNAMIC:
			dynamic();
			break;
		case CALIBRATION:
			calibration();
			break;
		case FLYING:
			flying();
			break;
		case LOW_BATT:
			lowBatt();
			break;
		case CRIT_BATT:
			critBatt();
			break;
		default:
			off();
			break;
	}
}

void Led::off() {
	fill_solid(get().m_leds, NUM_LEDS, CRGB::Black);
	FastLED.show();
}

void Led::dynamic() {
	if (shm().power.critical) {
		critBatt();
	} else if (shm().power.low) {
		lowBatt();
	} else if (!shm().switches.softKill && shm().controller.enabled) {
		flying();
	} else {
		off();
	}
}

void Led::calibration() {
	for (int row = 0; row < LED_ROWS; row++) {
		for (int col = 0; col < LED_COLS; col++) {
			auto color = col >= 7 ? CRGB::Green : CRGB::Black;
			get().m_leds[row * LED_COLS + col] = color;
		}
	}

	FastLED.show();
}

void Led::flying() {
	constexpr int COLOR_PERIOD = 5000,
			  WAVE_PERIOD = 500,
			  S = 255;
	unsigned long t = millis();
	int colorLerp = t % COLOR_PERIOD * 255 / COLOR_PERIOD;
	int waveLerp = t % WAVE_PERIOD * 255 / WAVE_PERIOD;

	for (int row = 0; row < LED_ROWS; row++) {
		for (int col = 0; col < LED_COLS; col++) {
			int h = row % 2 == 0 ? 0 : 255 / 3;
			h = (h + colorLerp) % 255;
			
			int vLerp = 255 * (LED_ROWS - row - 1) / LED_ROWS * 2;
			vLerp += -waveLerp + col * 255 / LED_COLS / 2;
			vLerp %= 255;
			int v = quadwave8(vLerp);

			get().m_leds[row * LED_COLS + col] = CHSV(h, S, v);
		}
	}

	FastLED.show();
}

void Led::lowBatt() {
	constexpr int TIME_PERIOD = 500,
			  ROWS_PERIOD = 6,
			  H = 25, S = 255;

	for (int row = 0; row < LED_ROWS; row++) {
		int timeLerp = (millis() % TIME_PERIOD) * 255 / TIME_PERIOD;
		timeLerp += ((row % ROWS_PERIOD) * 255 / ROWS_PERIOD) % 255;
		int v = sin8(timeLerp);

		fill_solid(&get().m_leds[row * LED_COLS], LED_COLS, CHSV(H, S, v));
	}

	FastLED.show();
}

void Led::critBatt() {
	constexpr int PERIOD = 250,
			  H = 0, S = 255;

	int v = quadwave8(millis() % PERIOD * 255 / PERIOD);
	fill_solid(get().m_leds, LED_ROWS * LED_COLS, CHSV(H, S, v));
	
	FastLED.show();
}

Led::Led() {
	FastLED.addLeds<NEOPIXEL, LED_PIN>(m_leds, NUM_LEDS);
	FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTAGE, LED_CURRENT_MA);
}

Led& Led::get() {
	static Led led;
	return led;
}
