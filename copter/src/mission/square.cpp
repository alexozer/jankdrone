#include "square.h"

using namespace mission;

Square::Square(): BaseTask{[] {

		std::vector<Task> subtasks;
		for (int i = 0; i < 4; i++) {
			subtasks.insert(subtasks.end(), {
					make<Pitch>(2),
					make<Timer>(2),
					make<Pitch>(0),
					make<DeltaYaw>(90),
					});
		}
		return make<Sequential>(subtasks);

	}()} {}
