#include <Arduino.h>
#include "shm.h"
#include "log.h"
#include "controller.h"

Controller::Controller():
	m_enabledBefore{Shm::var("controller.enabled")->getBool()}, 

	m_yawControl{"yaw"},
	m_pitchControl{"pitch"},
	m_rollControl{"roll"}
{
	initSettings();
	initThrusters();
	checkTorqueIndependence();
}

void Controller::initSettings() {
	static auto yaw = Shm::group("yawSettings"),
				pitch = Shm::group("pitchSettings"),
				roll = Shm::group("rollSettings");

	yaw->var("p")->set(DEFAULT_YAW_P);
	yaw->var("i")->set(DEFAULT_YAW_I);
	yaw->var("d")->set(DEFAULT_YAW_D);

	pitch->var("p")->set(DEFAULT_PITCH_P);
	pitch->var("i")->set(DEFAULT_PITCH_I);
	pitch->var("d")->set(DEFAULT_PITCH_D);

	roll->var("p")->set(DEFAULT_ROLL_P);
	roll->var("i")->set(DEFAULT_ROLL_I);
	roll->var("d")->set(DEFAULT_ROLL_D);
}

void Controller::initThrusters() {
	for (auto v : Shm::group("thrusters")->vars()) {
		int i = atoi(v->name().substr(1).c_str());
		if (i < 0 || i >= NUM_THRUSTERS) {
			// Extraneous thruster
			continue;
		}

		m_thrusters[i].shm = v;
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
		t.yaw.valuePerThrust = i % 2 == 0 ? -1 : 1;
		t.yaw.thrustPerTotalValue = -t.yaw.valuePerThrust / NUM_THRUSTERS;

		// Pitch and roll (by torque)
		float angle = radians(360.0 / NUM_THRUSTERS * i);
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
		bool ccw = i % 2 == 0;
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

	for (int i = 0; i < NUM_THRUSTERS; i++) {
		auto& t = m_thrusters[i];

		static auto force = Shm::var("desires.force");
		t.shm->set(t.force.thrustPerTotalValue * force->getFloat() 
			+ t.yaw.thrustPerTotalValue * m_yawControl.offset()
			+ t.pitch.thrustPerTotalValue * m_pitchControl.offset()
			+ t.roll.thrustPerTotalValue * m_rollControl.offset());
	}
}

Controller::AxisControl::AxisControl(std::string name):
	m_pid{angleDiff}
{
	auto settings = Shm::group(name + "Settings");
	m_enabled = settings->var("enabled");
	m_p = settings->var("p");
	m_i = settings->var("i");
	m_d = settings->var("d");
	m_current = Shm::group("placement")->var(name);
	m_desire = Shm::group("desires")->var(name);
	m_out = Shm::group("controllerOut")->var(name);
}

float Controller::AxisControl::offset() {
	float out;
	if (m_enabled->getBool()) {
		out = m_pid(
				m_current->getFloat(),
				m_desire->getFloat(),
				m_p->getFloat(),
				m_i->getFloat(),
				m_d->getFloat());
	} else out = 0;

	m_out->set(out);
	return out;
}

void Controller::AxisControl::reset() {
	m_pid.reset();
}
