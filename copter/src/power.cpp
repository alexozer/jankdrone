#include <Arduino.h>
#include "shm.h"
#include "config.h"
#include "power.h"

void Power::readVoltage() {
	float reading = analogRead(VOLTAGE_PIN);
	float voltage = reading * 3.3 * VOLTAGE_FACTOR / 1023;
	static auto voltageVar = Shm::var("power.voltage");
	voltageVar->set(voltage);
}
