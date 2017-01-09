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

void printPlacement() {
	static auto yaw = Shm::var("placement.yaw"),
				pitch = Shm::var("placement.pitch"),
				roll = Shm::var("placement.roll"),
				temperature = Shm::var("temperature.gyro");
	Serial.print("Yaw: "); Serial.println(yaw->getFloat());
	Serial.print("Pitch: "); Serial.println(pitch->getFloat());
	Serial.print("Roll: "); Serial.println(roll->getFloat());
	Serial.print("Temperature: "); Serial.println(temperature->getFloat());
	Serial.println();
}

void printThrusters() {
	static auto front = Shm::var("thrusters.front"),
				back = Shm::var("thrusters.back"),
				left = Shm::var("thrusters.left"),
				right = Shm::var("thrusters.right"),
				softKill = Shm::var("switches.softKill");
	Serial.print("Front thruster: "); Serial.println(front->getFloat());
	Serial.print("Back thruster: "); Serial.println(back->getFloat());
	Serial.print("Left thruster: "); Serial.println(left->getFloat());
	Serial.print("Right thruster: "); Serial.println(right->getFloat());
	Serial.print("Soft kill: "); Serial.println(softKill->getBool());
	Serial.println();
}

void printLeds() {
	static auto front = Shm::var("leds.front"),
				back = Shm::var("leds.back"),
				left = Shm::var("leds.left"),
				right = Shm::var("leds.right");
	for (auto v : {front, back, left, right}) {
		Serial.print(v->getInt());
		if (v != right) Serial.print(", ");
	}
	Serial.println();
}

// Statically-allocate everything inside setup() so we can set up serial first
void setup() {
	Serial.begin(115200);

	//while (!Serial.available()); // Wait for input first
	//while (Serial.available()) Serial.read(); // Discard input
	//Logger::info("Starting!");

	static Led led;
	static ThreadController threadController({
			Thread(Remote(), 0),
			Thread(Imu(), 0),
			Thread(Controller(), 1),
			Thread(Thrust(), 1),
			Thread([&] { led(); }, 15),
			Thread([&] { led.fade(); }, 15),
			
			//Thread(&printPlacement, 100),
			//Thread(&printThrusters, 100),
			//Thread(&printLeds, 100),
			});

	while (true) threadController();
}

void loop() {}
