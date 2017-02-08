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
			get().m_calibrationThread();
			break;
		case FLYING:
			get().m_flyingThread();
			break;
		case LOW_BATT:
			get().m_lowBattThread();
			break;
		case CRIT_BATT:
			get().m_critBattThread();
			break;
		default:
			get().m_offThread();
			break;
	}
}

void Led::off() {
	fill_solid(get().m_leds, NUM_LEDS, CRGB::Black);
	FastLED.show();
}

void Led::dynamic() {
	if (shm().power.critical) {
		get().m_critBattThread();
	} else if (shm().power.low) {
		get().m_lowBattThread();
	} else if (!shm().switches.softKill && shm().controller.enabled) {
		get().m_flyingThread();
	} else {
		get().m_offThread();
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
			  WAVE_PERIOD = 500;
	unsigned long t = millis();
	int colorLerp = t % COLOR_PERIOD * 255 / COLOR_PERIOD;
	int waveLerp = t % WAVE_PERIOD * 255 / WAVE_PERIOD;

	for (int row = 0; row < LED_ROWS; row++) {
		for (int col = 0; col < LED_COLS; col++) {
			int hue = row % 2 == 0 ? 0 : 255 / 3;
			hue = (hue + colorLerp) % 255;
			
			int vLerp = 255 * (LED_ROWS - row - 1) / LED_ROWS * 2;
			vLerp += -waveLerp + col * 255 / LED_COLS / 2;
			vLerp %= 255;
			int v = quadwave8(vLerp);

			get().m_leds[row * LED_COLS + col] = CHSV(hue, 255, v);
		}
	}

	FastLED.show();
}

void Led::lowBatt() {
	constexpr int MOD = 3;

	get().m_step = (get().m_step + 1) % MOD;

	for (int row = 0; row < LED_ROWS; row++) {
		auto color = row % MOD == get().m_step ? CRGB::Orange : CRGB::Black;
		fill_solid(&get().m_leds[row * LED_COLS], LED_COLS, color);
	}

	FastLED.show();
}

void Led::critBatt() {
	constexpr int MOD = 2;
	get().m_step = (get().m_step + 1) % MOD;

	auto color = get().m_step ? CRGB::Red : CRGB::Black;
	fill_solid(get().m_leds, NUM_LEDS, color);

	FastLED.show();
}

Led::Led():
	m_offThread{[&] { off(); }, Thread::SECOND / 10},
	m_calibrationThread{[&] { calibration(); }, Thread::SECOND / 10},
	m_flyingThread{[&] { flying(); }, Thread::SECOND / 30},
	m_lowBattThread{[&] { lowBatt(); }, Thread::SECOND / 8},
	m_critBattThread{[&] { critBatt(); }, Thread::SECOND / 8},
	m_step{0}
{
	FastLED.addLeds<NEOPIXEL, LED_PIN>(m_leds, NUM_LEDS);
	FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTAGE, LED_CURRENT_MA);
}

Led& Led::get() {
	static Led led;
	return led;
}
