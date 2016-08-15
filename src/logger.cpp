#include <Arduino.h>
#include "logger.h"

void write(std::string level, std::string message) {
	Serial.println(("[" + level + "]\t" + message).c_str());
	Serial.flush();
}

void Logger::info(std::string message) {
	write("info", message);
}

void Logger::warn(std::string message) {
	write("warn", message);
}

void Logger::error(std::string message) {
	write("error", message);
}
