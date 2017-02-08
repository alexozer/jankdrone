#include "log.h"
#include "mpu9250.h"
#include "imu.h"

Imu::Imu() {
	Log::info("Initializing IMU...");
	MPU9250::setup();
	Log::info("Done initializing IMU");
}

void Imu::operator()() {
	MPU9250::loop();
}
