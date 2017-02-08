#include <Arduino.h>
#include "config.h"
#include "shm.h"
#include "maths.h"
#include "log.h"
#include "deadman.h"

void Deadman::operator()() {
	if (!shm().deadman.enabled || shm().switches.softKill) return;

	if (!shm().remote.connected) {
		kill("remote disconnection");;
		return;
	}

	if (shm().power.critical) {
		kill("critically low power");
		return;
	}

	float pitchTilt = fabs(angleDiff(shm().placement.pitch, 0));
	float rollTilt = fabs(angleDiff(shm().placement.roll, 0));
	if (fmax(pitchTilt, rollTilt) > shm().deadman.maxTilt) {
		kill("extreme tilt");
		return;
	}
}

void Deadman::kill(std::string reason) {
	shm().switches.softKill = true;
	Log::warn("Softkilled by deadman due to %s", reason.c_str());
}
