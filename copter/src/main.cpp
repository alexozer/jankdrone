#include "teensy.h"
#include <Arduino.h>
#include "thread.h"
#include "logger.h"

#include "led.h"
#include "remote.h"

Led led;

ThreadController threadController({
		Thread(std::bind(&Led::operator(), led), 30),
		//Thread(std::bind(&Led::fade, led), 30),
		Thread(Remote(), 0),
		});

void setup() {
	Serial.begin(9600);
	while (!Serial.available()); // Wait for input first
	while (Serial.available()) Serial.read(); // Discard input
}

void loop() {
	threadController();
}
