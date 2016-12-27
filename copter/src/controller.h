#pragma once

#include <functional>

class Controller {
	public:
		Controller();
		void operator()();

	private:
		class PID {
			public:
				PID(std::function<float(float, float)> diffFunc = 
						[](float desire, float value) { return desire - value; });
				float operator()(float value, float desire, float p, float i, float d);
				void reset();

			private:
				std::function<float(float, float)> m_diffFunc;
				bool m_firstRun;
				size_t m_lastTimeMicros;
				float m_lastValue;
		};
		
		class AxisControl {
			public:
				AxisControl(std::string name);
				float forceOffset();
				void reset();

			private:
				Shm::Var *m_enabled, *m_current, *m_desire, *m_p, *m_i, *m_d;
				PID m_pid;
		};

		static float angleDiff(float a, float b);

		bool m_enabledBefore;
		AxisControl m_yawControl, m_pitchControl, m_rollControl;
};
