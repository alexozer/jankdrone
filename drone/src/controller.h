#pragma once

#include <functional>
#include "maths.h"
#include "shm.h"
#include "config.h"

class Controller {
	public:
		Controller();

		void operator()();

	private:
		struct ThrusterAxis {
			float thrustPerTotalValue;
			float valuePerThrust;
		};

		struct Thruster {
			ThrusterAxis force, yaw, pitch, roll;
			float* shm;
		};

		class AxisControl {
			public:
				AxisControl(std::string name);
				float out();
				void reset();

			private:
				bool* m_enabled;
				float *m_current, *m_desire, *m_out,
					*m_p, *m_i, *m_d;
				PID m_pid;
		};

		bool m_enabledBefore;
		unsigned long m_lastTime;
		Thruster m_thrusters[NUM_THRUSTERS];
		AxisControl m_yawControl, m_pitchControl, m_rollControl;

		void initSettings();
		void initThrusters();
		void checkTorqueIndependence();
		void applyVelDesires(unsigned long time);
};
