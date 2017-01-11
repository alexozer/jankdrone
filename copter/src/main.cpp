#include "teensy.h"
#include <Arduino.h>
#include <fmt/format.h>
#include "thread.h"
#include "logger.h"
#include "shm.h"

#include "led.h"
#include "remote.h"
#include "imu.h"
#include "controller.h"
#include "thrust.h"
#include "power.h"

// Statically-allocate everything inside setup() so we can set up serial first
void setup() {
	Serial.begin(115200);

	//while (!Serial.available()); // Wait for input first
	//while (Serial.available()) Serial.read(); // Discard input
	//Logger::info("Starting!");

	static Led led;
	static ThreadController threadController({
			Thread(Remote(), 0),
			//Thread(Imu(), 0),
			Thread(Controller(), 1),
			Thread(Thrust(), 1),
			Thread(&Power::readVoltage, 100),
			Thread([&] { led(); }, 15),
			Thread([&] { led.showStatus(); }, 15),
			});

	while (true) threadController();
}

void loop() {}
