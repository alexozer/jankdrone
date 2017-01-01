#include <Arduino.h>

constexpr int LEFT_X = A0, LEFT_Y = A1,
		  RIGHT_X = A2, RIGHT_Y = A3;
const int PINS[] = {LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y};

void setup() {
	Serial.begin(38400);
	//for (int pin : PINS) {
		//pinMode(pin, INPUT);
	//}
}

void loop() {
	for (int pin : PINS) {
		Serial.print(analogRead(pin));
		if (pin != PINS[sizeof(PINS) / sizeof(PINS[0]) - 1]) {
			Serial.print(' ');
		}
	}
	Serial.println();
	delay(20);
}
