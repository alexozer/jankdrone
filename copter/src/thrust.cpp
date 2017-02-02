#include <Arduino.h>
#include <cmath>
#include <EEPROM.h>

#include "shm.h"
#include "log.h"
#include "thrust.h"

using namespace Config::Thrust;

Thrust::Thruster::Thruster() {}

Thrust::Thruster::Thruster(int pin, float* thrustValue): m_thrustValue{thrustValue} {
	m_esc.attach(pin);
	(*this)(0);
}

void Thrust::Thruster::operator()() {
	(*this)(*m_thrustValue);
}

void Thrust::Thruster::operator()(float thrustValue) {
	if (shm().switches.calibrateEscs) {
		EEPROM.update(CALIBRATED_ADDRESS, false);
		Log::fatal("Shutting down to calibrate");
	}

	if (shm().switches.softKill) {
		thrustNoKillCheck(0);
	} else {
		thrustNoKillCheck(thrustValue);
	}
}

void Thrust::Thruster::thrustNoKillCheck(float thrustValue) {
	float clampedThrust = std::max(0.0, std::min(1.0, thrustValue));
	float dutyCycleMicros = MIN_PULSE + 
		(MAX_PULSE - MIN_PULSE) * clampedThrust;
	int roundedMicros = (int)round(dutyCycleMicros);
	m_esc.writeMicroseconds(roundedMicros);
}

Thrust::Thrust() {
	auto thrustersArray = shm().thrusters.array("t");
	for (int i = 0; i < NUM_THRUSTERS; i++) {
		m_thrusters[i] = {PINS[i], thrustersArray[i]->ptr<float>()};
	}

	if (!EEPROM.read(CALIBRATED_ADDRESS)) {
		EEPROM.update(CALIBRATED_ADDRESS, true); // Write early in case we're interrupted

		bool lastSoftKill = shm().switches.softKill;
		shm().switches.softKill = false;

		Log::info("Calibrating thrusters...");
		for (auto& t : m_thrusters) t(1);
		delay(2500); // Wait for ESC to power on and register first input
		for (auto& t : m_thrusters) t(0);
		delay(500); // Wait for ESC to register second input
		Log::info("Done calibrating thrusters");

		shm().switches.softKill = lastSoftKill;
	}
}

void Thrust::operator()() {
	for (auto& t : m_thrusters) t();
}
