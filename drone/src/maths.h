#pragma once

#include <functional>

class PID {
	public:
		PID(std::function<float(float, float)> diffFunc = 
				[](float desire, float value) { return desire - value; });
		float operator()(float dt, float value, float desire, float p, float i, float d);
		void reset();

	private:
		std::function<float(float, float)> m_diffFunc;
		bool m_firstRun;
		float m_lastValue;
};

// Split float point mod [-mod/2, mod/2]
float splitFmod(float a, float mod);

float angleDiff(float a, float b);
bool withinDeadband(float a, float b, float deadband, bool useMod);
bool fequals(float a, float b);
