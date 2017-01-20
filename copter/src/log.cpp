#include "log.h"

size_t BufferPrint::write(uint8_t b) {
	return write(&b, 1);
}

size_t BufferPrint::write(const uint8_t* buffer, size_t size) {
	m_buf.insert(m_buf.end(), buffer, &buffer[size]);
	return size;
}

void BufferPrint::flush() {
	Serial.write(m_buf.data(), m_buf.size());
}

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
void Log::log(Level level, std::string str, va_list argv, Print& printer) {
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
	printer.print('['); printer.print(tag); printer.print("]\t");

	const char* cstr = str.c_str();
	int i, j, count = 0;

	for (i = 0, j = 0; cstr[i] != '\0'; i++) {
		if (cstr[i] == '%') {
			count++;

			printer.write(reinterpret_cast<const uint8_t*>(cstr+j), i-j);

			switch (cstr[++i]) {
				case 'd': printer.print(va_arg(argv, int));
					break;
				case 'l': printer.print(va_arg(argv, long));
					break;
				case 'f': printer.print(va_arg(argv, double));
					break;
				case 'c': printer.print((char) va_arg(argv, int));
					break;
				case 's': printer.print(va_arg(argv, char *));
					break;
				case '%': printer.print("%");
					break;
				default:;
			};

			j = i+1;
		}
	};

	if (i > j) {
		printer.write(reinterpret_cast<const uint8_t*>(cstr+j), i-j);
	}
	printer.println();

	if (level == Level::FATAL) {
		exit(1);
	}
}
