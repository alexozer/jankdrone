#include <Arduino.h>
#include "shm.h"
#include "config.h"
#include "led.h"
#include "logger.h"

Led::Led() {
	for (auto pin : {FRONT_LED_PIN, LEFT_LED_PIN, RIGHT_LED_PIN, BACK_LED_PIN}) {
		pinMode(pin, OUTPUT);
	}
}

void Led::operator()() {
	static auto front = Shm::var("leds.front"),
				left = Shm::var("leds.left"),
				right = Shm::var("leds.right"),
				back = Shm::var("leds.back");

	analogWrite(FRONT_LED_PIN, front->getInt());
	analogWrite(LEFT_LED_PIN, left->getInt());
	analogWrite(RIGHT_LED_PIN, right->getInt());
	analogWrite(BACK_LED_PIN, back->getInt());
}

constexpr int FADE_PERIOD = 500,
		  FADE_MAX_VALUE = 200;

void Led::fade() {
	int modMillis = millis() % FADE_PERIOD;
	int pwm = (modMillis % (FADE_PERIOD / 2)) * FADE_MAX_VALUE / (FADE_PERIOD / 2);
	if (modMillis > FADE_PERIOD / 2) {
		pwm = FADE_MAX_VALUE - pwm;
	}

	static auto led = Shm::var("leds.front");
	led->set(pwm);
}
