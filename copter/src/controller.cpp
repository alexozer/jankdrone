#include <Arduino.h>
#include <cmath>
#include "shm.h"
#include "controller.h"

Controller::Controller():
	m_enabledBefore{Shm::var("controller.enabled")->getBool()}, 

	m_yawControl{"yaw"},
	m_pitchControl{"pitch"},
	m_rollControl{"roll"} {}

void Controller::operator()() {
	static auto enabled = Shm::var("controller.enabled");
	if (enabled->getBool()) {
		if (!m_enabledBefore) {
			m_yawControl.reset();
			m_pitchControl.reset();
			m_rollControl.reset();

			m_enabledBefore = true;
		}

	} else {
		if (m_enabledBefore) {
			static auto thrusters = Shm::group("thrusters");
			for (auto v : thrusters->vars()) {
				v->set(0);
			}

			m_enabledBefore = false;
		}

		return;
	}

	static auto verticalForce = Shm::var("controller.verticalForce");
	float initForce = verticalForce->getFloat() / 4;
	float frontForce = initForce,
		  backForce = initForce,
		  leftForce = initForce,
		  rightForce = initForce;

	float yawForceOffset = m_yawControl.forceOffset();
	frontForce += yawForceOffset / 2;
	backForce += yawForceOffset / 2;
	leftForce -= yawForceOffset / 2;
	rightForce -= yawForceOffset / 2;

	float pitchForceOffset = m_pitchControl.forceOffset();
	frontForce += pitchForceOffset / 2;
	backForce -= pitchForceOffset / 2;

	float rollForceOffset = m_rollControl.forceOffset();
	leftForce += rollForceOffset / 2;
	rightForce -= rollForceOffset / 2;

	static auto frontThruster = Shm::var("thrusters.front"),
				backThruster = Shm::var("thrusters.back"),
				leftThruster = Shm::var("thrusters.left"),
				rightThruster = Shm::var("thrusters.right");
	frontThruster->set(frontForce);
	backThruster->set(backForce);
	leftThruster->set(leftForce);
	rightThruster->set(rightForce);
}

Controller::PID::PID(std::function<float(float, float)> diffFunc):
	m_diffFunc{diffFunc}, m_firstRun{true}, m_lastTimeMicros{0}, m_lastValue{0} {}

float Controller::PID::operator()(float value, float desire, float p, float i, float d) {
	if (m_firstRun) {
		m_lastTimeMicros = micros();
		m_lastValue = value;
		m_firstRun = false;
		return 0;
	}

	float nowMicros = micros();
	float dt = (nowMicros - m_lastTimeMicros) / 1e6f;
	m_lastTimeMicros = nowMicros;

	// TODO implement integral term
	float error = m_diffFunc(desire, value);
	return p * error + d * m_diffFunc(-value, -m_lastValue) / dt;
}

void Controller::PID::reset() {
	m_firstRun = true;
}

Controller::AxisControl::AxisControl(std::string name):
	m_pid{angleDiff}
{
	auto g = Shm::group("controller");
	m_enabled = g->var(name + "Enabled");
	m_current = Shm::group("placement")->var(name);
	m_desire = g->var(name + "Desire");
	m_p = g->var(name + "P");
	m_i = g->var(name + "I");
	m_d = g->var(name + "D");
}

float Controller::AxisControl::forceOffset() {
	if (m_enabled->getBool()) {
		return m_pid(
				m_current->getFloat(),
				m_desire->getFloat(),
				m_p->getFloat(),
				m_i->getFloat(),
				m_d->getFloat());
	} else return 0;
}

void Controller::AxisControl::reset() {
	m_pid.reset();
}

float Controller::angleDiff(float a, float b) {
    float diff = std::fmod((a - b), 360);
	if (diff < 0) diff += 360;
	return (diff < 180) ? diff : diff - 360;
}
