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
		bool onRun() override;

	private:
		std::function<void()> m_func;
};

class Logger : BaseTask {
	public:
		bool onRun() override;

	protected:
		void write(Log::Level level, std::string str, va_list argv);

	private:
		BufferPrint m_buffer;
};

struct Debug : Logger {
	Debug(std::string str, ...);
};

struct Info : Logger {
	Info(std::string str, ...);
};

struct Warn : Logger {
	Warn(std::string str, ...);
};

struct Error : Logger {
	Error(std::string str, ...);
};

struct Fatal : Logger {
	Fatal(std::string str, ...);
};

} // namespace mission
