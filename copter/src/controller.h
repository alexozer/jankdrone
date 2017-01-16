#pragma once

#include <functional>
#include "maths.h"
#include "shm.h"

class Controller {
	public:
		Controller();
		void operator()();

	private:
		class AxisControl {
			public:
				AxisControl(std::string name);
				float forceOffset();
				void reset();

			private:
				Shm::Var *m_enabled, *m_current, *m_desire, *m_p, *m_i, *m_d;
				PID m_pid;
		};

		bool m_enabledBefore;
		AxisControl m_yawControl, m_pitchControl, m_rollControl;
};
