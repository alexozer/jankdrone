#pragma once

#include "../../log.h"
#include "task.h"

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
		bool onFirstRun() override;

	private:
		std::function<void()> m_func;
};

class Logger : Function {
	public:
		Logger(Log::Level level, std::string msg);
};

class Debug : Function {
	public:
		Debug(std::string msg);
};

class Info : Function {
	public:
		Info(std::string msg);
};

class Warn : Function {
	public:
		Warn(std::string msg);
};

class Error : Function {
	public:
		Error(std::string msg);
};

class Fatal : Function {
	public:
		Fatal(std::string msg);
};

// TODO support printf-like arguments in logging tasks
//class Logger : Function {
	//public:
		//template <typename... Args>
		//Logger(std::string format, Args&&... args):
			//Function{[=] {
				//Log::debug(format, std::forward<Args>(args)...);
			//}} {}
//};

//struct Debug : Logger {
	//template <typename... Args>
	//Debug(std::string str, Args&&... args):
		//Logger{str, std::forward<Args>(args)...} {}
//};

} // namespace mission
