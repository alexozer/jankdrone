#include <Arduino.h>
#include <cmath>
#include "maths.h"

PID::PID(std::function<float(float, float)> diffFunc):
	m_diffFunc{diffFunc}, m_firstRun{true}, m_lastValue{0} {}

float PID::operator()(float dt, float value, float desire, float p, float i, float d) {
	if (m_firstRun) {
		m_lastValue = value;
		m_firstRun = false;
		return 0;
	}

	// TODO implement integral term
	float error = m_diffFunc(desire, value);
	return p * error + d * m_diffFunc(-value, -m_lastValue) / dt;
}

void PID::reset() {
	m_firstRun = true;
}

float splitFmod(float a, float mod) {
	float modded = fmod(a, mod);
	if (fabs(modded) > mod / 2) {
		modded += modded > 0 ? -mod : mod;
	}
	return modded;
}

float angleDiff(float a, float b) {
	return splitFmod(a - b, 360);
}

bool withinDeadband(float a, float b, float deadband, bool useMod) {
	if (useMod) {
		return fabs(angleDiff(a, b)) < deadband;
	} else {
		return fabs(a - b) < deadband;
	}
}

bool fequals(float a, float b) {
	return withinDeadband(a, b, 0.000001, false);
}
