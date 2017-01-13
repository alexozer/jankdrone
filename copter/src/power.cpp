#include <Arduino.h>
#include "shm.h"
#include "config.h"
#include "power.h"

void Power::readVoltage() {
	static auto voltageVar = Shm::var("power.voltage");
	float reading = analogRead(VOLTAGE_PIN);
	float voltage = reading * 3.3 * VOLTAGE_FACTOR / 1023;
	voltageVar->set(voltage);

	static auto lowVar = Shm::var("power.low"),
				criticalVar = Shm::var("power.critical");
	lowVar->set(voltage <= LOW_BATTERY_VOLTAGE);
	criticalVar->set(voltage <= CRITICAL_BATTERY_VOLTAGE);
}
