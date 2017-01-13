#pragma once

#include <Arduino.h>
#include <string>
#include <fmt/format.h>

namespace Logger {
	enum class Level { DEBUG, INFO, WARN, ERROR, FATAL };

	template <typename... Args>
	void log(Level level, std::string format, const Args& ... args) {
		std::string levelStr;
		switch (level) {
			case Level::DEBUG:
				levelStr = "debug";
				break;
			case Level::INFO:
				levelStr = "info";
				break;
			case Level::WARN:
				levelStr = "warn";
				break;
			case Level::ERROR:
				levelStr = "error";
				break;
			case Level::FATAL:
				levelStr = "fatal";
				break;
		}

		auto preFormat = fmt::format("[{}]\t{}", levelStr, format);
		auto finalFormat = fmt::format(preFormat, args...);
		Serial.println(finalFormat.c_str());
		Serial.flush();

		if (level == Level::FATAL) exit(1);
	}

	template <typename... Args>
	void debug(std::string format, const Args& ... args) {
		log(Level::DEBUG, format, args...);
	}

	template <typename... Args>
	void info(std::string format, const Args& ... args) {
		log(Level::INFO, format, args...);
	}

	template <typename... Args>
	void warn(std::string format, const Args& ... args) {
		log(Level::WARN, format, args...);
	}

	template <typename... Args>
	void error(std::string format, const Args& ... args) {
		log(Level::ERROR, format, args...);
	}

	template <typename... Args>
	void fatal(std::string format, const Args& ... args) {
		log(Level::FATAL, format, args...);
	}
}
