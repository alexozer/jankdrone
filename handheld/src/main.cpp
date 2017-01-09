#include <Arduino.h>

constexpr int SOFT_KILL = 12,
		  LEFT_X = A0, LEFT_Y = A1,
		  RIGHT_X = A2, RIGHT_Y = A3,
		  LED = 13;
const int ANALOG_PINS[] = {LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y};
constexpr int BLINK_PERIOD = 500;

void setup() {
	Serial.begin(38400);
	pinMode(SOFT_KILL, INPUT_PULLUP);
	pinMode(LED, OUTPUT);
}

void loop() {
	bool softKill = digitalRead(SOFT_KILL);
	Serial.print(digitalRead(SOFT_KILL));
	Serial.print(' ');

	for (int pin : ANALOG_PINS) {
		Serial.print(analogRead(pin));
		if (pin != ANALOG_PINS[sizeof(ANALOG_PINS) / sizeof(ANALOG_PINS[0]) - 1]) {
			Serial.print(' ');
		}
	}

	Serial.println();

	bool ledState = !softKill && (millis() % BLINK_PERIOD) >= (BLINK_PERIOD / 2);
	digitalWrite(LED, ledState);

	delay(20);
}
