#include "square.h"

using namespace mission;

Square::Square() {
	std::vector<Task> subtasks;
	for (int i = 0; i < 4; i++) {
		subtasks.insert(subtasks.end(), {
				make<Pitch>(2),
				make<Timer>(2),
				make<Pitch>(0),
				make<DeltaYaw>(90),
				});
	}

	m_task = make<Sequential>(subtasks);
}

bool Square::onRun() {
	return (*m_task)();
}
