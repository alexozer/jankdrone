#include <Arduino.h>
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

	// TODO remove
	constexpr float forceScale = 3;
	frontForce *= forceScale;
	backForce *= forceScale;
	leftForce *= forceScale;
	rightForce *= forceScale;

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
