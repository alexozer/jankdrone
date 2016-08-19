#pragma once

#include "shm.h"

class Led {
	public:
		Led();

		void operator()();
		void fade();
};
