#pragma once

#include <Arduino.h>
#include <cstdarg>
#include <string>
#include <vector>

class BufferPrint : public Print {
	public:
		size_t write(uint8_t b) override;
		size_t write(const uint8_t* buffer, size_t size) override;
		void flush();

	private:
		std::vector<uint8_t> m_buf;
};

class Log {
	public:
		enum class Level { DEBUG, INFO, WARN, ERROR, FATAL };

		static void debug(std::string str, ...);
		static void info(std::string str, ...);
		static void warn(std::string str, ...);
		static void error(std::string str, ...);
		static void fatal(std::string str, ...);
		static void log(Level level, std::string str, va_list argv, Print& printer = Serial);
};
