#pragma once

#include <SparkFunMPL3115A2.h>

class Altimeter {
	public:
		Altimeter();
		void operator()();

	private:
		MPL3115A2 m_sensor;
		float m_groundAltitude;
};
