#include "../../logger.h"
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

Log::Log(Logger::Level level, std::string msg): m_level{level}, m_msg{msg} {}

bool Log::onRun() {
	Logger::log(m_level, m_msg);
	return true;
}

Debug::Debug(std::string msg): Log{Logger::Level::DEBUG, msg} {}
Info::Info(std::string msg): Log{Logger::Level::INFO, msg} {}
Warn::Warn(std::string msg): Log{Logger::Level::WARN, msg} {}
Error::Error(std::string msg): Log{Logger::Level::ERROR, msg} {}
Fatal::Fatal(std::string msg): Log{Logger::Level::FATAL, msg} {}
