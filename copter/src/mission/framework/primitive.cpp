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

//Debug::Debug(std::string msg, ...): BaseTask{[] {
	//va_list argv;
	//va_begin(argv, msg);
	//auto task = make<Function>([] { Log.log(Log.Level.DEBUG, msg, argv); });
	//va_end(argv);
	//return task;
//}} {}
