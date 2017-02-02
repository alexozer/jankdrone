#pragma once

#include <functional>
#include "maths.h"
#include "config.h"
#include "shm.h"

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
				float offset();
				void reset();

			private:
				bool* m_enabled;
				float *m_current, *m_desire, *m_out, *m_p, *m_i, *m_d;
				PID m_pid;
		};

		bool m_enabledBefore;
		Thruster m_thrusters[Config::Thrust::NUM_THRUSTERS];
		AxisControl m_yawControl, m_pitchControl, m_rollControl;

		void initThrusters();
		void checkTorqueIndependence();
};
