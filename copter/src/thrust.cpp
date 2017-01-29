#include <Arduino.h>
#include <cmath>
#include <EEPROM.h>

#include "config.h"
#include "shm.h"
#include "log.h"
#include "thrust.h"

Thrust::Thruster::Thruster(int pin, Shm::Var* thrustValue): m_thrustValue{thrustValue} {
	m_esc.attach(pin);
	(*this)(0);
}

void Thrust::Thruster::operator()() {
	(*this)(m_thrustValue->getFloat());
}

void Thrust::Thruster::operator()(float thrustValue) {
	static auto calibrate = Shm::var("switches.calibrateEscs");
	if (calibrate->getBool()) {
		EEPROM.update(CALIBRATED_ESCS_ADDRESS, false);
		Log::fatal("Shutting down to calibrate");
	}

	static auto softKill = Shm::var("switches.softKill");
	if (softKill->getBool()) {
		thrustNoKillCheck(0);
	} else {
		thrustNoKillCheck(thrustValue);
	}
}

void Thrust::Thruster::thrustNoKillCheck(float thrustValue) {
	float clampedThrust = std::max(0.0, std::min(1.0, thrustValue));
	float dutyCycleMicros = MIN_THRUSTER_PULSE + 
		(MAX_THRUSTER_PULSE - MIN_THRUSTER_PULSE) * clampedThrust;
	int roundedMicros = (int)round(dutyCycleMicros);
	m_esc.writeMicroseconds(roundedMicros);
}

Thrust::Thrust() {
	m_thrusters.reserve(NUM_THRUSTERS);
	for (auto v : Shm::group("thrusters")->vars()) {
		int i = atoi(v->name().substr(1).c_str());
		if (i >= 0 && i < NUM_THRUSTERS) {
			m_thrusters.emplace_back(THRUSTER_PINS[i], v);
		}
	}

	if (!EEPROM.read(CALIBRATED_ESCS_ADDRESS)) {
		EEPROM.update(CALIBRATED_ESCS_ADDRESS, true); // Write early in case we're interrupted

		static auto softKill = Shm::var("switches.softKill");
		bool lastSoftKill = softKill->getBool();
		softKill->set(false);

		static auto calibrating = Shm::var("thrust.calibrating");
		calibrating->set(true);

		Log::info("Calibrating thrusters...");
		for (auto& t : m_thrusters) t(1);
		delay(2500); // Wait for ESC to power on and register first input
		for (auto& t : m_thrusters) t(0);
		delay(500); // Wait for ESC to register second input
		Log::info("Done calibrating thrusters");

		calibrating->set(false);
		softKill->set(lastSoftKill);
	}
}

void Thrust::operator()() {
	for (auto& t : m_thrusters) t();
}
