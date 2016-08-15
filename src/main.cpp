#include "teensy.h"
#include <Arduino.h>
#include "shm.h"
#include "thread.h"
#include "logger.h"

#include "led.h"

Led led;

ThreadController threadController({
		Thread(std::bind(&Led::operator(), led), 50),
		Thread(std::bind(&Led::blink, led), 50),
		});

void setup() {
	Serial.begin(9600);
	while (!Serial.available()); // Wait for input first
}

void loop() {
	threadController();
}
