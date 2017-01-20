#pragma once

#include <Arduino.h>
#include <cstdarg>
#include <string>

class Log {
	public:
		enum class Level { DEBUG, INFO, WARN, ERROR, FATAL };

		static void debug(std::string str, ...);
		static void info(std::string str, ...);
		static void warn(std::string str, ...);
		static void error(std::string str, ...);
		static void fatal(std::string str, ...);

	private:
		static void log(Level level, std::string str, va_list argv);
};
