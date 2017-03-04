#include <Arduino.h>
#include <cmath>
#include "maths.h"

PID::PID(std::function<float(float, float)> diffFunc): m_diffFunc{diffFunc} {}

float PID::operator()(float dt, float value, float velValue, float desire, float p, float i, float d) {
	// TODO implement integral term
	float error = m_diffFunc(desire, value);
	return p * error - d * velValue;
}

void PID::reset() {
	// TODO implement integral term
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
