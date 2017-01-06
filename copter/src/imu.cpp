#include "logger.h"
#include "mpu9250.h"
#include "imu.h"

Imu::Imu() {
	Logger::info("Initializing IMU...");
	MPU9250::setup();
	Logger::info("Done initializing IMU");
}

void Imu::operator()() {
	MPU9250::loop();
}
