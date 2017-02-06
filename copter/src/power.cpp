#include <Arduino.h>
#include "shm.h"
#include "config.h"
#include "power.h"

void Power::readVoltage() {
	float reading = analogRead(VOLTAGE_PIN);
	float voltage = reading * 3.3 * VOLTAGE_FACTOR / 1023;

	shm().power.voltage = voltage;
	shm().power.low = voltage <= LOW_VOLTAGE;
	shm().power.critical = voltage <= CRITICAL_VOLTAGE;
}
