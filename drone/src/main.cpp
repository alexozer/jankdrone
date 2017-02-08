#include "teensy.h"
#include <Arduino.h>
#include "thread.h"
#include "log.h"
#include "shm.h"

#include "led.h"
#include "remote.h"
#include "imu.h"
#include "altimeter.h"
#include "controller.h"
#include "thrust.h"
#include "power.h"
#include "deadman.h"

struct Main {
	Thrust thrust;
	Imu imu;
	Altimeter altimeter;
	Remote remote;
	Controller controller;
	Deadman deadman;

	FuncSet threads;

	Main(): threads{
		Thread([&] { thrust(); }, Thread::SECOND / 1000, &shm().threadTime.thrust),
		Thread([&] { imu(); }, 0, &shm().threadTime.imu),
		Thread([&] { altimeter(); }, Thread::SECOND / 15, &shm().threadTime.altimeter),
		Thread([&] { remote(); }, 0, &shm().threadTime.remote),
		Thread([&] { controller(); }, Thread::SECOND / 1000, &shm().threadTime.controller),
		Thread([&] { deadman(); }, Thread::SECOND / 30),
		Thread(&Power::readVoltage, Thread::SECOND / 10),
		Thread(&Led::showShm, Thread::SECOND / 60, &shm().threadTime.led),
	} {}

	void operator()() { while (true) threads(); }
};

void setup() {
	beginI2C();
	Serial.begin(SERIAL_BAUD);
	//while (!Serial);

	Led::off();
	
	static Main main;
	main();
}

void loop() {}
