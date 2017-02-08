#pragma once

#include <Arduino.h>
#include <cstdarg>
#include <string>
#include <vector>

class Log {
	public:
		template <typename... Args>
		static void debug(std::string format, Args&& ...args) {
			Serial.print("[debug]\t");
			Serial.printf(format.c_str(), std::forward<Args>(args)...);
			Serial.println();
		}

		template <typename... Args>
		static void info(std::string format, Args&& ...args) {
			Serial.print("[info]\t");
			Serial.printf(format.c_str(), std::forward<Args>(args)...);
			Serial.println();
		}

		template <typename... Args>
		static void warn(std::string format, Args&& ...args) {
			Serial.print("[warn]\t");
			Serial.printf(format.c_str(), std::forward<Args>(args)...);
			Serial.println();
		}

		template <typename... Args>
		static void error(std::string format, Args&& ...args) {
			Serial.print("[error]\t");
			Serial.printf(format.c_str(), std::forward<Args>(args)...);
			Serial.println();
		}

		template <typename... Args>
		static void fatal(std::string format, Args&& ...args) {
			Serial.print("[fatal]\t");
			Serial.printf(format.c_str(), std::forward<Args>(args)...);
			Serial.println();
			exit(1);
		}

		enum class Level { DEBUG, INFO, WARN, ERROR, FATAL };

		template <typename... Args>
		static void log(Level level, std::string str, Args&&... args) {
			switch (level) {
				case Level::DEBUG:
					debug(str.c_str(), std::forward<Args>(args)...);
					break;
				case Level::INFO:
					info(str.c_str(), std::forward<Args>(args)...);
					break;
				case Level::WARN:
					warn(str.c_str(), std::forward<Args>(args)...);
					break;
				case Level::ERROR:
					error(str.c_str(), std::forward<Args>(args)...);
					break;
				case Level::FATAL:
					fatal(str.c_str(), std::forward<Args>(args)...);
					break;
			}
		}
};
