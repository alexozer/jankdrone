#include <Arduino.h>
#include <cmath>
#include "maths.h"

PID::PID(std::function<float(float, float)> diffFunc):
	m_diffFunc{diffFunc}, m_firstRun{true}, m_lastTimeMicros{0}, m_lastValue{0} {}

float PID::operator()(float value, float desire, float p, float i, float d) {
	if (m_firstRun) {
		m_lastTimeMicros = micros();
		m_lastValue = value;
		m_firstRun = false;
		return 0;
	}

	size_t nowMicros = micros();
	float dt = (float)(nowMicros - m_lastTimeMicros) / 1e6f;
	m_lastTimeMicros = nowMicros;

	// TODO implement integral term
	float error = m_diffFunc(desire, value);
	return p * error + d * m_diffFunc(-value, -m_lastValue) / dt;
}

void PID::reset() {
	m_firstRun = true;
}

float pfmod(float a, float mod) {
	float modded = std::fmod(a, mod);
	if (modded < 0) modded += mod;
	return modded;
}

float angleDiff(float a, float b) {
    float diff = pfmod(a - b, 360);
	return (diff < 180) ? diff : diff - 360;
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
