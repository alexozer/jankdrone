#pragma once

#include "../../log.h"
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

//struct Debug {
	//Debug(std::string msg, ...);
//};

} // namespace mission
