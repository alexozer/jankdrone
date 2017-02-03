#include <Arduino.h>
#include "shm.h"
#include "log.h"
#include "controller.h"

using namespace Config::Thrust;

Controller::Controller():
	m_enabledBefore{shm().var("controller.enabled")->get<bool>()}, 

	m_yawControl{"yaw"},
	m_pitchControl{"pitch"},
	m_rollControl{"roll"}
{
	initThrusters();
	checkTorqueIndependence();
}

void Controller::initThrusters() {
	auto thrustersArray = shm().thrusters.array("t");
	for (int i = 0; (size_t)i < thrustersArray.size(); i++) {
		m_thrusters[i].shm = thrustersArray[i]->ptr<float>();
	}

	float totalPitchTorque = 0, totalRollTorque = 0;
	for (int i = 0; i < NUM_THRUSTERS; i++) {
		auto& t = m_thrusters[i];

		// Force
		constexpr float FORCE_PORTION = 0.75;
		t.force.thrustPerTotalValue = FORCE_PORTION;
		t.force.valuePerThrust = FORCE_PORTION / NUM_THRUSTERS;

		// Yaw (by angular momentum of drone body)
		// Assume thrusters alternate direction starting at counterclockwise
		t.yaw.valuePerThrust = i % 2 != THRUSTER0_CCW ? -1 : 1;
		t.yaw.thrustPerTotalValue = -t.yaw.valuePerThrust / NUM_THRUSTERS;

		// Pitch and roll (by torque)
		float angle = radians(THRUSTER0_ANGLE + 360.0 / NUM_THRUSTERS * i);
		float pitchPortion = sin(angle), rollPortion = -cos(angle);
		float pitchTorque = pitchPortion * COPTER_RADIUS * FORCE_PER_THRUST;
		float rollTorque = rollPortion * COPTER_RADIUS * FORCE_PER_THRUST;

		t.pitch.valuePerThrust = pitchTorque;
		t.roll.valuePerThrust = rollTorque;
		t.pitch.thrustPerTotalValue = pitchPortion;
		t.roll.thrustPerTotalValue = rollPortion;
		totalPitchTorque += pitchTorque * pitchPortion;
		totalRollTorque += rollTorque * rollPortion;
	}
	for (auto& t : m_thrusters) {
		t.pitch.thrustPerTotalValue /= totalPitchTorque;
		t.roll.thrustPerTotalValue /= totalRollTorque;
	}
}

// Check independence of degrees of freedom (relies on symmetry of drone)
void Controller::checkTorqueIndependence() {
	float deltaForce = 0;
	float momentumByPitch = 0, momentumByRoll = 0;
	float rollTorqueByPitch = 0, pitchTorqueByRoll = 0;
	for (int i = 0; i < NUM_THRUSTERS; i++) {
		// Assume a torque of 1
		float thrustP = m_thrusters[i].pitch.thrustPerTotalValue;
		float thrustR = m_thrusters[i].roll.thrustPerTotalValue;
		float totalThrust = thrustP + thrustR;

		deltaForce += totalThrust;
		bool ccw = i % 2 != THRUSTER0_CCW;
		momentumByPitch += ccw ? thrustP : -thrustP;
		momentumByRoll += ccw ? thrustR : -thrustR;
		rollTorqueByPitch += thrustP * m_thrusters[i].roll.valuePerThrust;
		pitchTorqueByRoll += thrustR * m_thrusters[i].pitch.valuePerThrust;
	}
	if (!fequals(deltaForce, 0)
			|| !fequals(momentumByPitch, 0)
			|| !fequals(momentumByRoll, 0)
			|| !fequals(rollTorqueByPitch, 0) 
			|| !fequals(pitchTorqueByRoll, 0)) {
		Log::info("force\tpitch momentum\troll momentum\tpitch\troll");
		Serial.print(deltaForce); Serial.print('\t');
		Serial.print(momentumByPitch); Serial.print('\t');
		Serial.print(momentumByRoll); Serial.print('\t');
		Serial.print(rollTorqueByPitch); Serial.print('\t');
		Serial.print(pitchTorqueByRoll); Serial.println();
		Log::fatal("Torque axes dependent");
	}
}

void Controller::operator()() {
	if (shm().controller.enabled) {
		if (!m_enabledBefore) {
			m_yawControl.reset();
			m_pitchControl.reset();
			m_rollControl.reset();

			m_enabledBefore = true;
		}

	} else {
		if (m_enabledBefore) {
			for (auto v : shm().thrusters.vars()) {
				v->set(0);
			}

			m_enabledBefore = false;
		}

		return;
	}

	for (auto& t : m_thrusters) {
		*t.shm = t.force.thrustPerTotalValue * shm().desires.force
			+ t.yaw.thrustPerTotalValue * m_yawControl.offset()
			+ t.pitch.thrustPerTotalValue * m_pitchControl.offset()
			+ t.roll.thrustPerTotalValue * m_rollControl.offset();
	}
}

Controller::AxisControl::AxisControl(std::string name):
	m_pid{angleDiff}
{
	auto settings = shm().group(name + "Conf");
	m_enabled = settings->var("enabled")->ptr<bool>();
	m_p = settings->var("p")->ptr<float>();
	m_i = settings->var("i")->ptr<float>();
	m_d = settings->var("d")->ptr<float>();
	m_current = shm().placement.var(name)->ptr<float>();
	m_desire = shm().desires.var(name)->ptr<float>();
	m_out = shm().controllerOut.var(name)->ptr<float>();
}

float Controller::AxisControl::offset() {
	if (*m_enabled) {
		*m_out = m_pid(*m_current, *m_desire, *m_p, *m_i, *m_d);
	} else {
		*m_out = 0;
	}

	return *m_out;
}

void Controller::AxisControl::reset() {
	m_pid.reset();
}
