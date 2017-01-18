#pragma once

#include "task.h"

namespace Logger {
	enum class Level;
}

namespace mission {

class NoOp : public BaseTask {
	public:
		NoOp(bool finite = true);
		bool onRun() override;

	private:
		bool m_finite;
};

class Function : public BaseTask {
	public:
		Function(std::function<void()> func);
		bool onRun() override;

	private:
		std::function<void()> m_func;
};

class Log : public BaseTask {
	public:
		Log(Logger::Level level, std::string msg);
		virtual ~Log();
		bool onRun() override;

	private:
		Logger::Level m_level;
		std::string m_msg;
};

class Debug : public Log {
	public:
		Debug(std::string msg);
};

class Info : public Log {
	public:
		Info(std::string msg);
};

class Warn : public Log {
	public:
		Warn(std::string msg);
};

class Error : public Log {
	public:
		Error(std::string msg);
};

class Fatal : public Log {
	public:
		Fatal(std::string msg);
};

} // namespace mission
