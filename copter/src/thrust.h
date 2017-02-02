#pragma once

#include <Servo.h>
#include "shm.h"
#include "config.h"

class Thrust {
	public:
		Thrust();
		void operator()();

	private:
		class Thruster {
			public:
				Thruster(); // Default until thruster array initialized
				Thruster(int pin, float* thrustValue);
				void operator()(); // Thrust value from shm
				void operator()(float thrustValue); // Thrust this value

			private:
				Servo m_esc;
				float* m_thrustValue;

				void thrustNoKillCheck(float thrustValue);
		};

		Thruster m_thrusters[Config::Thrust::NUM_THRUSTERS];
};
