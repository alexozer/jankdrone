#include <Arduino.h>
#include <cstdarg>
#include "../../log.h"
#include "primitive.h"

using namespace mission;

NoOp::NoOp(bool finite): m_finite{finite} {}

bool NoOp::onRun() {
	return m_finite;
}

Function::Function(std::function<void()> func): m_func{func} {}

bool Function::onRun() {
	m_func();
	return true;
}

bool Logger::onRun() {
	m_buffer.flush();
	return true;
}

void Logger::write(Log::Level level, std::string str, va_list argv) {
	Log::log(level, str, argv, m_buffer);
}

Debug::Debug(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	write(Log::Level::DEBUG, str, argv);
	va_end(argv);
}

Info::Info(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	write(Log::Level::INFO, str, argv);
	va_end(argv);
}

Warn::Warn(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	write(Log::Level::WARN, str, argv);
	va_end(argv);
}

Error::Error(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	write(Log::Level::ERROR, str, argv);
	va_end(argv);
}

Fatal::Fatal(std::string str, ...) {
	va_list argv;
	va_start(argv, str);
	write(Log::Level::FATAL, str, argv);
	va_end(argv);
}
