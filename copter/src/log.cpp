#include "log.h"

// Unfortunately these near-duplicates are necessary to pass va_list

void Log::debug(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	log(Level::DEBUG, str, argv); 
	va_end(argv);
}

void Log::info(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	log(Level::INFO, str, argv); 
	va_end(argv);
}

void Log::warn(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	log(Level::WARN, str, argv); 
	va_end(argv);
}

void Log::error(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	log(Level::ERROR, str, argv); 
	va_end(argv);
}

void Log::fatal(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	log(Level::FATAL, str, argv); 
	va_end(argv);
}

// Original aprintf from https://gist.github.com/EleotleCram/eb586037e2976a8d9884
void Log::log(Level level, std::string str, va_list argv) {
	const char* tag = "";
	switch (level) {
		case Level::DEBUG:
			tag = "debug";
			break;
		case Level::INFO:
			tag = "info";
			break;
		case Level::WARN:
			tag = "warn";
			break;
		case Level::ERROR:
			tag = "error";
			break;
		case Level::FATAL:
			tag = "fatal";
			break;
	}
	Serial.print('['); Serial.print(tag); Serial.print("]\t");

	const char* cstr = str.c_str();
	int i, j, count = 0;

	for (i = 0, j = 0; cstr[i] != '\0'; i++) {
		if (cstr[i] == '%') {
			count++;

			Serial.write(reinterpret_cast<const uint8_t*>(cstr+j), i-j);

			switch (cstr[++i]) {
				case 'd': Serial.print(va_arg(argv, int));
					break;
				case 'l': Serial.print(va_arg(argv, long));
					break;
				case 'f': Serial.print(va_arg(argv, double));
					break;
				case 'c': Serial.print((char) va_arg(argv, int));
					break;
				case 's': Serial.print(va_arg(argv, char *));
					break;
				case '%': Serial.print("%");
					break;
				default:;
			};

			j = i+1;
		}
	};

	if (i > j) {
		Serial.write(reinterpret_cast<const uint8_t*>(cstr+j), i-j);
	}
	Serial.println();

	if (level == Level::FATAL) {
		exit(1);
	}
}
