#pragma once

#include <Arduino.h>
#include <string>
#include <fmt/format.h>

namespace Logger {
	template <typename... Args>
	void write(std::string level, std::string format, const Args& ... args) {
		auto preFormat = fmt::format("[{}]\t{}", level, format);
		auto finalFormat = fmt::format(preFormat, args...);
		Serial.println(finalFormat.c_str());
		Serial.flush();
	}

	template <typename... Args>
	void debug(std::string format, const Args& ... args) {
		write("debug", format, args...);
	}

	template <typename... Args>
	void info(std::string format, const Args& ... args) {
		write("info", format, args...);
	}

	template <typename... Args>
	void warn(std::string format, const Args& ... args) {
		write("warn", format, args...);
	}

	template <typename... Args>
	void error(std::string format, const Args& ... args) {
		write("error", format, args...);
	}

	template <typename... Args>
	void fatal(std::string format, const Args& ... args) {
		write("fatal", format, args...);
		exit(1);
	}
}
