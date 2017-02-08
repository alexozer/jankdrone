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

bool Function::onFirstRun() {
	m_func();
	return true;
}

Logger::Logger(Log::Level level, std::string msg):
	Function{[=] { Log::log(level, msg); }} {}

Debug::Debug(std::string msg):
	Function{[=] { Log::debug(msg); }} {}

Info::Info(std::string msg):
	Function{[=] { Log::info(msg); }} {}

Warn::Warn(std::string msg):
	Function{[=] { Log::warn(msg); }} {}

Error::Error(std::string msg):
	Function{[=] { Log::error(msg); }} {}

Fatal::Fatal(std::string msg):
	Function{[=] { Log::fatal(msg); }} {}
