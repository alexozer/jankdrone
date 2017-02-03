#pragma once

#include <string>

class Deadman {
	public:
		void operator()();

	private:
		void kill(std::string reason);
};
