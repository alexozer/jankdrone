#include <Arduino.h>
#include "shm.h"
#include "config.h"
#include "led.h"

Led::Led() {
	for (auto pin : {FRONT_LED_PIN, LEFT_LED_PIN, RIGHT_LED_PIN, BACK_LED_PIN}) {
		pinMode(pin, OUTPUT);
	}
}

void Led::operator()() {
	static auto front = Shm::var("led.front"),
				left = Shm::var("led.left"),
				right = Shm::var("led.right"),
				back = Shm::var("led.back");

	analogWrite(FRONT_LED_PIN, front->getInt());
	analogWrite(LEFT_LED_PIN, left->getInt());
	analogWrite(RIGHT_LED_PIN, right->getInt());
	analogWrite(BACK_LED_PIN, back->getInt());
}

constexpr int BLINK_PERIOD = 500;

void Led::blink() {
	static auto led = Shm::var("led.front");

	if ((millis() % BLINK_PERIOD) < BLINK_PERIOD / 2) {
		led->set(255);
	} else {
		led->set(0);
	}
}
