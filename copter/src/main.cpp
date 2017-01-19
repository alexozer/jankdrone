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

struct Main {
	Remote remote;
	Imu imu;
	Controller controller;
	Thrust thrust;
	Led led;

	ThreadController threadController;

	Main(): threadController{
		Thread([&] { remote(); }, 0),
		Thread([&] { imu(); }, 0),
		Thread([&] { controller(); }, 1),
		Thread([&] { thrust(); }, 1),
		Thread(&Power::readVoltage, 100),
		Thread([&] { led(); }, 1000 / 60),
		Thread([&] { led.showStatus(); }, 1000 / 60),
	} {}

	void operator()() { while (true) threadController(); }
};

void setup() {
	Serial.begin(115200);

	static Main main;
	main();
}

void loop() {}
