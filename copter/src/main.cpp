#include "teensy.h"
#include <Arduino.h>
#include "thread.h"
#include "log.h"
#include "shm.h"

#include "led.h"
#include "remote.h"
#include "imu.h"
#include "controller.h"
#include "thrust.h"
#include "power.h"
#include "deadman.h"

struct Main {
	Thrust thrust;
	Imu imu;
	Remote remote;
	Controller controller;
	Deadman deadman;

	ThreadController threadController;

	Main(): threadController{
		Thread([&] { thrust(); }, Thread::SECOND / 1000),
		Thread([&] { remote(); }, 0),
		Thread([&] { imu(); }, 0),
		Thread([&] { controller(); }, Thread::SECOND / 1000),
		Thread(&Power::readVoltage, Thread::SECOND / 10),
		Thread(&Led::showShm, Thread::SECOND / 60),
		Thread([&] { deadman(); }, Thread::SECOND / 30),
	} {}

	void operator()() { while (true) threadController(); }
};

void setup() {
	Serial.begin(115200);
	//while (!Serial);

	Led::off();
	
	static Main main;
	main();
}

void loop() {}
