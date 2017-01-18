#pragma once

#include "framework/all.h"

class Square : public BaseTask {
	public:
		Square();
		bool onRun() override;

	private:
		Task m_task;
};
