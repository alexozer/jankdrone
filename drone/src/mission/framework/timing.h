#pragma once

#include "task.h"

namespace mission {

class Timer : public BaseTask {
	public:
		Timer(unsigned long period);
		bool onFirstRun() override;
		bool onRun() override;

	private:
		unsigned long m_period;
		unsigned long m_startTime;
};

class Timeout : public BaseTask {
	public:
		Timeout(Task task, unsigned long timeout);
		bool onRun() override;
		bool timedOut() const;

	private:
		Task m_task, m_timer;
		bool m_timedOut;
};

} // namespace mission
