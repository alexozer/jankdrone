#include "teensy.h"
#include <Arduino.h>
#include <fmt/format.h>
#include "thread.h"
#include "logger.h"
#include "shm.h"

#include "led.h"
#include "remote.h"
#include "mpu9250.h"
#include "controller.h"

void printOrientation() {
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

void printMotors() {
	static auto front = Shm::var("motors.front"),
				back = Shm::var("motors.back"),
				left = Shm::var("motors.left"),
				right = Shm::var("motors.right"),
				yaw = Shm::var("placement.yaw");
	Serial.print("Front motor: "); Serial.println(front->getFloat());
	Serial.print("Back motor: "); Serial.println(back->getFloat());
	Serial.print("Left motor: "); Serial.println(left->getFloat());
	Serial.print("Right motor: "); Serial.println(right->getFloat());
	Serial.print("Yaw: "); Serial.println(yaw->getFloat());
	Serial.println();
}

// Statically-allocate everything inside setup() so we can set up serial first
void setup() {
	Serial.begin(38400);
	while (!Serial.available()); // Wait for input first
	while (Serial.available()) Serial.read(); // Discard input
	Logger::info("Starting!");

	static Led led;
	static ThreadController threadController({
			Thread(std::bind(&Led::operator(), led), 15),
			Thread(std::bind(&Led::fade, led), 15),
			Thread(&MPU9250::loop, 0),
			Thread(Remote(), 0),
			Thread(Controller(), 1),
			//Thread(&printOrientation, 100),
			Thread(&printMotors, 100),
			});

	Logger::info("Initializing MPU9250...");
	MPU9250::setup();
	Logger::info("Done initializing MPU9250");

	Logger::info("Enabling controller");
	Shm::var("controller.enabled")->set(true);
	Logger::info("Adding some vertical force");
	Shm::var("controller.verticalForce")->set(50);

	while (true) threadController();
}

void loop() {}
