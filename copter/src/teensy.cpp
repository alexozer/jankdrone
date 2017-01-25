#include "teensy.h"

extern "C" {
	int _getpid() { return -1; }
	int _kill(int, int) { return -1; }
}
